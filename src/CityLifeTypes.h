/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_TYPES_H
#define CITY_LIFE_TYPES_H

#include "Define.h"

#include <string>
#include <vector>

namespace CityLife
{
    enum class TeamSide : uint8
    {
        Alliance = 0,
        Horde = 1,
        Any = 2
    };

    struct Spot
    {
        uint32 Id = 0;
        uint32 HubId = 0;
        std::string Name;
        std::string Category;
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
        float O = 0.0f;
        float Radius = 3.0f;
        uint32 Weight = 100;
    };

    struct Hub
    {
        uint32 Id = 0;
        std::string Name;
        bool Enabled = true;
        TeamSide Team = TeamSide::Any;
        uint8 MinLevel = 1;
        uint8 MaxLevel = 80;
        uint32 MapId = 0;
        uint32 DefaultPopulation = 0;
        uint32 ConfiguredPopulation = 0;
        uint32 Priority = 100;
        std::vector<Spot> Spots;
    };

    struct BotCandidate
    {
        uint32 GuidLow = 0;
        uint32 AccountId = 0;
        std::string Name;
        uint8 Level = 1;
        uint8 Race = 0;
        TeamSide Team = TeamSide::Any;
    };

    struct Resident
    {
        BotCandidate Bot;
        uint32 HubId = 0;
        uint32 SpotId = 0;
        uint32 TargetMap = 0;
        uint32 OriginalMap = 0;
        float OriginalX = 0.0f;
        float OriginalY = 0.0f;
        float OriginalZ = 0.0f;
        float OriginalO = 0.0f;
        float TargetX = 0.0f;
        float TargetY = 0.0f;
        float TargetZ = 0.0f;
        float TargetO = 0.0f;
        uint32 NextMoveAt = 0;
        uint32 NextEmoteAt = 0;
        uint32 NextReservationRefreshAt = 0;
        bool Stationary = false;
    };
}

#endif
