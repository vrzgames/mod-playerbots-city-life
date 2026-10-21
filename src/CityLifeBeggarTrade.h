/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_BEGGAR_TRADE_H
#define CITY_LIFE_BEGGAR_TRADE_H

#include "Define.h"
#include "ObjectGuid.h"

#include <unordered_map>

class Player;

namespace CityLife
{
    // World-thread-only ambient requests. Normal Playerbot trade actions stay paused throughout.
    class BeggarTrade
    {
    public:
        static BeggarTrade& Instance();

        void LoadConfig();
        void Begin(Player* player);
        void End(Player* player);
        void Forget(uint32 guidLow);
        // Always maintain an existing request; allowRequest permits a new request only while idle.
        void Update(Player* player, bool allowRequest = true);
        bool IsInitiating(Player const* player, Player const* target) const;

    private:
        struct Request
        {
            ObjectGuid Partner;
            uint32 Until = 0;
        };

        bool CanParticipate(Player* player) const;
        bool IsEligibleTarget(Player* player, Player* target) const;
        void Cancel(uint32 guidLow, Player* player, Request& request);
        void PruneCooldowns(uint32 now);

        bool _enabled = true;
        uint32 _minSeconds = 180;
        uint32 _maxSeconds = 420;
        uint32 _chancePct = 25;
        float _range = 8.0f;
        uint32 _durationSeconds = 15;
        uint32 _playerCooldownSeconds = 600;
        uint32 _nextPruneAt = 0;
        ObjectGuid _initiatingBot;
        ObjectGuid _initiatingTarget;
        std::unordered_map<uint32, Request> _beggars;
        std::unordered_map<uint32, uint32> _nextBotRequestAt;
        std::unordered_map<uint32, uint32> _nextPlayerRequestAt;
    };
}

#define sCityLifeBeggarTrade CityLife::BeggarTrade::Instance()

#endif
