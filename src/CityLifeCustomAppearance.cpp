/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeCustomAppearance.h"
#include "CityLifeLegendaryGear.h"
#include "CityLifeLegendarySkins.h"
#include "CityLifeLegendaryTransmog.h"

#include "Bag.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SharedDefines.h"
#include "UpdateFields.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace
{
    uint16 EquipmentPosition(uint8 slot)
    {
        return static_cast<uint16>((INVENTORY_SLOT_BAG_0 << 8) | slot);
    }

    std::vector<uint16> FreeGeneralSlots(Player const* player)
    {
        std::vector<uint16> slots;
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
            if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                slots.push_back(EquipmentPosition(slot));
        for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
            if (Bag* bag = player->GetBagByPos(bagSlot))
                if (bag->GetTemplate()->Class == ITEM_CLASS_CONTAINER &&
                    bag->GetTemplate()->SubClass == ITEM_SUBCLASS_CONTAINER && bag->GetTemplate()->BagFamily == 0)
                    for (uint8 slot = 0; slot < bag->GetBagSize(); ++slot)
                        if (!bag->GetItemByPos(slot))
                            slots.push_back(static_cast<uint16>((bagSlot << 8) | slot));
        return slots;
    }

    bool NeedsEquipment(CityLife::CustomRole role)
    {
        return role == CityLife::CustomRole::Legendary || role == CityLife::CustomRole::Beggar ||
            role == CityLife::CustomRole::Fishing;
    }

    uint32 MountChoice(uint32 guid)
    {
        uint32 value = guid ^ 0x4D4F554Eu;
        value ^= value >> 16;
        value *= 0x7FEB352Du;
        value ^= value >> 15;
        value *= 0x846CA68Bu;
        return value ^ (value >> 16);
    }

    uint8 OutfitSlot(ItemTemplate const* item, std::map<uint8, uint32> const& outfit)
    {
        switch (item->InventoryType)
        {
            case INVTYPE_HEAD: return EQUIPMENT_SLOT_HEAD;
            case INVTYPE_NECK: return EQUIPMENT_SLOT_NECK;
            case INVTYPE_SHOULDERS: return EQUIPMENT_SLOT_SHOULDERS;
            case INVTYPE_BODY: return EQUIPMENT_SLOT_BODY;
            case INVTYPE_CHEST:
            case INVTYPE_ROBE: return EQUIPMENT_SLOT_CHEST;
            case INVTYPE_WAIST: return EQUIPMENT_SLOT_WAIST;
            case INVTYPE_LEGS: return EQUIPMENT_SLOT_LEGS;
            case INVTYPE_FEET: return EQUIPMENT_SLOT_FEET;
            case INVTYPE_WRISTS: return EQUIPMENT_SLOT_WRISTS;
            case INVTYPE_HANDS: return EQUIPMENT_SLOT_HANDS;
            case INVTYPE_FINGER:
                return outfit.count(EQUIPMENT_SLOT_FINGER1) ? EQUIPMENT_SLOT_FINGER2 : EQUIPMENT_SLOT_FINGER1;
            case INVTYPE_TRINKET:
                return outfit.count(EQUIPMENT_SLOT_TRINKET1) ? EQUIPMENT_SLOT_TRINKET2 : EQUIPMENT_SLOT_TRINKET1;
            case INVTYPE_CLOAK: return EQUIPMENT_SLOT_BACK;
            case INVTYPE_WEAPON:
                return outfit.count(EQUIPMENT_SLOT_MAINHAND) ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND;
            case INVTYPE_WEAPONMAINHAND:
            case INVTYPE_2HWEAPON: return EQUIPMENT_SLOT_MAINHAND;
            case INVTYPE_WEAPONOFFHAND:
            case INVTYPE_SHIELD:
            case INVTYPE_HOLDABLE: return EQUIPMENT_SLOT_OFFHAND;
            case INVTYPE_RANGED:
            case INVTYPE_RANGEDRIGHT:
            case INVTYPE_THROWN:
            case INVTYPE_RELIC: return EQUIPMENT_SLOT_RANGED;
            case INVTYPE_TABARD: return EQUIPMENT_SLOT_TABARD;
            default: return EQUIPMENT_SLOT_END;
        }
    }

    struct DefaultOutfits
    {
        uint8 Class;
        char const* Name;
        char const* Items;
        char const* Profiles;
    };

    // Entries and class restrictions are verified against the local 3.3.5 WORLD item templates.
    DefaultOutfits const Defaults[] =
    {
        {CLASS_WARRIOR, "Warrior", "51227,51229,51225,51226,51228,32837,32838;51227,51229,51225,51226,51228,49623",
            "Fury80PvEP4BiS;Arms80PvEP4BiS"},
        {CLASS_PALADIN, "Paladin", "51277,51279,51275,51276,51278,49623;51272,51273,51274,51270,51271,46017",
            "Retribution80PvEP4BiS;Holy80PvEP4BiSHeal"},
        {CLASS_HUNTER, "Hunter", "51286,51288,51289,51285,51287,34334;51153,51151,51150,51154,51152,34334",
            "Marksmanship80PvEP4BiS;Survival80PvEP4BiS"},
        {CLASS_ROGUE, "Rogue", "51252,51254,51250,51251,51253,32837,32838;51187,51185,51189,51188,51186,32837,32838",
            "Combat80PvEP4BiS;Assassination80PvEP4BiS"},
        {CLASS_PRIEST, "Priest", "51261,51264,51263,51260,51262,46017;51256,51257,51259,51255,51258,22631",
            "Discipline80PvEP4BiSHeal;Shadow80PvEP4BiS"},
        {CLASS_DEATH_KNIGHT, "DeathKnight", "51312,51314,51310,51311,51313,49623;51127,51125,51129,51128,51126,49623",
            "Unholy80PvEP4BiS;Frost80PvEP4BiS"},
        {CLASS_SHAMAN, "Shaman", "51247,51245,51249,51248,51246,46017;51192,51194,51190,51191,51193,46017",
            "Restoration80PvEP4BiSHeal;Elemental80PvEP4BiS"},
        {CLASS_MAGE, "Mage", "51281,51284,51283,51280,51282,22589;51158,51155,51156,51159,51157,22589",
            "Arcane80PvEP4BiS;Fire80PvEP4BiS"},
        {CLASS_WARLOCK, "Warlock", "51231,51234,51233,51230,51232,22630;51208,51205,51206,51209,51207,22630",
            "Affliction80PvEP4BiS;Destruction80PvEP4BiS"},
        {CLASS_DRUID, "Druid", "51290,51292,51294,51291,51293,46017;51302,51304,51300,51301,51303,22632",
            "Restoration80PvEP4BiSHeal;Balance80PvEP4BiS"}
    };
}

namespace CityLife
{
    CustomAppearance& CustomAppearance::Instance()
    {
        static CustomAppearance instance;
        return instance;
    }

    void CustomAppearance::LoadConfig()
    {
        _templateGearEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.TemplateGear.Enable", true);
        _enchantsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Enchants.Enable", true);
        _gemsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Gems.Enable", true);
        _skinsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Skins.Enable", true);
        _armorSkinsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Skins.Armor.Enable", true);
        _weaponSkinsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Skins.Weapons.Enable", true);
        _hideSkinOffhand = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Skins.HideOffhand", true);
        _mountDisplayId = sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.MountDisplayId", 21974);
        if (_mountDisplayId && !sCreatureDisplayInfoStore.LookupEntry(_mountDisplayId))
        {
            LOG_ERROR("module", "[CityLife.Custom] Invalid mount display {}; mount display disabled.", _mountDisplayId);
            _mountDisplayId = 0;
        }
        _randomizeMount = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Mount.Randomize", true);
        _mountDisplayIds.clear();
        std::string mounts = sConfigMgr->GetOption<std::string>(
            "CityLife.Custom.Legendary.Mount.DisplayIds", "21974,15676");
        std::replace(mounts.begin(), mounts.end(), ',', ' ');
        std::istringstream mountList(mounts);
        for (uint32 display; mountList >> display;)
        {
            if (!display || !sCreatureDisplayInfoStore.LookupEntry(display))
            {
                LOG_WARN("module", "[CityLife.Custom] Invalid randomized mount display {}; skipped.", display);
                continue;
            }
            if (std::find(_mountDisplayIds.begin(), _mountDisplayIds.end(), display) == _mountDisplayIds.end())
                _mountDisplayIds.push_back(display);
        }
        if (!mountList.eof())
        {
            LOG_WARN("module", "[CityLife.Custom] Invalid Mount.DisplayIds list; using the fixed mount display.");
            _mountDisplayIds.clear();
        }

        _legendaryOutfits.clear();
        for (DefaultOutfits const& entry : Defaults)
        {
            std::string configured = sConfigMgr->GetOption<std::string>(
                std::string("CityLife.Custom.Legendary.") + entry.Name + ".Outfits", "default");
            bool const useTemplate = _templateGearEnabled && configured == "default";
            if (configured == "default")
                configured = entry.Items;
            std::istringstream alternatives(configured);
            std::istringstream profiles(entry.Profiles);
            std::string alternative;
            while (std::getline(alternatives, alternative, ';'))
            {
                std::string profile;
                std::getline(profiles, profile, ';');
                Outfit outfit;
                bool valid = true;
                std::replace(alternative.begin(), alternative.end(), ',', ' ');
                std::istringstream items(alternative);
                uint32 itemId = 0;
                while (items >> itemId)
                {
                    ItemTemplate const* item = sObjectMgr->GetItemTemplate(itemId);
                    if (!item || item->Duration || item->Class == ITEM_CLASS_CONTAINER)
                    {
                        valid = false;
                        break;
                    }
                    uint8 slot = OutfitSlot(item, outfit.Items);
                    if (slot >= EQUIPMENT_SLOT_END || outfit.Items.count(slot) ||
                        (item->AllowableClass & (1 << (entry.Class - 1))) == 0)
                    {
                        valid = false;
                        break;
                    }
                    outfit.Items[slot] = itemId;
                }
                if (!items.eof())
                    valid = false;
                auto mainHand = outfit.Items.find(EQUIPMENT_SLOT_MAINHAND);
                if (valid && mainHand != outfit.Items.end())
                    if (sObjectMgr->GetItemTemplate(mainHand->second)->InventoryType == INVTYPE_2HWEAPON)
                    {
                        if (outfit.Items.count(EQUIPMENT_SLOT_OFFHAND))
                            valid = false;
                        else
                            outfit.Items[EQUIPMENT_SLOT_OFFHAND] = 0;
                    }
                if (valid && !outfit.Items.empty())
                {
                    if (useTemplate)
                        AddTemplateOutfits(entry.Class, profile);
                    else
                        _legendaryOutfits[entry.Class].push_back(std::move(outfit));
                }
                else
                    LOG_ERROR("module", "[CityLife.Custom] Ignored invalid {} outfit.", entry.Name);
            }
        }

        _fishingOutfit = {};
        uint32 poleId = sConfigMgr->GetOption<uint32>("FishingLife.PoleItem", 6256);
        ItemTemplate const* pole = sObjectMgr->GetItemTemplate(poleId);
        if (pole && pole->Class == ITEM_CLASS_WEAPON && pole->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE &&
            !pole->Duration)
        {
            _fishingOutfit.Items[EQUIPMENT_SLOT_MAINHAND] = poleId;
            _fishingOutfit.Items[EQUIPMENT_SLOT_OFFHAND] = 0;
        }
        else
            LOG_ERROR("module", "[CityLife.Custom] Invalid fishing pole item {}; fishing cannot reserve bots.", poleId);

        _databaseReady = static_cast<bool>(
            CharacterDatabase.Query("SHOW TABLES LIKE 'city_life_custom_equipment'"));
        if (!_databaseReady)
        {
            LOG_ERROR("module", "[CityLife.Custom] Apply data/sql/manual/characters_city_life_custom.sql "
                "to CHARACTERS before enabling legendary, beggar or fishing equipment.");
            return;
        }

        QueryResult count = CharacterDatabase.Query("SELECT COUNT(*) FROM city_life_custom_equipment");
        if (!count)
        {
            _databaseReady = false;
            return;
        }
        std::unordered_map<uint32, std::vector<EquipmentRecord>> pending;
        QueryResult result = CharacterDatabase.Query(
            "SELECT bot_guid,slot,original_guid,temporary_guid,temporary_entry "
            "FROM city_life_custom_equipment ORDER BY bot_guid,slot");
        if (count->Fetch()[0].Get<uint64>() && !result)
        {
            _databaseReady = false;
            return;
        }
        if (result)
            do
            {
                Field* fields = result->Fetch();
                uint32 guidLow = fields[0].Get<uint32>();
                EquipmentRecord record;
                record.Slot = fields[1].Get<uint8>();
                record.OriginalGuid = fields[2].Get<uint32>();
                record.TemporaryGuid = fields[3].Get<uint32>();
                record.TemporaryEntry = fields[4].Get<uint32>();
                pending[guidLow].push_back(record);
            } while (result->NextRow());
        _pending.swap(pending);

        std::vector<uint32> recover;
        for (auto const& entry : _pending)
            if (!_active.count(entry.first))
                recover.push_back(entry.first);
        for (uint32 guidLow : recover)
            if (Player* player = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(guidLow)))
                RecoverOnLogin(player);
    }

    bool CustomAppearance::AddTemplateOutfits(uint8 classId, std::string const& profile)
    {
        std::map<uint32, Outfit> variants;
        for (LegendaryGear::Row const& row : LegendaryGear::Rows)
        {
            if (row.Class != classId || profile != row.Profile)
                continue;
            Outfit& outfit = variants[row.RaceMask];
            outfit.RaceMask = row.RaceMask;
            outfit.Items[row.Slot] = row.Item;
            outfit.Bonuses[row.Slot] = {row.Enchant, row.Gems, row.Bonus, row.Prismatic};
        }
        bool loaded = false;
        for (auto& variant : variants)
        {
            Outfit& outfit = variant.second;
            // Actual weapons, enchants and gems always come from the same complete level-80 build.
            // Iconic weapons are cosmetic skins; provider availability must never alter these stats.
            auto mainHand = outfit.Items.find(EQUIPMENT_SLOT_MAINHAND);
            if (mainHand != outfit.Items.end() && mainHand->second)
                if (ItemTemplate const* item = sObjectMgr->GetItemTemplate(mainHand->second))
                    if (item->InventoryType == INVTYPE_2HWEAPON && !outfit.Items.count(EQUIPMENT_SLOT_OFFHAND))
                        outfit.Items[EQUIPMENT_SLOT_OFFHAND] = 0;
            bool valid = std::all_of(outfit.Items.begin(), outfit.Items.end(), [classId](auto const& entry)
            {
                if (!entry.second)
                    return true;
                ItemTemplate const* item = sObjectMgr->GetItemTemplate(entry.second);
                return item && !item->Duration && item->Class != ITEM_CLASS_CONTAINER &&
                    (item->AllowableClass & (1 << (classId - 1))) != 0;
            });
            if (valid && !outfit.Items.empty())
            {
                auto addSkin = [&](LegendarySkins::Row const& row)
                {
                    auto actual = outfit.Items.find(row.Slot);
                    if (row.Class != classId || profile != row.Profile || actual == outfit.Items.end() ||
                        !actual->second || row.Slot >= EQUIPMENT_SLOT_END)
                        return;
                    if (row.Item == LegendarySkins::HiddenItem)
                    {
                        if (_hideSkinOffhand && row.Slot == EQUIPMENT_SLOT_OFFHAND)
                            outfit.Skins[row.Slot] = row.Item;
                        return;
                    }
                    ItemTemplate const* skin = sObjectMgr->GetItemTemplate(row.Item);
                    if (!skin || skin->Duration ||
                        (skin->Class != ITEM_CLASS_ARMOR && skin->Class != ITEM_CLASS_WEAPON) ||
                        (skin->AllowableClass & (1 << (classId - 1))) == 0)
                    {
                        LOG_WARN("module", "[CityLife.Custom] Invalid cosmetic item {} for profile {}; skipped.",
                            row.Item, profile);
                        return;
                    }
                    outfit.Skins[row.Slot] = row.Item;
                };
                if (_skinsEnabled && _armorSkinsEnabled)
                    for (LegendarySkins::Row const& row : LegendarySkins::ArmorRows)
                        addSkin(row);
                if (_skinsEnabled && _weaponSkinsEnabled)
                    for (LegendarySkins::Row const& row : LegendarySkins::WeaponRows)
                        addSkin(row);
                _legendaryOutfits[classId].push_back(std::move(outfit));
                loaded = true;
            }
        }
        if (!loaded)
            LOG_WARN("module", "[CityLife.Custom] Phase 4 profile {} unavailable; profile skipped.", profile);
        return loaded;
    }

    void CustomAppearance::ApplyBonuses(Player* player, Item* item, ItemBonuses const& bonuses) const
    {
        // These items are not equipped yet. EquipItem applies their effects after the journal is durable.
        auto setEnchant = [player, item](EnchantmentSlot slot, uint32 id)
        {
            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(id);
            if (!enchant || enchant->requiredLevel > player->GetLevel() ||
                (enchant->requiredSkill && player->GetSkillValue(enchant->requiredSkill) < enchant->requiredSkillValue))
                return;
            item->SetEnchantment(slot, id, 0, 0, player->GetGUID());
        };
        if (_enchantsEnabled)
        {
            uint32 id = bonuses.Enchant;
            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(id);
            if (enchant && enchant->requiredSkill &&
                player->GetSkillValue(enchant->requiredSkill) < enchant->requiredSkillValue)
            {
                bool caster = std::any_of(std::begin(item->GetTemplate()->ItemStat),
                    std::end(item->GetTemplate()->ItemStat), [](auto const& stat)
                    {
                        return stat.ItemStatType == ITEM_MOD_SPELL_POWER && stat.ItemStatValue > 0;
                    });
                // Use ordinary enchants when the source build assumes a profession this bot does not have.
                // IDs and restrictions are verified against the local SpellItemEnchantment.dbc.
                switch (item->GetTemplate()->InventoryType)
                {
                    case INVTYPE_FEET: id = 3232; break;
                    case INVTYPE_CLOAK: id = 3831; break;
                    case INVTYPE_WRISTS:
                        id = enchant->spellid[0] == ITEM_MOD_STAMINA ? 3850 : (caster ? 2332 : 3845);
                        break;
                    case INVTYPE_HANDS:
                        id = caster ? 3246 : 1603;
                        break;
                    default: id = 0; break;
                }
            }
            setEnchant(PERM_ENCHANTMENT_SLOT, id);
        }
        if (!_gemsEnabled)
            return;

        ItemTemplate const* itemTemplate = item->GetTemplate();
        uint8 firstExtraSocket = 0;
        while (firstExtraSocket < MAX_GEM_SOCKETS && itemTemplate->Socket[firstExtraSocket].Color)
            ++firstExtraSocket;
        uint32 extraGem = 0;
        if (SpellItemEnchantmentEntry const* prismatic = sSpellItemEnchantmentStore.LookupEntry(bonuses.Prismatic))
        {
            bool addsSocket = std::any_of(std::begin(prismatic->type), std::end(prismatic->type),
                [](uint32 type) { return type == ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET; });
            if (addsSocket && firstExtraSocket < MAX_GEM_SOCKETS)
                setEnchant(PRISMATIC_ENCHANTMENT_SLOT, bonuses.Prismatic);
            else if (prismatic->GemID && itemTemplate->InventoryType == INVTYPE_WAIST &&
                firstExtraSocket < MAX_GEM_SOCKETS)
            {
                // The source templates store the buckle's gem in prismaticEnchant, not the socket enchant.
                // 3729 is the Eternal Belt Buckle socket, verified against SpellItemEnchantment.dbc.
                setEnchant(PRISMATIC_ENCHANTMENT_SLOT, 3729);
                extraGem = bonuses.Prismatic;
            }
        }
        for (uint8 index = 0; index < MAX_GEM_SOCKETS; ++index)
        {
            uint32 gem = bonuses.Gems[index];
            uint8 socketColor = itemTemplate->Socket[index].Color;
            if (!socketColor)
            {
                if (index != firstExtraSocket || !item->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT))
                    continue;
                if (!gem)
                    gem = extraGem;
            }
            if (SpellItemEnchantmentEntry const* requested = sSpellItemEnchantmentStore.LookupEntry(gem))
                if (requested->requiredSkill &&
                    player->GetSkillValue(requested->requiredSkill) < requested->requiredSkillValue)
                {
                    // Dragon's Eyes need Jewelcrafting. Ordinary epic gems keep the same primary stat.
                    // This also covers a Dragon's Eye selected for the buckle's extra socket.
                    switch (gem)
                    {
                        case 3732: gem = 3518; break; // Strength.
                        case 3733: gem = 3519; break; // Agility.
                        case 3737: gem = 3526; break; // Intellect.
                        case 3745: gem = 3525; break; // Armor penetration.
                        case 3746: gem = 3524; break; // Expertise.
                        default: break;
                    }
                }
            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(gem);
            ItemTemplate const* gemItem = enchant ? sObjectMgr->GetItemTemplate(enchant->GemID) : nullptr;
            GemPropertiesEntry const* properties = gemItem ?
                sGemPropertiesStore.LookupEntry(gemItem->GemProperties) : nullptr;
            if (!properties || (socketColor == SOCKET_COLOR_META) != (properties->color == SOCKET_COLOR_META))
                continue;
            setEnchant(static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT + index), gem);
        }
        if (item->GemsFitSockets())
            setEnchant(BONUS_ENCHANTMENT_SLOT, itemTemplate->socketBonus);
    }

    bool CustomAppearance::KeepOriginal(Player const* player, Outfit const& outfit, uint8 slot, Item const* item) const
    {
        auto entry = outfit.Items.find(slot);
        if (!item || entry == outfit.Items.end() || item->GetEntry() != entry->second)
            return false;
        // A max-owned unique item cannot have a second instance; keep it without touching its enchants.
        return !outfit.Bonuses.count(slot) ||
            player->CanTakeMoreSimilarItems(entry->second, 1) != EQUIP_ERR_OK;
    }

    CustomAppearance::Outfit const* CustomAppearance::FishingOutfit() const
    {
        return _fishingOutfit.Items.empty() ? nullptr : &_fishingOutfit;
    }

    bool CustomAppearance::IsOutfitUsable(Player* player, Outfit const& outfit) const
    {
        if (outfit.RaceMask && (outfit.RaceMask & player->getRaceMask()) == 0)
            return false;
        auto offHand = outfit.Items.find(EQUIPMENT_SLOT_OFFHAND);
        if (offHand != outfit.Items.end() && offHand->second)
        {
            ItemTemplate const* offItem = sObjectMgr->GetItemTemplate(offHand->second);
            if (offItem && offItem->Class == ITEM_CLASS_WEAPON && !player->CanDualWield())
                return false;
            auto mainHand = outfit.Items.find(EQUIPMENT_SLOT_MAINHAND);
            ItemTemplate const* mainItem = mainHand != outfit.Items.end() ?
                sObjectMgr->GetItemTemplate(mainHand->second) : nullptr;
            if ((offItem && offItem->InventoryType == INVTYPE_2HWEAPON) ||
                (mainItem && mainItem->InventoryType == INVTYPE_2HWEAPON))
                if (!player->CanTitanGrip() || !player->CanDualWield())
                    return false;
        }
        std::vector<uint16> freeSlots = FreeGeneralSlots(player);
        std::size_t nextFree = 0;
        for (auto const& entry : outfit.Items)
        {
            uint8 slot = entry.first;
            Item* current = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (KeepOriginal(player, outfit, slot, current))
                continue;
            if (current)
            {
                if (current->IsInTrade() || player->CanUnequipItem(EquipmentPosition(slot), false) != EQUIP_ERR_OK ||
                    nextFree >= freeSlots.size())
                    return false;
                uint16 position = freeSlots[nextFree++];
                ItemPosCountVec destination;
                if (player->CanStoreItem(position >> 8, position & 255, destination, current) != EQUIP_ERR_OK)
                    return false;
            }
            if (entry.second)
            {
                ItemTemplate const* item = sObjectMgr->GetItemTemplate(entry.second);
                if (!item || player->CanUseItem(item) != EQUIP_ERR_OK ||
                    (item->GetSkill() && !player->GetSkillValue(item->GetSkill())) ||
                    player->CanTakeMoreSimilarItems(entry.second, 1) != EQUIP_ERR_OK)
                    return false;
            }
        }
        return true;
    }

    std::vector<CustomAppearance::Outfit const*> CustomAppearance::UsableOutfits(Player* player, CustomRole role) const
    {
        std::vector<Outfit const*> outfits;
        if (role == CustomRole::Fishing)
        {
            if (Outfit const* outfit = FishingOutfit())
                if (IsOutfitUsable(player, *outfit))
                    outfits.push_back(outfit);
        }
        else if (role == CustomRole::Legendary)
        {
            auto found = _legendaryOutfits.find(player->getClass());
            if (found != _legendaryOutfits.end())
                for (Outfit const& outfit : found->second)
                    if (IsOutfitUsable(player, outfit))
                        outfits.push_back(&outfit);
        }
        return outfits;
    }

    bool CustomAppearance::CanUse(Player* player, CustomRole role) const
    {
        if (!player || HasPendingRecovery(player) || _active.count(player->GetGUID().GetCounter()))
            return false;
        if (!NeedsEquipment(role))
            return true;
        if (!_databaseReady || player->IsMounted() || player->GetShapeshiftForm() != FORM_NONE ||
            player->IsBeingTeleported() || !player->IsAlive() || player->IsInCombat() ||
            player->IsNonMeleeSpellCast(false))
            return false;
        if (role == CustomRole::Beggar)
        {
            Outfit naked;
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                naked.Items[slot] = 0;
            return IsOutfitUsable(player, naked);
        }
        return !UsableOutfits(player, role).empty();
    }

    bool CustomAppearance::CanCreateLegendary(uint8 classId, uint8 level) const
    {
        if (!_databaseReady)
            return false;
        auto found = _legendaryOutfits.find(classId);
        if (found == _legendaryOutfits.end())
            return false;
        return std::any_of(found->second.begin(), found->second.end(), [level](Outfit const& outfit)
        {
            return std::all_of(outfit.Items.begin(), outfit.Items.end(), [level](auto const& entry)
            {
                if (!entry.second)
                    return true;
                ItemTemplate const* item = sObjectMgr->GetItemTemplate(entry.second);
                return item && item->RequiredLevel <= level;
            });
        });
    }

    bool CustomAppearance::Begin(Player* player, CustomRole role)
    {
        if (!CanUse(player, role))
            return false;
        if (!NeedsEquipment(role))
            return true;

        uint32 guidLow = player->GetGUID().GetCounter();
        Outfit outfit;
        if (role == CustomRole::Beggar)
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                outfit.Items[slot] = 0;
        else
        {
            std::vector<Outfit const*> outfits = UsableOutfits(player, role);
            // A pinned persona keeps the same recognizable outfit after a restart.
            outfit = *outfits[guidLow % outfits.size()];
        }

        std::vector<EquipmentRecord> records;
        std::map<uint8, std::unique_ptr<Item>> temporaryItems;
        std::map<uint8, uint16> originalDestinations;
        std::vector<uint16> freeSlots = FreeGeneralSlots(player);
        std::size_t nextFree = 0;
        for (auto const& entry : outfit.Items)
        {
            Item* original = player->GetItemByPos(INVENTORY_SLOT_BAG_0, entry.first);
            if (KeepOriginal(player, outfit, entry.first, original))
                continue;
            if (!original && !entry.second)
                continue;
            EquipmentRecord record;
            record.Slot = entry.first;
            if (original)
            {
                record.OriginalGuid = original->GetGUID().GetCounter();
                if (nextFree >= freeSlots.size())
                    return false;
                originalDestinations[entry.first] = freeSlots[nextFree++];
            }
            if (entry.second)
            {
                std::unique_ptr<Item> item(Item::CreateItem(entry.second, 1, player));
                if (!item)
                    return false;
                item->SetBinding(true);
                if (auto bonuses = outfit.Bonuses.find(entry.first); bonuses != outfit.Bonuses.end())
                    ApplyBonuses(player, item.get(), bonuses->second);
                // Enchanting queues even an unequipped item. Keep it out of the original inventory save;
                // its own journal save reserves its GUID, and EquipItem later queues its actual position.
                item->RemoveFromUpdateQueueOf(player);
                record.TemporaryGuid = item->GetGUID().GetCounter();
                record.TemporaryEntry = entry.second;
                temporaryItems.emplace(entry.first, std::move(item));
            }
            records.push_back(record);
        }

        // Durable write-ahead rows exist before the first inventory mutation. If the process stops before
        // the inventory save, login recovery simply finds the original items still equipped.
        if (!records.empty())
        {
            CharacterDatabaseTransaction journal = CharacterDatabase.BeginTransaction();
            std::vector<std::pair<ObjectGuid, ItemUpdateState>> queuedStates;
            for (Item* item : player->GetItemUpdateQueue())
                if (item && item->GetState() != ITEM_REMOVED)
                    queuedStates.emplace_back(item->GetGUID(), item->GetState());
            // Make the original inventory durable in the same transaction as its recovery rows,
            // including newly-created gear which has not reached a normal character autosave yet.
            player->SaveInventoryAndGoldToDB(journal);
            // Reserve every generated GUID in item_instance too. The generator is seeded from that table
            // after a restart, so an interrupted outfit must never let another item reuse its GUID.
            for (auto const& entry : temporaryItems)
                entry.second->SaveToDB(journal);
            for (EquipmentRecord const& record : records)
                journal->Append("INSERT INTO city_life_custom_equipment "
                    "(bot_guid,slot,original_guid,temporary_guid,temporary_entry) VALUES ({},{},{},{},{})",
                    guidLow, record.Slot, record.OriginalGuid, record.TemporaryGuid, record.TemporaryEntry);
            // Core inventory statements are async-only; await the worker before changing the outfit.
            bool const committed = CharacterDatabase.AsyncCommitTransaction(journal).m_future.get();
            QueryResult written = CharacterDatabase.Query(
                "SELECT slot,original_guid,temporary_guid,temporary_entry FROM city_life_custom_equipment "
                "WHERE bot_guid={} ORDER BY slot", guidLow);
            bool matches = committed && written && written->GetRowCount() == records.size();
            if (matches)
            {
                std::size_t index = 0;
                do
                {
                    Field* fields = written->Fetch();
                    EquipmentRecord const& record = records[index++];
                    matches = matches && fields[0].Get<uint8>() == record.Slot &&
                        fields[1].Get<uint32>() == record.OriginalGuid &&
                        fields[2].Get<uint32>() == record.TemporaryGuid &&
                        fields[3].Get<uint32>() == record.TemporaryEntry;
                } while (written->NextRow());
            }
            if (!matches)
            {
                LOG_ERROR("module", "[CityLife.Custom] Could not verify equipment journal for {}; outfit unchanged.",
                    player->GetGUID().ToString());
                // Inventory saving clears update flags before the SQL transaction completes. Retry
                // pre-existing new/changed items too when the durable write could not be confirmed.
                for (auto const& queued : queuedStates)
                    if (Item* item = player->GetItemByGuid(queued.first))
                    {
                        item->FSetState(queued.second);
                        item->AddToUpdateQueueOf(player);
                    }
                _pending[guidLow] = records;
                End(player);
                return false;
            }
            _pending[guidLow] = records;
        }

        AppearanceState state;
        state.Role = role;
        if (role == CustomRole::Legendary && _mountDisplayId)
            state.TargetMountDisplayId = _randomizeMount && !_mountDisplayIds.empty() ?
                _mountDisplayIds[MountChoice(guidLow) % _mountDisplayIds.size()] : _mountDisplayId;
        state.OriginalSheath = static_cast<uint8>(player->GetSheath());
        _active[guidLow] = state;

        bool const useTransmog = role == CustomRole::Legendary && LegendaryTransmog::Available();
        if (useTransmog)
            for (auto const& entry : temporaryItems)
                if (!LegendaryTransmog::RegisterTemporary(player, entry.second.get()))
                {
                    LOG_ERROR("module", "[CityLife.Custom] Could not register temporary cosmetics for {}; "
                        "restoring original equipment.", player->GetGUID().ToString());
                    End(player);
                    return false;
                }

        // Move every original first, including offhand items before equipping a two-handed weapon.
        for (EquipmentRecord const& record : records)
            if (record.OriginalGuid)
            {
                Item* original = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(record.OriginalGuid));
                uint16 position = originalDestinations[record.Slot];
                ItemPosCountVec destination;
                if (!original ||
                    player->CanStoreItem(position >> 8, position & 255, destination, original) != EQUIP_ERR_OK)
                {
                    End(player);
                    return false;
                }
                player->RemoveItem(INVENTORY_SLOT_BAG_0, record.Slot, true);
                player->StoreItem(destination, original, true);
            }

        for (auto& entry : temporaryItems)
        {
            uint16 destination = 0;
            if (player->CanEquipItem(entry.first, destination, entry.second.get(), false) != EQUIP_ERR_OK ||
                destination != EquipmentPosition(entry.first))
            {
                End(player);
                return false;
            }
            player->EquipItem(destination, entry.second.release(), true);
        }
        player->UpdateTitansGrip();

        if (!records.empty())
        {
            CharacterDatabaseTransaction save = CharacterDatabase.BeginTransaction();
            player->SaveInventoryAndGoldToDB(save);
            if (!CharacterDatabase.AsyncCommitTransaction(save).m_future.get())
            {
                LOG_ERROR("module", "[CityLife.Custom] Could not save the outfit for {}; restoring original equipment.",
                    player->GetGUID().ToString());
                End(player);
                return false;
            }
        }
        if (useTransmog)
            for (auto const& [slot, skin] : outfit.Skins)
            {
                Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                // KeepOriginal items are not in the journal and never receive a temporary cosmetic.
                if (!IsTemporaryItem(player, item))
                    continue;
                if (!LegendaryTransmog::Apply(player, item, slot, skin))
                {
                    LOG_ERROR("module", "[CityLife.Custom] Could not apply temporary cosmetics for {}; "
                        "restoring original equipment.", player->GetGUID().ToString());
                    End(player);
                    return false;
                }
            }
        _active[guidLow].Applied = true;
        Update(player, role);
        return true;
    }

    void CustomAppearance::Update(Player* player, CustomRole role)
    {
        if (!player || player->IsBeingTeleported() || player->IsInCombat() || !player->IsAlive())
            return;
        auto found = _active.find(player->GetGUID().GetCounter());
        if (found == _active.end() || !found->second.Applied || found->second.Role != role)
            return;
        if (role == CustomRole::Legendary && found->second.TargetMountDisplayId && !player->IsMounted())
        {
            player->Mount(found->second.TargetMountDisplayId);
            found->second.MountDisplayId = found->second.TargetMountDisplayId;
        }
    }

    bool CustomAppearance::RestoreEquipment(Player* player, std::vector<EquipmentRecord> const& records)
    {
        bool complete = true;
        std::set<uint32> originals;
        for (EquipmentRecord const& record : records)
            if (record.OriginalGuid)
                originals.insert(record.OriginalGuid);
        // Destruction and orphan SQL cleanup both bypass the normal transmog delete hook in this core.
        // Clear only exact registered temporary GUIDs, including items absent from the loaded inventory.
        for (EquipmentRecord const& record : records)
            if (record.TemporaryGuid && !originals.count(record.TemporaryGuid))
                LegendaryTransmog::ForgetTemporary(player, record.TemporaryGuid);
        for (EquipmentRecord const& record : records)
        {
            if (record.Slot >= EQUIPMENT_SLOT_END)
            {
                complete = false;
                continue;
            }
            if (record.TemporaryGuid && originals.count(record.TemporaryGuid))
            {
                complete = false;
                continue;
            }
            if (record.TemporaryGuid)
                if (Item* temporary = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(record.TemporaryGuid)))
                {
                    if (temporary->GetEntry() == record.TemporaryEntry)
                        player->DestroyItem(temporary->GetBagSlot(), temporary->GetSlot(), true);
                    else
                        complete = false;
                }
        }
        for (EquipmentRecord const& record : records)
        {
            if (record.Slot >= EQUIPMENT_SLOT_END || !record.OriginalGuid)
                continue;
            Item* original = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(record.OriginalGuid));
            if (!original)
            {
                LOG_ERROR("module", "[CityLife.Custom] Original item {} missing for {}; recovery journal retained.",
                    record.OriginalGuid, player->GetGUID().ToString());
                complete = false;
                continue;
            }
            if (original->GetPos() == EquipmentPosition(record.Slot))
            {
                // A previous restoration transaction may have failed after clearing item update flags.
                // Always save this position again before deleting the recovery journal.
                original->SetState(ITEM_CHANGED, player);
                continue;
            }
            uint8 sourceBag = original->GetBagSlot();
            uint8 sourceSlot = original->GetSlot();
            Item* occupant = player->GetItemByPos(INVENTORY_SLOT_BAG_0, record.Slot);
            ItemPosCountVec occupantDestination;
            if (occupant && (!Player::IsInventoryPos(sourceBag, sourceSlot) ||
                player->CanStoreItem(sourceBag, sourceSlot, occupantDestination, occupant, true) != EQUIP_ERR_OK))
            {
                LOG_ERROR("module", "[CityLife.Custom] Cannot safely restore slot {} for {}; journal retained.",
                    record.Slot, player->GetGUID().ToString());
                complete = false;
                continue;
            }
            player->RemoveItem(sourceBag, sourceSlot, true);
            if (occupant)
                player->RemoveItem(INVENTORY_SLOT_BAG_0, record.Slot, true);
            player->EquipItem(EquipmentPosition(record.Slot), original, true);
            if (occupant)
                player->StoreItem(occupantDestination, occupant, true);
        }
        player->UpdateTitansGrip();
        return complete;
    }

    bool CustomAppearance::JournalExists(uint32 guidLow) const
    {
        QueryResult result = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM city_life_custom_equipment WHERE bot_guid={}", guidLow);
        return !result || result->Fetch()[0].Get<uint64>() != 0;
    }

    bool CustomAppearance::End(Player* player)
    {
        if (!player)
            return false;
        uint32 guidLow = player->GetGUID().GetCounter();
        auto state = _active.find(guidLow);
        if (state != _active.end())
        {
            state->second.Applied = false;
            if (state->second.MountDisplayId && player->IsMounted() &&
                player->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) == state->second.MountDisplayId)
                player->Dismount();
            player->SetSheath(static_cast<SheathState>(state->second.OriginalSheath));
        }
        auto pending = _pending.find(guidLow);
        if (pending == _pending.end())
        {
            _active.erase(guidLow);
            return true;
        }
        bool restored = RestoreEquipment(player, pending->second);
        CharacterDatabaseTransaction save = CharacterDatabase.BeginTransaction();
        player->SaveInventoryAndGoldToDB(save);
        // Unlinked item_instance rows are possible if the server stopped after writing the journal but
        // before the inventory save. Delete only journal-generated entries belonging to this character.
        std::set<uint32> originals;
        for (EquipmentRecord const& record : pending->second)
            if (record.OriginalGuid)
                originals.insert(record.OriginalGuid);
        for (EquipmentRecord const& record : pending->second)
            if (record.TemporaryGuid && !originals.count(record.TemporaryGuid) &&
                !player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(record.TemporaryGuid)))
            {
                save->Append("DELETE FROM character_inventory WHERE guid={} AND item={}",
                    player->GetGUID().GetRawValue(), record.TemporaryGuid);
                save->Append("DELETE FROM item_instance WHERE guid={} AND owner_guid={} AND itemEntry={}",
                    record.TemporaryGuid, player->GetGUID().GetRawValue(), record.TemporaryEntry);
            }
        if (restored)
            for (EquipmentRecord const& record : pending->second)
                save->Append("DELETE FROM city_life_custom_equipment WHERE bot_guid={} AND slot={} "
                    "AND original_guid={} AND temporary_guid={} AND temporary_entry={}",
                    guidLow, record.Slot, record.OriginalGuid, record.TemporaryGuid, record.TemporaryEntry);
        bool const committed = CharacterDatabase.AsyncCommitTransaction(save).m_future.get();
        if (!committed || !restored || JournalExists(guidLow))
        {
            LOG_ERROR("module", "[CityLife.Custom] Equipment recovery for {} remains pending; "
                "do not remove the journal table.", player->GetGUID().ToString());
            return false;
        }
        _pending.erase(guidLow);
        _active.erase(guidLow);
        return true;
    }

    bool CustomAppearance::RecoverOnLogin(Player* player)
    {
        if (!player)
            return false;
        uint32 guidLow = player->GetGUID().GetCounter();
        if (!_pending.count(guidLow) && !_active.count(guidLow))
            return true;
        if (!_databaseReady)
            return false;
        return End(player);
    }

    bool CustomAppearance::HasPendingRecovery(Player const* player) const
    {
        return player && _pending.count(player->GetGUID().GetCounter()) != 0;
    }

    bool CustomAppearance::IsApplied(Player const* player) const
    {
        if (!player)
            return false;
        auto found = _active.find(player->GetGUID().GetCounter());
        return found != _active.end() && found->second.Applied;
    }

    std::vector<uint32> CustomAppearance::PendingGuids() const
    {
        std::vector<uint32> guids;
        guids.reserve(_pending.size());
        for (auto const& pending : _pending)
            guids.push_back(pending.first);
        return guids;
    }

    bool CustomAppearance::IsTemporaryItem(Player const* player, Item const* item) const
    {
        if (!player || !item || item->GetOwnerGUID() != player->GetGUID())
            return false;
        auto found = _pending.find(player->GetGUID().GetCounter());
        if (found == _pending.end())
            return false;
        uint32 const itemGuid = item->GetGUID().GetCounter();
        if (std::any_of(found->second.begin(), found->second.end(), [itemGuid](EquipmentRecord const& record)
            { return record.OriginalGuid == itemGuid; }))
            return false;
        return std::any_of(found->second.begin(), found->second.end(),
            [&](EquipmentRecord const& record)
            {
                return record.Slot < EQUIPMENT_SLOT_END && record.TemporaryGuid && record.TemporaryGuid == itemGuid &&
                    record.TemporaryEntry == item->GetEntry();
            });
    }
}
