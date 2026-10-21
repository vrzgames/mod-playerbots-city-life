/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_CUSTOM_ACTIVITIES_H
#define CITY_LIFE_CUSTOM_ACTIVITIES_H

#include "CityLifeTypes.h"
#include "ObjectGuid.h"

#include <array>
#include <memory>
#include <unordered_map>

class Creature;
class Player;

namespace CityLife
{
    struct CustomPauseControl;

    class CustomActivities
    {
    public:
        static CustomActivities& Instance();

        void LoadConfig();
        void LoadData();
        void ConfigureHub(Hub& hub) const;
        bool CanUse(Player* player, CustomRole role) const;
        bool Begin(Player* player, CustomRole role);
        bool End(Player* player);
        void Forget(uint32 guidLow);
        void Update(Player* player, Resident const& resident);
        void RecoverPending();
        bool IsActive(uint32 guidLow) const;
        bool IsTraining(uint32 guidLow) const;
        bool IsSafeCombat(Player* player) const;
        bool CanBeggarWander(Player const* player) const;
        bool FindFishingWater(Player* player, uint32 mapId, float x, float y, float z, float& facing) const;

    private:
        struct RoleSettings
        {
            bool Enabled = true;
            uint8 MinLevel = 1;
            uint8 MaxLevel = 80;
            uint32 ChatMin = 180;
            uint32 ChatMax = 420;
            uint32 YellChance = 0;
        };

        struct Line
        {
            std::string Group;
            TeamSide Team = TeamSide::Any;
            std::string Text;
            uint32 Weight = 100;
        };

        struct State
        {
            CustomRole Role = CustomRole::None;
            bool AppearanceReady = false;
            uint32 NextChat = 0;
            uint32 NextAction = 0;
            uint32 PracticeUntil = 0;
            uint32 RestUntil = 0;
            uint32 DanceUntil = 0;
            uint32 BusyUntil = 0;
            uint32 NextHoldRefresh = 0;
            uint32 OriginalRandomize = 0;
            bool RandomizeHeld = false;
            ObjectGuid Dummy;
            std::vector<std::shared_ptr<CustomPauseControl>> Controls;
        };

        void PauseAI(Player* player, State& state);
        void StopPractice(Player* player, State& state);
        void Practice(Player* player, State& state, uint32 now);
        void Fish(Player* player, State& state, uint32 now);
        void Chat(Player* player, State& state, uint32 now);
        Creature* FindDummy(Player* player) const;
        bool HasObserver(Player* player, float range) const;

        bool _enabled = true;
        bool _timeScaling = false;
        uint32 _residenceSeconds = 0;
        float _chatRange = 40.0f;
        bool _fishingTimeScaling = false;
        uint32 _fishingResidenceSeconds = 0;
        float _fishingChatRange = 40.0f;
        float _dummyRange = 15.0f;
        uint32 _practiceMin = 60;
        uint32 _practiceMax = 180;
        uint32 _restMin = 15;
        uint32 _restMax = 45;
        uint32 _fishMin = 20;
        uint32 _fishMax = 35;
        uint32 _fishSpell = 7620;
        float _fishMaximumWaterHeightDifference = 12.0f;
        uint32 _beggarEmoteMin = 45;
        uint32 _beggarEmoteMax = 120;
        uint32 _beggarDanceChance = 25;
        uint32 _beggarDanceSeconds = 8;
        std::vector<uint32> _dummyEntries;
        std::array<RoleSettings, 6> _roles;
        std::vector<Line> _lines;
        std::unordered_map<uint32, State> _states;
        std::unordered_map<uint32, std::vector<std::shared_ptr<CustomPauseControl>>> _idleControls;
        uint32 _nextControlPruneAt = 0;
    };
}

#define sCityLifeCustomActivities CityLife::CustomActivities::Instance()

#endif
