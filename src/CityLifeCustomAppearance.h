/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_CUSTOM_APPEARANCE_H
#define CITY_LIFE_CUSTOM_APPEARANCE_H

#include "CityLifeTypes.h"

#include <map>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class Item;
class Player;

namespace CityLife
{
    // Only call these methods on the world thread. Inventory and journal changes must be serialized.
    class CustomAppearance
    {
    public:
        static CustomAppearance& Instance();

        void LoadConfig();
        bool CanUse(Player* player, CustomRole role) const;
        bool CanCreateLegendary(uint8 classId, uint8 level) const;
        bool Begin(Player* player, CustomRole role);
        void Update(Player* player, CustomRole role);
        bool End(Player* player);
        bool RecoverOnLogin(Player* player);
        bool HasPendingRecovery(Player const* player) const;
        bool IsApplied(Player const* player) const;
        std::vector<uint32> PendingGuids() const;
        bool IsTemporaryItem(Player const* player, Item const* item) const;
        bool IsReady() const { return _databaseReady; }

    private:
        struct ItemBonuses
        {
            uint32 Enchant = 0;
            std::array<uint32, 3> Gems{};
            uint32 Bonus = 0;
            uint32 Prismatic = 0;
        };

        struct Outfit
        {
            std::map<uint8, uint32> Items;
            std::map<uint8, ItemBonuses> Bonuses;
            uint32 RaceMask = 0;
        };

        struct EquipmentRecord
        {
            uint8 Slot = 0;
            uint32 OriginalGuid = 0;
            uint32 TemporaryGuid = 0;
            uint32 TemporaryEntry = 0;
        };

        struct AppearanceState
        {
            CustomRole Role = CustomRole::None;
            uint32 MountDisplayId = 0;
            uint8 OriginalSheath = 0;
            bool Applied = false;
        };

        bool IsOutfitUsable(Player* player, Outfit const& outfit) const;
        bool KeepOriginal(Player const* player, Outfit const& outfit, uint8 slot, Item const* item) const;
        bool AddTemplateOutfits(uint8 classId, std::string const& profile, Outfit const& iconic);
        void ApplyBonuses(Player* player, Item* item, ItemBonuses const& bonuses) const;
        std::vector<Outfit const*> UsableOutfits(Player* player, CustomRole role) const;
        Outfit const* FishingOutfit() const;
        bool RestoreEquipment(Player* player, std::vector<EquipmentRecord> const& records);
        bool JournalExists(uint32 guidLow) const;

        bool _databaseReady = false;
        bool _templateGearEnabled = true;
        bool _enchantsEnabled = true;
        bool _gemsEnabled = true;
        uint32 _mountDisplayId = 21974;
        Outfit _fishingOutfit;
        std::unordered_map<uint8, std::vector<Outfit>> _legendaryOutfits;
        std::unordered_map<uint32, std::vector<EquipmentRecord>> _pending;
        std::unordered_map<uint32, AppearanceState> _active;
    };
}

#define sCityLifeCustomAppearance CityLife::CustomAppearance::Instance()

#endif
