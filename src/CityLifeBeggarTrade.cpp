/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeBeggarTrade.h"
#include "LifeBotReservation.h"

#include "Config.h"
#include "GameTime.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Random.h"
#include "SocialMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <algorithm>
#include <cmath>

namespace
{
    uint32 TradeTime()
    {
        return static_cast<uint32>(GameTime::GetGameTime().count());
    }

    struct InitiationScope
    {
        ObjectGuid& Bot;
        ObjectGuid& Target;

        ~InitiationScope()
        {
            Bot.Clear();
            Target.Clear();
        }
    };
}

namespace CityLife
{
    BeggarTrade& BeggarTrade::Instance()
    {
        static BeggarTrade instance;
        return instance;
    }

    void BeggarTrade::LoadConfig()
    {
        // Cancel before the caller releases outfits/listeners. Cooldowns survive reloads and role rotation.
        for (auto& [guidLow, request] : _beggars)
            Cancel(guidLow, ObjectAccessor::FindConnectedPlayer(
                ObjectGuid::Create<HighGuid::Player>(guidLow)), request);

        _enabled = sConfigMgr->GetOption<bool>("CityLife.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Beggar.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Beggar.Trade.Enable", true);
        _minSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.Trade.MinSeconds", 180), 30, 86400);
        _maxSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.Trade.MaxSeconds", 420), _minSeconds, 86400);
        _chancePct = std::min<uint32>(100,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.Trade.ChancePct", 25));
        _range = sConfigMgr->GetOption<float>("CityLife.Custom.Beggar.Trade.Range", 8.0f);
        _range = std::isfinite(_range) ? std::clamp(_range, 1.0f, TRADE_DISTANCE) : 8.0f;
        _durationSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.Trade.DurationSeconds", 15), 5, 60);
        _playerCooldownSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.Trade.PlayerCooldownSeconds", 600), 60, 86400);
        _nextPruneAt = 0;
    }

    void BeggarTrade::Begin(Player* player)
    {
        if (!player || !GET_PLAYERBOT_AI(player) || !PlayerbotsLife::IsReservedBy(
            player->GetGUID().GetCounter(), PlayerbotsLife::ReservationOwner::CustomLife))
            return;
        uint32 const guidLow = player->GetGUID().GetCounter();
        _beggars.try_emplace(guidLow);
        _nextBotRequestAt.try_emplace(guidLow, TradeTime() + urand(_minSeconds, _maxSeconds));
    }

    void BeggarTrade::Cancel(uint32 guidLow, Player* player, Request& request)
    {
        if (!request.Partner)
            return;
        // Never close another trade after either participant has switched to a different partner.
        if (player && player->GetTrader() && player->GetTrader()->GetGUID() == request.Partner &&
            player->GetTrader()->GetTrader() == player)
            player->TradeCancel(true);
        else if (Player* partner = ObjectAccessor::FindConnectedPlayer(request.Partner))
            if (partner->GetTrader() && partner->GetTrader()->GetGUID() ==
                ObjectGuid::Create<HighGuid::Player>(guidLow) && partner->GetTrader()->GetTrader() == partner)
                partner->TradeCancel(true);
        request.Partner.Clear();
        request.Until = 0;
    }

    void BeggarTrade::End(Player* player)
    {
        if (!player)
            return;
        uint32 const guidLow = player->GetGUID().GetCounter();
        auto request = _beggars.find(guidLow);
        if (request == _beggars.end())
            return;
        Cancel(guidLow, player, request->second);
        _beggars.erase(request);
    }

    void BeggarTrade::Forget(uint32 guidLow)
    {
        auto request = _beggars.find(guidLow);
        if (request == _beggars.end())
            return;
        Cancel(guidLow, ObjectAccessor::FindConnectedPlayer(
            ObjectGuid::Create<HighGuid::Player>(guidLow)), request->second);
        _beggars.erase(request);
    }

    bool BeggarTrade::CanParticipate(Player* player) const
    {
        if (!player || !player->GetSession() || !player->IsInWorld() || player->IsBeingTeleported() ||
            !player->IsAlive() || player->IsInCombat() || player->HasUnitState(UNIT_STATE_STUNNED) ||
            player->IsInFlight() || player->GetTransport() || player->GetVehicle() || player->GetTransGUID() ||
            player->GetGroup() || player->InBattleground() || player->InBattlegroundQueue() || player->duel ||
            player->GetSession()->IsLoggingOut() || !player->GetMap() || player->GetMap()->Instanceable() ||
            !PlayerbotsLife::IsReservedBy(player->GetGUID().GetCounter(),
                PlayerbotsLife::ReservationOwner::CustomLife))
            return false;
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        return ai && !ai->HasGameClientMaster();
    }

    bool BeggarTrade::IsEligibleTarget(Player* player, Player* target) const
    {
        if (!target || target == player || !::IsRealPlayer(target) || !target->GetSession() ||
            !target->IsInWorld() || !target->IsAlive() || target->IsBeingTeleported() ||
            target->IsInCombat() || target->HasUnitState(UNIT_STATE_STUNNED) || target->IsInFlight() ||
            target->GetTransport() || target->GetVehicle() || target->GetTransGUID() ||
            target->InBattleground() || target->InBattlegroundQueue() || target->duel ||
            target->isAFK() || target->isDND() || target->GetSession()->IsLoggingOut() ||
            target->GetTeamId() != player->GetTeamId() || !player->InSamePhase(target) ||
            !player->IsWithinDistInMap(target, _range, false) || !player->IsWithinLOSInMap(target) ||
            !player->CanSeeOrDetect(target) || !target->GetSocial() || !player->GetSocial() ||
            target->GetSocial()->HasIgnore(player->GetGUID()) || player->GetSocial()->HasIgnore(target->GetGUID()))
            return false;
        return !target->GetTradeData() || target->GetTrader() == player;
    }

    void BeggarTrade::PruneCooldowns(uint32 now)
    {
        if (_nextPruneAt > now)
            return;
        _nextPruneAt = now + MINUTE;
        std::erase_if(_nextBotRequestAt, [&](auto const& entry)
        {
            return entry.second <= now && !_beggars.count(entry.first);
        });
        std::erase_if(_nextPlayerRequestAt, [now](auto const& entry) { return entry.second <= now; });
    }

    void BeggarTrade::Update(Player* player, bool allowRequest)
    {
        if (!player)
            return;
        uint32 const guidLow = player->GetGUID().GetCounter();
        auto registered = _beggars.find(guidLow);
        if (registered == _beggars.end())
            return;
        uint32 const now = TradeTime();
        PruneCooldowns(now);
        Request& request = registered->second;
        if (request.Partner)
        {
            Player* partner = ObjectAccessor::FindConnectedPlayer(request.Partner);
            if (!_enabled || now >= request.Until || !partner || !CanParticipate(player) ||
                !IsEligibleTarget(player, partner) || player->GetTrader() != partner ||
                partner->GetTrader() != player)
                Cancel(guidLow, player, request);
            return;
        }
        if (!_enabled || !allowRequest || !CanParticipate(player) || player->GetTradeData() ||
            player->isMoving() || player->IsNonMeleeSpellCast(false))
            return;
        auto next = _nextBotRequestAt.try_emplace(guidLow, now + urand(_minSeconds, _maxSeconds)).first;
        if (now < next->second)
            return;
        next->second = now + urand(_minSeconds, _maxSeconds);
        if (!roll_chance_i(_chancePct))
            return;

        Player* selected = nullptr;
        uint32 eligible = 0;
        for (auto const& reference : player->GetMap()->GetPlayers())
        {
            Player* target = reference.GetSource();
            if (!IsEligibleTarget(player, target) || target->GetTradeData())
                continue;
            auto cooldown = _nextPlayerRequestAt.find(target->GetGUID().GetCounter());
            if (cooldown != _nextPlayerRequestAt.end() && cooldown->second > now)
                continue;
            if (urand(1, ++eligible) == 1)
                selected = target;
        }
        if (!selected)
            return;

        ObjectGuid const targetGuid = selected->GetGUID();
        {
            // Only this synchronous, exact initiator/target pair may pass the module's trade-init guard.
            _initiatingBot = player->GetGUID();
            _initiatingTarget = targetGuid;
            InitiationScope scope{_initiatingBot, _initiatingTarget};
            WorldPacket packet(CMSG_INITIATE_TRADE);
            packet << targetGuid;
            player->GetSession()->HandleInitiateTradeOpcode(packet);
        }
        registered = _beggars.find(guidLow);
        if (player->GetTrader() != selected || selected->GetTrader() != player)
            return;
        if (registered == _beggars.end())
        {
            player->TradeCancel(true);
            return;
        }
        registered->second.Partner = targetGuid;
        registered->second.Until = now + _durationSeconds;
        _nextPlayerRequestAt[selected->GetGUID().GetCounter()] = now + _playerCooldownSeconds;
        player->SetFacingToObject(selected);
        player->HandleEmoteCommand(EMOTE_ONESHOT_BEG);
        // No Begin/Accept/SetTradeGold/SetTradeItem handler is called. The normal client owns its response.
    }

    bool BeggarTrade::IsInitiating(Player const* player, Player const* target) const
    {
        return _enabled && player && target && _initiatingBot == player->GetGUID() &&
            _initiatingTarget == target->GetGUID() && _beggars.count(player->GetGUID().GetCounter());
    }
}
