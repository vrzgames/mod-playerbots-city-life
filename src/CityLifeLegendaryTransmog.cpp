/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeLegendaryTransmog.h"

#if defined(CITY_LIFE_TRANSMOG_BRIDGE)
#include "Transmogrification.h"
#endif

#include <limits>

namespace CityLife::LegendaryTransmog
{
    bool Available()
    {
#if defined(CITY_LIFE_TRANSMOG_API_VERSION) && CITY_LIFE_TRANSMOG_API_VERSION >= 1
        return sTransmogrification->IsEnabled();
#else
        return false;
#endif
    }

    bool RegisterTemporary(Player const* player, Item const* item)
    {
#if defined(CITY_LIFE_TRANSMOG_API_VERSION) && CITY_LIFE_TRANSMOG_API_VERSION >= 1
        return sTransmogrification->RegisterCityLifeTemporary(player, item);
#else
        (void)player;
        (void)item;
        return false;
#endif
    }

    bool Apply(Player* player, Item* item, uint8 slot, uint32 skin)
    {
#if defined(CITY_LIFE_TRANSMOG_API_VERSION) && CITY_LIFE_TRANSMOG_API_VERSION >= 1
        uint32 const entry = skin == std::numeric_limits<uint32>::max() ? HIDDEN_ITEM_ID : skin;
        return sTransmogrification->SetCityLifeTemporaryAppearance(player, item, slot, entry);
#else
        (void)player;
        (void)item;
        (void)slot;
        (void)skin;
        return false;
#endif
    }

    void ForgetTemporary(Player* player, uint32 itemGuid)
    {
#if defined(CITY_LIFE_TRANSMOG_API_VERSION) && CITY_LIFE_TRANSMOG_API_VERSION >= 1
        sTransmogrification->ForgetCityLifeTemporary(player, ObjectGuid::Create<HighGuid::Item>(itemGuid));
#else
        (void)player;
        (void)itemGuid;
#endif
    }
}
