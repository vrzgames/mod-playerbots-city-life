/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeHumanBehavior.h"

#include "CellImpl.h"
#include "Config.h"
#include "DBCStores.h"
#include "GameTime.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Random.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    uint32 BehaviorTime()
    {
        return static_cast<uint32>(GameTime::GetGameTime().count());
    }

    Player* FindBehaviorPlayer(uint32 guid)
    {
        return ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(guid));
    }

    float ReadRange(char const* name, float fallback, float maximum)
    {
        float value = sConfigMgr->GetOption<float>(name, fallback);
        return std::isfinite(value) ? std::clamp(value, 1.0f, maximum) : fallback;
    }

    bool IsPureCompanionSpell(SpellInfo const* spell, Player* player)
    {
        if (!spell || spell->IsPassive() || spell->IsChanneled() || spell->CalcCastTime(player) != 0 ||
            spell->Speed != 0.0f || spell->HasAttribute(SPELL_ATTR1_DISMISS_PET_FIRST))
            return false;

        uint32 summons = 0;
        for (auto const& effect : spell->Effects)
        {
            if (!effect.Effect)
                continue;
            if (effect.Effect != SPELL_EFFECT_SUMMON)
                return false;
            SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(effect.MiscValueB);
            // The core routes flag 512 and PET categories through the guardian/combat-pet path.
            if (!properties || properties->Type != SUMMON_TYPE_MINIPET || (properties->Flags & 512) ||
                (properties->Category != SUMMON_CATEGORY_WILD && properties->Category != SUMMON_CATEGORY_ALLY &&
                    properties->Category != SUMMON_CATEGORY_UNK))
                return false;
            ++summons;
        }
        return summons == 1;
    }
}

namespace CityLife
{
    HumanBehavior& HumanBehavior::Instance()
    {
        static HumanBehavior instance;
        return instance;
    }

    void HumanBehavior::LoadConfig()
    {
        Reset();
        _enabled = sConfigMgr->GetOption<bool>("CityLife.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.HumanBehavior.Enable", true);
        _checkIntervalSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.CheckIntervalSeconds", 10), 5, 60);
        _maxBotsPerCheck = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.MaxBotsPerCheck", 32), 1, 128);
        _observerRange = ReadRange("CityLife.HumanBehavior.ObserverRange", 60.0f, 100.0f);
        _inspectRange = ReadRange("CityLife.HumanBehavior.Inspect.Range", 20.0f, INSPECT_DISTANCE);
        _idleSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.IdleSeconds", 15), 5, 300);

        auto loadFeature = [this](Behavior behavior, char const* name, bool enabled, uint32 chance,
            uint32 minSeconds, uint32 maxSeconds)
        {
            std::string prefix = std::string("CityLife.HumanBehavior.") + name;
            Feature& feature = _features[behavior];
            feature.Enabled = sConfigMgr->GetOption<bool>(prefix + ".Enable", enabled);
            feature.Chance = std::min<uint32>(100,
                sConfigMgr->GetOption<uint32>(prefix + ".ChancePct", chance));
            feature.MinSeconds = std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(prefix + ".MinSeconds", minSeconds), 30, 86400);
            feature.MaxSeconds = std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(prefix + ".MaxSeconds", maxSeconds), feature.MinSeconds, 86400);
        };
        loadFeature(Inspect, "Inspect", true, 15, 120, 300);
        loadFeature(WeaponSheath, "WeaponSheath", true, 10, 120, 300);
        loadFeature(Sit, "Sit", false, 10, 300, 900);
        loadFeature(Companion, "Companion", false, 10, 300, 900);
        _enabled = _enabled && std::any_of(_features.begin(), _features.end(),
            [](Feature const& feature) { return feature.Enabled && feature.Chance != 0; });
        _sitMinSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.Sit.DurationMinSeconds", 8), 2, 60);
        _sitMaxSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.Sit.DurationMaxSeconds", 20), _sitMinSeconds, 60);
        _companionMinSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.Companion.DurationMinSeconds", 60), 10, 3600);
        _companionMaxSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.HumanBehavior.Companion.DurationMaxSeconds", 180),
            _companionMinSeconds, 3600);

        for (char const* name : {"RandomJump", "RandomStrafe"})
            if (sConfigMgr->GetOption<bool>(std::string("CityLife.HumanBehavior.") + name + ".Enable", false))
                LOG_WARN("module", "[CityLife] {} remains disabled: this core has no additive movement API "
                    "that preserves the active route and movement generator.", name);
        if (_enabled && _features[Sit].Enabled)
            LOG_WARN("module", "[CityLife] Ambient sitting enabled: the core's normal sitting-victim "
                "critical-hit rules still apply to the first incoming hit.");
    }

    bool HumanBehavior::IsEligible(Player* player) const
    {
        if (!player || !player->GetSession() || player->GetSession()->IsLoggingOut() ||
            !player->IsInWorld() || !player->IsAlive() || player->IsBeingTeleported() ||
            !player->GetMap() || !player->GetMap()->IsWorldMap() || player->GetMap()->Instanceable() ||
            player->InBattleground() ||
            player->InBattlegroundQueue() || player->IsInCombat() || player->GetVictim() || player->duel ||
            player->GetGroup() || player->IsInFlight() || player->IsMounted() || player->IsFlying() ||
            player->IsFalling() || player->IsInWater() || player->GetTransport() || player->GetVehicle() ||
            player->GetTransGUID() || player->HasUnitState(UNIT_STATE_LOST_CONTROL | UNIT_STATE_NOT_MOVE) ||
            player->IsNonMeleeSpellCast(false) || player->GetTradeData() ||
            player->HasAuraType(SPELL_AURA_MOD_STEALTH) || player->HasAuraType(SPELL_AURA_MOD_INVISIBILITY) ||
            player->GetShapeshiftForm() != FORM_NONE ||
            (player->GetInterruptMask() & AURA_INTERRUPT_FLAG_NOT_SEATED) || ::IsRealPlayer(player))
            return false;
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        return ai && !ai->HasGameClientMaster() && ai->GetState() == BOT_STATE_NON_COMBAT;
    }

    bool HumanBehavior::IsIdle(Player* player) const
    {
        return !player->isMoving() && player->IsStopped() && player->movespline->Finalized() &&
            player->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE &&
            player->GetUInt32Value(UNIT_NPC_EMOTESTATE) == EMOTE_ONESHOT_NONE;
    }

    bool HumanBehavior::CanObserve(Player* observer, Player* bot) const
    {
        return observer && observer != bot && observer->GetSession() && observer->IsInWorld() &&
            observer->IsAlive() && !observer->IsBeingTeleported() && ::IsRealPlayer(observer) &&
            observer->InSamePhase(bot) && observer->IsWithinDistInMap(bot, _observerRange) &&
            observer->CanSeeOrDetect(bot) && bot->CanSeeOrDetect(observer);
    }

    bool HumanBehavior::IsSitting(uint32 guid)
    {
        std::lock_guard<std::mutex> lock(_sittingMutex);
        return _sittingBots.count(guid) != 0;
    }

    void HumanBehavior::Wake(Player* player)
    {
        if (!player)
            return;
        bool owned;
        {
            std::lock_guard<std::mutex> lock(_sittingMutex);
            owned = _sittingBots.erase(player->GetGUID().GetCounter()) != 0;
        }
        // A rest action may have adopted the posture. Never cancel its food/drink auras.
        if (owned && player->getStandState() == UNIT_STAND_STATE_SIT &&
            !(player->GetInterruptMask() & AURA_INTERRUPT_FLAG_NOT_SEATED))
            player->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void HumanBehavior::DismissCompanion(Player* player, State& state)
    {
        if (player && player->IsInWorld() && !state.Critter.IsEmpty() &&
            player->GetCritterGUID() == state.Critter)
            if (Creature* creature = ObjectAccessor::GetCreature(*player, state.Critter))
                if (TempSummon* summon = creature->ToTempSummon())
                    if (summon->GetSummonerGUID() == player->GetGUID() && summon->m_Properties &&
                        summon->m_Properties->Type == SUMMON_TYPE_MINIPET)
                        summon->UnSummon();
        state.Critter.Clear();
        state.CompanionUntil = 0;
    }

    void HumanBehavior::Remove(uint32 guid)
    {
        auto found = _states.find(guid);
        if (found == _states.end())
            return;
        Player* player = FindBehaviorPlayer(guid);
        Wake(player);
        if (!player)
        {
            std::lock_guard<std::mutex> lock(_sittingMutex);
            _sittingBots.erase(guid);
        }
        DismissCompanion(player, found->second);
        _states.erase(found);
    }

    void HumanBehavior::Reset()
    {
        while (!_states.empty())
            Remove(_states.begin()->first);
        std::lock_guard<std::mutex> lock(_sittingMutex);
        _sittingBots.clear();
        _nextCheckAt = 0;
        _cursor = 0;
    }

    void HumanBehavior::TryInspect(Player* player, std::vector<Player*> const& nearby) const
    {
        Player* target = nullptr;
        uint32 matches = 0;
        for (Player* other : nearby)
        {
            if (!other || other == player || !other->IsInWorld() || !other->IsAlive() ||
                other->IsBeingTeleported() || other->IsInCombat() || !player->InSamePhase(other) ||
                !player->IsWithinDistInMap(other, _inspectRange, false) ||
                player->IsValidAttackTarget(other) || !player->CanSeeOrDetect(other) ||
                !player->IsWithinLOSInMap(other))
                continue;
            if (urand(1, ++matches) == 1)
                target = other;
        }
        if (!target)
            return;
        // Inspect the supplied GUID directly; never change selection, facing or an AI target value.
        WorldPacket packet(CMSG_INSPECT, 8);
        packet << target->GetGUID();
        player->GetSession()->HandleInspectOpcode(packet);
    }

    void HumanBehavior::TrySheath(Player* player) const
    {
        for (WeaponAttackType attack : {BASE_ATTACK, OFF_ATTACK})
            if (Item* weapon = player->GetWeaponForAttack(attack, true))
                if (weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                    return;
        // Player::SetSheath(MELEE) consumes temporary-enchant charges in this core. Guard both weapons.
        if (player->GetSheath() == SHEATH_STATE_UNARMED)
            player->SetSheath(SHEATH_STATE_MELEE);
        else if (player->GetSheath() == SHEATH_STATE_MELEE)
            player->SetSheath(SHEATH_STATE_UNARMED);
    }

    void HumanBehavior::TryCompanion(Player* player, State& state, uint32 now) const
    {
        // Leave an existing companion alone, including one summoned by another script or the AI.
        if (!player->GetCritterGUID().IsEmpty() || !state.Critter.IsEmpty())
            return;
        uint32 chosenSpell = 0;
        uint32 matches = 0;
        for (auto const& [spellId, learned] : player->GetSpellMap())
        {
            if (!player->HasActiveSpell(spellId) || player->HasSpellCooldown(spellId) ||
                !IsPureCompanionSpell(sSpellMgr->GetSpellInfo(spellId), player))
                continue;
            if (urand(1, ++matches) == 1)
                chosenSpell = spellId;
        }
        if (!chosenSpell)
            return;
        // Only instant, single-effect cosmetic summons reach here. Triggering avoids occupying the AI's
        // current spell slot, starting a GCD, resetting attacks or changing its next-action delay.
        if (player->CastSpell(player, chosenSpell, TRIGGERED_FULL_MASK) == SPELL_CAST_OK &&
            !player->GetCritterGUID().IsEmpty())
        {
            state.Critter = player->GetCritterGUID();
            state.CompanionUntil = now + urand(_companionMinSeconds, _companionMaxSeconds);
        }
    }

    void HumanBehavior::Update(std::vector<HumanBehaviorBot> const& candidates)
    {
        if (!_enabled)
            return;
        uint32 const now = BehaviorTime();
        std::unordered_set<uint32> present;
        present.reserve(candidates.size());
        for (HumanBehaviorBot const& candidate : candidates)
        {
            present.insert(candidate.Guid);
            auto [entry, inserted] = _states.try_emplace(candidate.Guid);
            State& state = entry->second;
            if (inserted)
                for (uint8 behavior = 0; behavior < BehaviorCount; ++behavior)
                    state.NextAt[behavior] = now + urand(_features[behavior].MinSeconds,
                        _features[behavior].MaxSeconds);
            Player* player = FindBehaviorPlayer(candidate.Guid);
            bool const eligible = IsEligible(player);
            bool const idle = eligible && candidate.AllowIdleActions && IsIdle(player);
            if (idle && player->getStandState() == UNIT_STAND_STATE_STAND)
            {
                if (!state.IdleSince)
                    state.IdleSince = now;
            }
            else
                state.IdleSince = 0;

            if (state.SitUntil && (!idle || now >= state.SitUntil || !IsSitting(candidate.Guid) ||
                player->getStandState() != UNIT_STAND_STATE_SIT))
            {
                Wake(player);
                state.SitUntil = 0;
            }
            // Cleanup never changes combat pets. Ordinary timed dismiss waits until out of combat.
            if (!state.Critter.IsEmpty() && (!player || !player->IsInCombat()) &&
                (!eligible || now >= state.CompanionUntil || player->GetCritterGUID() != state.Critter))
                DismissCompanion(player, state);
        }
        for (auto entry = _states.begin(); entry != _states.end();)
        {
            uint32 guid = entry->first;
            ++entry;
            if (!present.count(guid))
                Remove(guid);
        }
        if (candidates.empty() || now < _nextCheckAt)
            return;
        _nextCheckAt = now + _checkIntervalSeconds;
        uint32 const count = static_cast<uint32>(std::min<std::size_t>(_maxBotsPerCheck, candidates.size()));
        for (uint32 index = 0; index < count; ++index)
        {
            _cursor %= candidates.size();
            HumanBehaviorBot const& candidate = candidates[_cursor++];
            Player* player = FindBehaviorPlayer(candidate.Guid);
            if (!IsEligible(player))
                continue;
            State& state = _states.at(candidate.Guid);
            bool due = false;
            for (uint8 behavior = 0; behavior < BehaviorCount; ++behavior)
                due = due || (_features[behavior].Enabled && now >= state.NextAt[behavior]);
            if (!due)
                continue;

            std::vector<Player*> nearby;
            Acore::AnyPlayerInObjectRangeCheck check(player, std::max(_observerRange, _inspectRange));
            Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(player, nearby, check);
            Cell::VisitObjects(player, searcher, std::max(_observerRange, _inspectRange));
            bool const observed = std::any_of(nearby.begin(), nearby.end(),
                [this, player](Player* other) { return CanObserve(other, player); });
            for (uint8 behavior = 0; behavior < BehaviorCount; ++behavior)
            {
                Feature const& feature = _features[behavior];
                if (!feature.Enabled || now < state.NextAt[behavior])
                    continue;
                // Failed observer/chance/idle checks also consume the cooldown.
                state.NextAt[behavior] = now + urand(feature.MinSeconds, feature.MaxSeconds);
                if (!observed || !roll_chance_i(feature.Chance))
                    continue;
                if (behavior == Inspect)
                    TryInspect(player, nearby);
                else if (behavior == WeaponSheath)
                    TrySheath(player);
                else if (candidate.AllowIdleActions && state.IdleSince &&
                    now >= state.IdleSince + _idleSeconds && IsIdle(player) &&
                    player->getStandState() == UNIT_STAND_STATE_STAND)
                {
                    if (behavior == Sit)
                    {
                        {
                            std::lock_guard<std::mutex> lock(_sittingMutex);
                            _sittingBots.insert(candidate.Guid);
                        }
                        state.SitUntil = now + urand(_sitMinSeconds, _sitMaxSeconds);
                        player->SetStandState(UNIT_STAND_STATE_SIT);
                    }
                    else if (behavior == Companion)
                        TryCompanion(player, state, now);
                }
            }
        }
    }
}
