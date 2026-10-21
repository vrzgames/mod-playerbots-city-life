/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeCustomActivities.h"
#include "CityLifeCustomAppearance.h"
#include "CityLifeBeggarTrade.h"
#include "LifeBotReservation.h"

#include "Config.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "Engine.h"
#include "Event.h"
#include "GameTime.h"
#include "Log.h"
#include "Map.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "SpellMgr.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <list>
#include <sstream>
#include <utility>

namespace CityLife
{
    struct CustomPauseControl
    {
        std::atomic<bool> Active{true};
        std::atomic<bool> Attached{true};
        std::uintptr_t EngineTag = 0;
    };
}

namespace
{
    constexpr std::array<char const*, 6> RoleNames =
        {"", "Legendary", "Beggar", "Buyer", "TrainingDummy", "Fishing"};

    uint32 Now() { return static_cast<uint32>(GameTime::GetGameTime().count()); }

    class CustomPauseListener final : public ActionExecutionListener
    {
    public:
        explicit CustomPauseListener(std::shared_ptr<CityLife::CustomPauseControl> control)
            : _control(std::move(control)) { }

        ~CustomPauseListener() override { _control->Attached.store(false); }
        bool Before(Action* /*action*/, Event /*event*/) override { return !_control->Active.load(); }
        bool AllowExecution(Action* /*action*/, Event /*event*/) override { return !_control->Active.load(); }
        void After(Action* /*action*/, bool /*executed*/, Event /*event*/) override { }
        bool OverrideResult(Action* /*action*/, bool executed, Event /*event*/) override { return executed; }

    private:
        std::shared_ptr<CityLife::CustomPauseControl> _control;
    };

    char const* PracticeSpell(uint8 playerClass)
    {
        switch (playerClass)
        {
            case CLASS_WARRIOR: return "heroic strike";
            case CLASS_PALADIN: return "judgement of wisdom";
            case CLASS_HUNTER: return "arcane shot";
            case CLASS_ROGUE: return "sinister strike";
            case CLASS_PRIEST: return "smite";
            case CLASS_DEATH_KNIGHT: return "icy touch";
            case CLASS_SHAMAN: return "lightning bolt";
            case CLASS_MAGE: return "frostbolt";
            case CLASS_WARLOCK: return "shadow bolt";
            case CLASS_DRUID: return "wrath";
            default: return "";
        }
    }
}

namespace CityLife
{
    CustomActivities& CustomActivities::Instance()
    {
        static CustomActivities instance;
        return instance;
    }

    void CustomActivities::LoadConfig()
    {
        _enabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Enable", true);
        _timeScaling = sConfigMgr->GetOption<bool>("CityLife.Custom.TimeScaling", false);
        _residenceSeconds = sConfigMgr->GetOption<uint32>("CityLife.Custom.ResidenceSeconds", 0);
        _chatRange = std::clamp(sConfigMgr->GetOption<float>("CityLife.Custom.Chat.Range", 40.0f), 5.0f, 100.0f);
        _fishingTimeScaling = sConfigMgr->GetOption<bool>("FishingLife.TimeScaling", false);
        _fishingResidenceSeconds = sConfigMgr->GetOption<uint32>("FishingLife.ResidenceSeconds", 0);
        _fishingChatRange = std::clamp(
            sConfigMgr->GetOption<float>("FishingLife.Chat.Range", 40.0f), 5.0f, 100.0f);
        for (size_t index = 1; index < _roles.size(); ++index)
        {
            RoleSettings& role = _roles[index];
            bool fishing = index == static_cast<size_t>(CustomRole::Fishing);
            std::string prefix = fishing ? "FishingLife" : std::string("CityLife.Custom.") + RoleNames[index];
            role.Enabled = (fishing || _enabled) && sConfigMgr->GetOption<bool>(prefix + ".Enable", true);
            uint32 minLevel = (index == static_cast<size_t>(CustomRole::Legendary) ||
                index == static_cast<size_t>(CustomRole::TrainingDummy)) ? 80 : 1;
            uint32 maxLevel = index == static_cast<size_t>(CustomRole::Beggar) ? 1 : 80;
            role.MinLevel = static_cast<uint8>(std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(prefix + ".MinLevel", minLevel), 1, 80));
            role.MaxLevel = static_cast<uint8>(std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(prefix + ".MaxLevel", maxLevel), role.MinLevel, 80));
            if (index == static_cast<size_t>(CustomRole::Beggar) ||
                index == static_cast<size_t>(CustomRole::Buyer) || index == static_cast<size_t>(CustomRole::Fishing))
            {
                role.ChatMin = std::clamp<uint32>(
                    sConfigMgr->GetOption<uint32>(prefix + ".ChatMinSeconds", 180), 30, 86400);
                role.ChatMax = std::clamp<uint32>(
                    sConfigMgr->GetOption<uint32>(prefix + ".ChatMaxSeconds", 420), role.ChatMin, 86400);
                role.YellChance = std::min<uint32>(100,
                    sConfigMgr->GetOption<uint32>(prefix + ".YellChancePct", index == 2 ? 20 : 0));
            }
        }
        _dummyRange = std::clamp(
            sConfigMgr->GetOption<float>("CityLife.Custom.TrainingDummy.TargetRange", 15.0f), 3.0f, 40.0f);
        _practiceMin = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.TrainingDummy.PracticeMinSeconds", 60), 10, 3600);
        _practiceMax = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.TrainingDummy.PracticeMaxSeconds", 180),
            _practiceMin, 3600);
        _restMin = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.TrainingDummy.RestMinSeconds", 15), 5, 3600);
        _restMax = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.TrainingDummy.RestMaxSeconds", 45), _restMin, 3600);
        _fishMin = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("FishingLife.CastMinSeconds", 20), 10, 300);
        _fishMax = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("FishingLife.CastMaxSeconds", 35), _fishMin, 300);
        _fishSpell = sConfigMgr->GetOption<uint32>("FishingLife.Spell", 7620);
        _fishMaximumWaterHeightDifference = std::clamp(
            sConfigMgr->GetOption<float>("FishingLife.MaximumWaterHeightDifference", 12.0f), 0.5f, 30.0f);
        _beggarEmoteMin = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.EmoteMinSeconds", 45), 10, 3600);
        _beggarEmoteMax = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.EmoteMaxSeconds", 120), _beggarEmoteMin, 3600);
        _beggarDanceChance = std::min<uint32>(100,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.DanceChancePct", 25));
        _beggarDanceSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Beggar.DanceSeconds", 8), 2, 30);
        _dummyEntries.clear();
        std::string entries = sConfigMgr->GetOption<std::string>(
            "CityLife.Custom.TrainingDummy.TargetEntries", "31144,31146,32666,32667");
        std::replace(entries.begin(), entries.end(), ',', ' ');
        std::istringstream entryStream(entries);
        uint32 entry = 0;
        while (entryStream >> entry)
            if (entry)
                _dummyEntries.push_back(entry);
        sCityLifeCustomAppearance.LoadConfig();
        sCityLifeBeggarTrade.LoadConfig();
    }

    void CustomActivities::LoadData()
    {
        _lines.clear();
        if ((!_enabled && !_roles[static_cast<size_t>(CustomRole::Fishing)].Enabled) ||
            !WorldDatabase.Query("SHOW TABLES LIKE 'city_life_custom_chat'"))
            return;
        QueryResult result = WorldDatabase.Query(
            "SELECT group_name,speaker_team,text,weight FROM city_life_custom_chat "
            "WHERE enabled=1 AND group_name IN ('beggar','buyer','fishing') ORDER BY id");
        if (!result)
            return;
        do
        {
            Field* fields = result->Fetch();
            uint8 team = fields[1].Get<uint8>();
            if (team > 2)
                continue;
            _lines.push_back({fields[0].Get<std::string>(), static_cast<TeamSide>(team),
                fields[2].Get<std::string>(), std::clamp<uint32>(fields[3].Get<uint32>(), 1, 100000)});
        } while (result->NextRow());
    }

    void CustomActivities::ConfigureHub(Hub& hub) const
    {
        std::string fishingPrefix = hub.Name.rfind("CustomFishing", 0) == 0 ? "CustomFishing" : "Fishing";
        if (hub.Name.rfind(fishingPrefix, 0) == 0)
        {
            hub.CustomCity = hub.Name.substr(fishingPrefix.size());
            hub.Custom = CustomRole::Fishing;
            RoleSettings const& settings = _roles[static_cast<size_t>(CustomRole::Fishing)];
            std::string prefix = "FishingLife." + hub.CustomCity;
            hub.Enabled = settings.Enabled && sConfigMgr->GetOption<bool>(prefix + ".Enable", true, false);
            hub.Questing = false;
            // Regional defaults keep low-level anglers out of high-level zones unless explicitly overridden.
            uint32 minLevel = std::clamp<uint32>(hub.MinLevel, settings.MinLevel, settings.MaxLevel);
            uint32 maxLevel = std::clamp<uint32>(hub.MaxLevel, minLevel, settings.MaxLevel);
            hub.MinLevel = static_cast<uint8>(std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(prefix + ".MinLevel", minLevel, false),
                settings.MinLevel, settings.MaxLevel));
            hub.MaxLevel = static_cast<uint8>(std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(prefix + ".MaxLevel", maxLevel, false),
                hub.MinLevel, settings.MaxLevel));
            hub.UseTimeScaling = _fishingTimeScaling;
            hub.ResidenceSeconds = _fishingResidenceSeconds;
            hub.ConfiguredPopulation = std::min<uint32>(2000, sConfigMgr->GetOption<uint32>(
                prefix + ".Population", hub.DefaultPopulation, false));
            return;
        }
        for (size_t index = 1; index < static_cast<size_t>(CustomRole::Fishing); ++index)
        {
            std::string prefix = std::string("Custom") + RoleNames[index];
            if (hub.Name.rfind(prefix, 0) != 0)
                continue;
            hub.CustomCity = hub.Name.substr(prefix.size());
            hub.Custom = static_cast<CustomRole>(index);
            RoleSettings const& settings = _roles[index];
            hub.Enabled = settings.Enabled;
            hub.Questing = false;
            hub.MinLevel = settings.MinLevel;
            hub.MaxLevel = settings.MaxLevel;
            hub.UseTimeScaling = _timeScaling;
            hub.ResidenceSeconds = _residenceSeconds;
            hub.ConfiguredPopulation = std::min<uint32>(2000, sConfigMgr->GetOption<uint32>(
                std::string("CityLife.Custom.") + RoleNames[index] + "." + hub.CustomCity + ".Population",
                hub.DefaultPopulation, false));
            return;
        }
    }

    bool CustomActivities::CanUse(Player* player, CustomRole role) const
    {
        if (role == CustomRole::None)
            return true;
        if (!player || !_roles[static_cast<size_t>(role)].Enabled ||
            !GET_PLAYERBOT_AI(player) || !GET_PLAYERBOT_AI(player)->GetCurrentEngine() ||
            player->IsMounted() || player->GetShapeshiftForm() != FORM_NONE ||
            player->GetGroup() || player->GetTradeData() || player->IsNonMeleeSpellCast(false))
            return false;
        if (role == CustomRole::Legendary || role == CustomRole::Beggar || role == CustomRole::Fishing)
            return sCityLifeCustomAppearance.CanUse(player, role);
        return true;
    }

    bool CustomActivities::Begin(Player* player, CustomRole role)
    {
        if (!CanUse(player, role))
            return false;
        uint32 guidLow = player->GetGUID().GetCounter();
        State state;
        state.Role = role;
        if (auto previous = _idleControls.find(guidLow); previous != _idleControls.end())
        {
            state.Controls = std::move(previous->second);
            _idleControls.erase(previous);
        }
        PauseAI(player, state);
        _states.emplace(guidLow, std::move(state));
        bool appearanceReady = true;
        if (role == CustomRole::Legendary || role == CustomRole::Beggar || role == CustomRole::Fishing)
        {
            appearanceReady = sCityLifeCustomAppearance.Begin(player, role);
            if (!appearanceReady && !sCityLifeCustomAppearance.HasPendingRecovery(player))
            {
                Forget(guidLow);
                return false;
            }
        }
        State& active = _states.at(guidLow);
        active.AppearanceReady = appearanceReady;
        active.NextAction = Now() + urand(3, 10);
        RoleSettings const& settings = _roles[static_cast<size_t>(role)];
        active.NextChat = Now() + urand(settings.ChatMin, settings.ChatMax);
        if (role == CustomRole::Beggar && appearanceReady)
            sCityLifeBeggarTrade.Begin(player);
        return true;
    }

    void CustomActivities::PauseAI(Player* player, State& state)
    {
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        Engine* engine = ai ? ai->GetCurrentEngine() : nullptr;
        if (!engine)
            return;
        std::uintptr_t tag = reinterpret_cast<std::uintptr_t>(engine);
        std::erase_if(state.Controls, [](auto const& control) { return !control->Attached.load(); });
        for (auto const& control : state.Controls)
            control->Active.store(true);
        if (state.Controls.size() == 4 && std::any_of(state.Controls.begin(), state.Controls.end(),
            [tag](auto const& control) { return control->EngineTag == tag; }))
            return;
        // Cover reaction/combat/death engines too: no normal gear, trade or movement action may run
        // between an engine switch and the next world tick while a persona owns the character.
        BotState previous = ai->GetState();
        for (BotState botState : {BOT_STATE_NON_COMBAT, BOT_STATE_COMBAT, BOT_STATE_DEAD, BOT_STATE_REACTION})
        {
            ai->ChangeEngine(botState);
            Engine* target = ai->GetCurrentEngine();
            if (!target)
                continue;
            std::uintptr_t targetTag = reinterpret_cast<std::uintptr_t>(target);
            if (std::any_of(state.Controls.begin(), state.Controls.end(),
                [targetTag](auto const& control) { return control->EngineTag == targetTag; }))
                continue;
            auto control = std::make_shared<CustomPauseControl>();
            control->EngineTag = targetTag;
            target->AddActionExecutionListener(new CustomPauseListener(control));
            state.Controls.push_back(std::move(control));
        }
        ai->ChangeEngine(previous);
    }

    bool CustomActivities::End(Player* player)
    {
        if (!player)
            return false;
        auto found = _states.find(player->GetGUID().GetCounter());
        if (found != _states.end() && found->second.Role == CustomRole::Beggar)
            sCityLifeBeggarTrade.End(player);
        if (found != _states.end() && !found->second.AppearanceReady && found->second.NextAction > Now())
            return false;
        if (found != _states.end())
            StopPractice(player, found->second);
        if (!sCityLifeCustomAppearance.End(player))
        {
            if (found != _states.end())
            {
                found->second.AppearanceReady = false;
                found->second.NextAction = Now() + 30;
            }
            return false;
        }
        Forget(player->GetGUID().GetCounter());
        return true;
    }

    void CustomActivities::Forget(uint32 guidLow)
    {
        sCityLifeBeggarTrade.Forget(guidLow);
        auto found = _states.find(guidLow);
        if (found == _states.end())
            return;
        for (auto const& control : found->second.Controls)
            control->Active.store(false);
        if (found->second.RandomizeHeld)
        {
            sRandomPlayerbotMgr.SetValue(guidLow, "randomize", found->second.OriginalRandomize);
            sRandomPlayerbotMgr.ScheduleTeleport(guidLow);
        }
        _idleControls[guidLow] = std::move(found->second.Controls);
        _states.erase(found);
    }

    void CustomActivities::RecoverPending()
    {
        if (Now() >= _nextControlPruneAt)
        {
            _nextControlPruneAt = Now() + MINUTE;
            for (auto entry = _idleControls.begin(); entry != _idleControls.end();)
            {
                std::erase_if(entry->second, [](auto const& control) { return !control->Attached.load(); });
                if (entry->second.empty())
                    entry = _idleControls.erase(entry);
                else
                    ++entry;
            }
        }
        for (uint32 guidLow : sCityLifeCustomAppearance.PendingGuids())
        {
            Player* player = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(guidLow));
            if (!player || sCityLifeCustomAppearance.IsApplied(player))
                continue;
            auto active = _states.find(guidLow);
            if (active != _states.end() && active->second.Role != CustomRole::None)
                continue;
            bool owned = PlayerbotsLife::IsReservedBy(guidLow, PlayerbotsLife::ReservationOwner::CustomLife);
            if (!owned && !PlayerbotsLife::TryReserve(guidLow, PlayerbotsLife::ReservationOwner::CustomLife))
                continue;
            State& quarantine = _states[guidLow];
            if (quarantine.Controls.empty())
                if (auto previous = _idleControls.find(guidLow); previous != _idleControls.end())
                {
                    quarantine.Controls = std::move(previous->second);
                    _idleControls.erase(previous);
                }
            PauseAI(player, quarantine);
            if (sRandomPlayerbotMgr.IsRandomBot(player))
            {
                if (!quarantine.RandomizeHeld)
                {
                    quarantine.RandomizeHeld = true;
                    quarantine.OriginalRandomize = sRandomPlayerbotMgr.GetValue(guidLow, "randomize");
                }
                if (Now() >= quarantine.NextHoldRefresh)
                {
                    sRandomPlayerbotMgr.SetValue(guidLow, "randomize", 1);
                    sRandomPlayerbotMgr.ScheduleTeleport(guidLow, 20 * MINUTE);
                    quarantine.NextHoldRefresh = Now() + MINUTE;
                }
            }
            if (Now() < quarantine.NextAction)
                continue;
            quarantine.NextAction = Now() + 30;
            if (sCityLifeCustomAppearance.RecoverOnLogin(player))
            {
                Forget(guidLow);
                PlayerbotsLife::Release(guidLow, PlayerbotsLife::ReservationOwner::CustomLife);
            }
        }
    }

    bool CustomActivities::IsActive(uint32 guidLow) const { return _states.contains(guidLow); }

    bool CustomActivities::CanBeggarWander(Player const* player) const
    {
        if (!player || !player->IsAlive() || player->IsBeingTeleported() || player->IsInCombat() ||
            player->GetGroup() || player->GetTradeData() || player->duel || player->isMoving() ||
            player->IsNonMeleeSpellCast(false) || player->InBattleground() || player->InBattlegroundQueue())
            return false;
        auto found = _states.find(player->GetGUID().GetCounter());
        return found != _states.end() && found->second.Role == CustomRole::Beggar &&
            _roles[static_cast<size_t>(CustomRole::Beggar)].Enabled && found->second.AppearanceReady &&
            !found->second.DanceUntil && Now() >= found->second.BusyUntil;
    }

    bool CustomActivities::IsTraining(uint32 guidLow) const
    {
        auto found = _states.find(guidLow);
        return found != _states.end() && found->second.Role == CustomRole::TrainingDummy;
    }

    bool CustomActivities::IsSafeCombat(Player* player) const
    {
        if (!player || !player->getAttackers().empty())
            return false;
        if (!player->IsInCombat())
            return true;
        auto found = _states.find(player->GetGUID().GetCounter());
        return found != _states.end() && found->second.Role == CustomRole::TrainingDummy &&
            (!player->GetVictim() || player->GetVictim()->GetGUID() == found->second.Dummy);
    }

    bool CustomActivities::HasObserver(Player* player, float range) const
    {
        if (!player->GetMap())
            return false;
        for (auto const& reference : player->GetMap()->GetPlayers())
        {
            Player* observer = reference.GetSource();
            if (observer && ::IsRealPlayer(observer) && observer->IsInWorld() &&
                !observer->IsBeingTeleported() && player->InSamePhase(observer) &&
                player->GetTeamId() == observer->GetTeamId() && player->IsWithinDistInMap(observer, range))
                return true;
        }
        return false;
    }

    Creature* CustomActivities::FindDummy(Player* player) const
    {
        std::list<Creature*> targets;
        player->GetCreatureListWithEntryInGrid(targets, _dummyEntries, _dummyRange);
        uint32 scriptId = sObjectMgr->GetScriptId("npc_training_dummy");
        Creature* nearest = nullptr;
        for (Creature* target : targets)
            if (scriptId && target->GetScriptId() == scriptId && target->IsAlive() &&
                target->InSamePhase(player) && player->IsValidAttackTarget(target) &&
                player->IsWithinLOSInMap(target) && (!nearest || player->GetDistance(target) <
                    player->GetDistance(nearest)))
                nearest = target;
        return nearest;
    }

    void CustomActivities::StopPractice(Player* player, State& state)
    {
        player->InterruptNonMeleeSpells(false);
        player->ClearEmoteState();
        player->SetStandState(UNIT_STAND_STATE_STAND);
        if (state.Dummy)
        {
            if (player->GetVictim() && player->GetVictim()->GetGUID() == state.Dummy)
                player->AttackStop();
            if (player->getAttackers().empty())
                player->CombatStop(false);
            state.Dummy.Clear();
        }
        state.PracticeUntil = 0;
    }

    void CustomActivities::Practice(Player* player, State& state, uint32 now)
    {
        if (state.PracticeUntil && now >= state.PracticeUntil)
        {
            StopPractice(player, state);
            state.RestUntil = now + urand(_restMin, _restMax);
            state.NextAction = state.RestUntil;
            player->SetStandState(UNIT_STAND_STATE_SIT);
            player->SetEmoteState(EMOTE_STATE_EAT);
            return;
        }
        if (now < state.NextAction)
            return;
        player->SetStandState(UNIT_STAND_STATE_STAND);
        player->ClearEmoteState();
        Creature* dummy = state.Dummy ? ObjectAccessor::GetCreature(*player, state.Dummy) : FindDummy(player);
        if (!dummy || !dummy->IsAlive() || !dummy->InSamePhase(player) ||
            !player->IsWithinDistInMap(dummy, _dummyRange) || !player->IsValidAttackTarget(dummy) ||
            !player->IsWithinLOSInMap(dummy))
        {
            StopPractice(player, state);
            state.NextAction = now + 15;
            return;
        }
        state.Dummy = dummy->GetGUID();
        if (!state.PracticeUntil)
            state.PracticeUntil = now + urand(_practiceMin, _practiceMax);
        player->SetFacingToObject(dummy);
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        if (ai && !player->IsNonMeleeSpellCast(false))
        {
            char const* spell = player->getClass() == CLASS_HUNTER && player->IsWithinMeleeRange(dummy) ?
                "raptor strike" : PracticeSpell(player->getClass());
            if (ai->HasSpell(spell) && ai->CanCastSpell(spell, dummy))
                ai->CastSpell(spell, dummy);
            if ((PlayerbotAI::IsMelee(player) || player->getClass() == CLASS_HUNTER) &&
                player->IsWithinMeleeRange(dummy))
            {
                player->Attack(dummy, true);
                if (player->isAttackReady(BASE_ATTACK))
                {
                    player->AttackerStateUpdate(dummy, BASE_ATTACK);
                    player->resetAttackTimer(BASE_ATTACK);
                }
            }
        }
        state.NextAction = now + 2;
    }

    bool CustomActivities::FindFishingWater(Player* player, uint32 mapId, float x, float y, float z,
        float& facing) const
    {
        Map* map = player->GetMapId() == mapId ? player->GetMap() : sMapMgr->CreateBaseMap(mapId);
        if (!map)
            return false;
        constexpr float twoPi = 6.28318530718f;
        for (uint32 direction = 0; direction < 16; ++direction)
        {
            float angle = facing + twoPi * direction / 16;
            for (float distance : {12.0f, 16.0f, 19.0f})
            {
                float waterX = x + std::cos(angle) * distance;
                float waterY = y + std::sin(angle) * distance;
                LiquidData liquid = map->GetLiquidData(player->GetPhaseMask(), waterX, waterY,
                    z + 2.0f, player->GetCollisionHeight(), MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN);
                if (liquid.Entry == MAP_LIQUID_TYPE_NO_WATER || liquid.Level - liquid.DepthLevel < 0.75f ||
                    std::abs(liquid.Level - z) > _fishMaximumWaterHeightDifference ||
                    !map->isInLineOfSight(x, y, z + player->GetCollisionHeight(), waterX, waterY,
                        liquid.Level + player->GetCollisionHeight(), player->GetPhaseMask(),
                        LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
                    continue;
                facing = angle;
                return true;
            }
        }
        return false;
    }

    void CustomActivities::Fish(Player* player, State& state, uint32 now)
    {
        if (now < state.NextAction || player->IsNonMeleeSpellCast(false))
            return;
        state.NextAction = now + urand(_fishMin, _fishMax);
        float facing = player->GetOrientation();
        if (!sSpellMgr->GetSpellInfo(_fishSpell) || !FindFishingWater(player, player->GetMapId(),
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), facing))
            return;
        player->SetFacingTo(facing);
        player->SetSheath(SHEATH_STATE_MELEE);
        // Ambient fishing casts no learned spell/skill and never loots or grants fishing skill.
        player->CastSpell(player, _fishSpell, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
    }

    void CustomActivities::Chat(Player* player, State& state, uint32 now)
    {
        if (now < state.NextChat)
            return;
        RoleSettings const& settings = _roles[static_cast<size_t>(state.Role)];
        state.NextChat = now + urand(settings.ChatMin, settings.ChatMax);
        std::string group;
        if (state.Role == CustomRole::Beggar)
            group = "beggar";
        else if (state.Role == CustomRole::Buyer)
            group = "buyer";
        else if (state.Role == CustomRole::Fishing)
            group = "fishing";
        float range = state.Role == CustomRole::Fishing ? _fishingChatRange : _chatRange;
        if (group.empty() || !HasObserver(player, range))
            return;
        TeamSide team = player->GetTeamId() == TEAM_ALLIANCE ? TeamSide::Alliance : TeamSide::Horde;
        std::vector<Line const*> choices;
        uint64 total = 0;
        for (Line const& line : _lines)
            if (line.Group == group && (line.Team == team || line.Team == TeamSide::Any))
            {
                choices.push_back(&line);
                total += line.Weight;
            }
        if (choices.empty())
            return;
        // Bound the random range even if a custom table contains many weighted lines.
        uint64 pick = static_cast<uint64>(frand(0.0f, 1.0f) * static_cast<double>(total));
        Line const* chosen = choices.back();
        for (Line const* line : choices)
        {
            if (pick < line->Weight)
            {
                chosen = line;
                break;
            }
            pick -= line->Weight;
        }
        if (PlayerbotAI* ai = GET_PLAYERBOT_AI(player))
        {
            if (urand(1, 100) <= settings.YellChance)
                ai->Yell(chosen->Text);
            else
                ai->Say(chosen->Text);
        }
        if (state.Role == CustomRole::Beggar)
        {
            player->HandleEmoteCommand(EMOTE_ONESHOT_BEG);
            state.BusyUntil = now + 3;
        }
    }

    void CustomActivities::Update(Player* player, Resident const& resident)
    {
        auto found = _states.find(resident.Bot.GuidLow);
        if (found == _states.end())
            return;
        State& state = found->second;
        uint32 now = Now();
        if (state.Role == CustomRole::Beggar)
        {
            bool allowRequest = _roles[static_cast<size_t>(CustomRole::Beggar)].Enabled &&
                state.AppearanceReady && !resident.Travelling && resident.RetryMoveAt == 0 &&
                !state.DanceUntil && now >= state.BusyUntil && player->GetMapId() == resident.TargetMap &&
                player->GetExactDist2d(resident.TargetX, resident.TargetY) <= 3.0f &&
                std::abs(player->GetPositionZ() - resident.TargetZ) <= 3.0f;
            // Existing invitations still expire while the bot moves, teleports or restores its outfit.
            sCityLifeBeggarTrade.Update(player, allowRequest);
        }
        if (!_roles[static_cast<size_t>(state.Role)].Enabled)
        {
            End(player);
            return;
        }
        PauseAI(player, state);
        if (!state.AppearanceReady)
        {
            End(player);
            return;
        }
        if (player->IsBeingTeleported() || !player->IsAlive() || player->GetGroup() ||
            player->InBattlegroundQueue() || player->InBattleground() || player->duel ||
            player->GetTradeData() ||
            player->GetMapId() != resident.TargetMap || player->isMoving() ||
            player->GetExactDist2d(resident.TargetX, resident.TargetY) > 3.0f ||
            std::abs(player->GetPositionZ() - resident.TargetZ) > 3.0f)
            return;
        sCityLifeCustomAppearance.Update(player, state.Role);
        if (state.Role == CustomRole::TrainingDummy)
            Practice(player, state, now);
        else if (!player->IsInCombat())
        {
            if (state.Role == CustomRole::Fishing)
                Fish(player, state, now);
            else if (state.Role == CustomRole::Beggar && now >= state.NextAction)
            {
                if (state.DanceUntil)
                {
                    player->ClearEmoteState();
                    state.DanceUntil = 0;
                    state.BusyUntil = now + 3;
                    state.NextAction = now + urand(_beggarEmoteMin, _beggarEmoteMax);
                }
                else if (urand(1, 100) <= _beggarDanceChance)
                {
                    player->SetEmoteState(EMOTE_STATE_DANCE);
                    state.DanceUntil = now + _beggarDanceSeconds;
                    state.NextAction = state.DanceUntil;
                }
                else
                {
                    player->HandleEmoteCommand(EMOTE_ONESHOT_BEG);
                    state.BusyUntil = now + 3;
                    state.NextAction = now + urand(_beggarEmoteMin, _beggarEmoteMax);
                }
            }
            Chat(player, state, now);
        }
    }
}
