/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_LEGENDARY_TRANSMOG_H
#define CITY_LIFE_LEGENDARY_TRANSMOG_H

#include "Define.h"

class Item;
class Player;

namespace CityLife::LegendaryTransmog
{
    bool Available();
    bool RegisterTemporary(Player const* player, Item const* item);
    bool Apply(Player* player, Item* item, uint8 slot, uint32 skin);
    // Cleanup remains available after transmog is disabled; only registered temporary GUIDs are removed.
    void ForgetTemporary(Player* player, uint32 itemGuid);
}

#endif
