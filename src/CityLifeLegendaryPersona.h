/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_LEGENDARY_PERSONA_H
#define CITY_LIFE_LEGENDARY_PERSONA_H

#include "Define.h"

#include <string>
#include <vector>

class Guild;
class Player;

namespace CityLife
{
    // World-thread initialization for registered, module-owned Legendary characters only.
    class LegendaryPersona
    {
    public:
        static LegendaryPersona& Instance();

        void LoadConfig();
        // Saves achievement history internally. The caller saves the player's HK field afterwards.
        // On failure, discard the owned session before retrying from its durable character state.
        bool Apply(Player* player);

    private:
        bool ApplyAchievements(Player* player, uint32 targetPoints);
        bool RefreshAchievements(Player* player) const;
        bool IsOwnedGuild(Guild const* guild) const;
        void ApplyGuild(Player* player);

        bool _enabled = true;
        bool _honorEnabled = true;
        bool _achievementsEnabled = true;
        bool _randomizeHonorableKills = true;
        bool _randomizeAchievementPoints = true;
        bool _guildEnabled = true;
        bool _gearReputation = true;
        bool _defaultAchievements = true;
        bool _guildWarningLogged = false;
        bool _achievementWarningLogged = false;
        uint32 _honorableKills = 10000;
        uint32 _achievementPoints = 5000;
        uint32 _honorableKillsMin = 11000;
        uint32 _honorableKillsMax = 25000;
        uint32 _achievementPointsMin = 5100;
        uint32 _achievementPointsMax = 6000;
        std::string _guildName = "The GOATS";
        std::vector<uint32> _achievementIds;
    };
}

#define sCityLifeLegendaryPersona CityLife::LegendaryPersona::Instance()

#endif
