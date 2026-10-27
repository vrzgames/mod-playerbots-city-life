/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_HUMAN_BEHAVIOR_H
#define CITY_LIFE_HUMAN_BEHAVIOR_H

#include "Define.h"
#include "ObjectGuid.h"

#include <array>
#include <cstddef>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;

namespace CityLife
{
    struct HumanBehaviorBot
    {
        uint32 Guid = 0;
        bool AllowIdleActions = false;
    };

    class HumanBehavior
    {
    public:
        static HumanBehavior& Instance();

        void LoadConfig();
        // World-thread scheduler; candidates are existing City Life residents, never a new bot pool.
        void Update(std::vector<HumanBehaviorBot> const& candidates);
        void Remove(uint32 guid);
        void Reset();
        // Also called by the combat hook: only the small sit-ownership set is shared with map threads.
        void Wake(Player* player);

    private:
        enum Behavior : uint8
        {
            Inspect,
            WeaponSheath,
            Sit,
            Companion,
            BehaviorCount
        };

        struct Feature
        {
            bool Enabled = false;
            uint32 Chance = 0;
            uint32 MinSeconds = 120;
            uint32 MaxSeconds = 300;
        };

        struct State
        {
            std::array<uint32, BehaviorCount> NextAt = {};
            uint32 IdleSince = 0;
            uint32 SitUntil = 0;
            uint32 CompanionUntil = 0;
            ObjectGuid Critter;
        };

        HumanBehavior() = default;

        bool IsEligible(Player* player) const;
        bool IsIdle(Player* player) const;
        bool CanObserve(Player* observer, Player* bot) const;
        bool IsSitting(uint32 guid);
        void DismissCompanion(Player* player, State& state);
        void TryInspect(Player* player, std::vector<Player*> const& nearby) const;
        void TrySheath(Player* player) const;
        void TryCompanion(Player* player, State& state, uint32 now) const;

        bool _enabled = false;
        uint32 _checkIntervalSeconds = 10;
        uint32 _maxBotsPerCheck = 32;
        float _observerRange = 60.0f;
        float _inspectRange = 20.0f;
        uint32 _idleSeconds = 15;
        uint32 _sitMinSeconds = 8;
        uint32 _sitMaxSeconds = 20;
        uint32 _companionMinSeconds = 60;
        uint32 _companionMaxSeconds = 180;
        uint32 _nextCheckAt = 0;
        std::size_t _cursor = 0;
        std::array<Feature, BehaviorCount> _features;
        std::unordered_map<uint32, State> _states;
        std::mutex _sittingMutex;
        std::unordered_set<uint32> _sittingBots;
    };
}

#define sCityLifeHumanBehavior CityLife::HumanBehavior::Instance()

#endif
