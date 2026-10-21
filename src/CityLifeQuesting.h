/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_QUESTING_H
#define CITY_LIFE_QUESTING_H

#include <atomic>
#include <memory>
#include <vector>

#include "Define.h"

class Player;

namespace CityLife
{
    struct QuestingWaypoint
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    struct QuestingArea
    {
        uint32 MapId = 0;
        uint32 ZoneId = 0;
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
        float O = 0.0f;
        float Radius = 1500.0f;
        std::vector<QuestingWaypoint> Waypoints;
    };

    struct QuestingControl
    {
        std::atomic<std::shared_ptr<QuestingArea const>> Area;
        std::atomic<bool> Attached{false};
        std::atomic<uint32> LastActivityAt{0};
    };

    std::shared_ptr<QuestingControl> StartQuesting(Player* player, QuestingArea const& area,
                                                   std::weak_ptr<QuestingControl> const& previous);
    void StopQuesting(std::shared_ptr<QuestingControl> const& control);
}  // namespace CityLife

#endif
