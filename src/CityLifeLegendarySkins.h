/*
 * City Life - cosmetic-only Legendary appearances.
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 * T6 sets and item entries verified against local 3.3.5 ItemSet.dbc and WORLD item_template.
 * These entries change visible equipment only; never equip them or replace the Phase 4 BiS items.
 */

#ifndef CITY_LIFE_LEGENDARY_SKINS_H
#define CITY_LIFE_LEGENDARY_SKINS_H

#include "Define.h"

namespace CityLife::LegendarySkins
{
    // The consumer translates this sentinel to the appearance provider's hidden-item marker.
    // It is not an item entry and must never be created, equipped, or checked as an item template.
    inline constexpr uint32 HiddenItem = 0xFFFFFFFFu;

    struct Profile
    {
        uint8 Class;
        char const* Name;
        uint32 ArmorSet;
    };

    struct Row
    {
        uint8 Class;
        char const* Profile;
        uint8 Slot;
        uint32 Item;
    };

    // Profile names match the bundled LegendaryGear template keys exactly.
    // Death Knights did not have Tier 6: ArmorSet 0 preserves their actual BiS armor appearance.
    inline constexpr Profile Profiles[] =
    {
        {1, "Fury80PvEP4BiS", 672},
        {1, "Arms80PvEP4BiS", 672},
        {2, "Retribution80PvEP4BiS", 680},
        {2, "Holy80PvEP4BiSHeal", 681},
        {3, "Marksmanship80PvEP4BiS", 669},
        {3, "Survival80PvEP4BiS", 669},
        {4, "Combat80PvEP4BiS", 668},
        {4, "Assassination80PvEP4BiS", 668},
        {5, "Discipline80PvEP4BiSHeal", 675},
        {5, "Shadow80PvEP4BiS", 674},
        {6, "Unholy80PvEP4BiS", 0},
        {6, "Frost80PvEP4BiS", 0},
        {7, "Restoration80PvEP4BiSHeal", 683},
        {7, "Elemental80PvEP4BiS", 684},
        {8, "Arcane80PvEP4BiS", 671},
        {8, "Fire80PvEP4BiS", 671},
        {9, "Affliction80PvEP4BiS", 670},
        {9, "Destruction80PvEP4BiS", 670},
        {11, "Restoration80PvEP4BiSHeal", 678},
        {11, "Balance80PvEP4BiS", 677},
    };

    // Equipment slots: head 0, shoulders 2, chest 4, waist 5, legs 6, feet 7, wrists 8, hands 9.
    // The three Sunwell Tier 6 pieces supply waist/feet/wrists; their actual BiS stats stay intact.
    // T6 has no matching neck, rings, trinkets, cloak, shirt, or tabard in these class sets.
    inline constexpr Row ArmorRows[] =
    {
        // Fury80PvEP4BiS
        {1, "Fury80PvEP4BiS", 0, 30972},
        {1, "Fury80PvEP4BiS", 2, 30979},
        {1, "Fury80PvEP4BiS", 4, 30975},
        {1, "Fury80PvEP4BiS", 5, 34546},
        {1, "Fury80PvEP4BiS", 6, 30977},
        {1, "Fury80PvEP4BiS", 7, 34569},
        {1, "Fury80PvEP4BiS", 8, 34441},
        {1, "Fury80PvEP4BiS", 9, 30969},

        // Arms80PvEP4BiS
        {1, "Arms80PvEP4BiS", 0, 30972},
        {1, "Arms80PvEP4BiS", 2, 30979},
        {1, "Arms80PvEP4BiS", 4, 30975},
        {1, "Arms80PvEP4BiS", 5, 34546},
        {1, "Arms80PvEP4BiS", 6, 30977},
        {1, "Arms80PvEP4BiS", 7, 34569},
        {1, "Arms80PvEP4BiS", 8, 34441},
        {1, "Arms80PvEP4BiS", 9, 30969},

        // Retribution80PvEP4BiS
        {2, "Retribution80PvEP4BiS", 0, 30989},
        {2, "Retribution80PvEP4BiS", 2, 30997},
        {2, "Retribution80PvEP4BiS", 4, 30990},
        {2, "Retribution80PvEP4BiS", 5, 34485},
        {2, "Retribution80PvEP4BiS", 6, 30993},
        {2, "Retribution80PvEP4BiS", 7, 34561},
        {2, "Retribution80PvEP4BiS", 8, 34431},
        {2, "Retribution80PvEP4BiS", 9, 30982},

        // Holy80PvEP4BiSHeal
        {2, "Holy80PvEP4BiSHeal", 0, 30988},
        {2, "Holy80PvEP4BiSHeal", 2, 30996},
        {2, "Holy80PvEP4BiSHeal", 4, 30992},
        {2, "Holy80PvEP4BiSHeal", 5, 34487},
        {2, "Holy80PvEP4BiSHeal", 6, 30994},
        {2, "Holy80PvEP4BiSHeal", 7, 34559},
        {2, "Holy80PvEP4BiSHeal", 8, 34432},
        {2, "Holy80PvEP4BiSHeal", 9, 30983},

        // Marksmanship80PvEP4BiS
        {3, "Marksmanship80PvEP4BiS", 0, 31003},
        {3, "Marksmanship80PvEP4BiS", 2, 31006},
        {3, "Marksmanship80PvEP4BiS", 4, 31004},
        {3, "Marksmanship80PvEP4BiS", 5, 34549},
        {3, "Marksmanship80PvEP4BiS", 6, 31005},
        {3, "Marksmanship80PvEP4BiS", 7, 34570},
        {3, "Marksmanship80PvEP4BiS", 8, 34443},
        {3, "Marksmanship80PvEP4BiS", 9, 31001},

        // Survival80PvEP4BiS
        {3, "Survival80PvEP4BiS", 0, 31003},
        {3, "Survival80PvEP4BiS", 2, 31006},
        {3, "Survival80PvEP4BiS", 4, 31004},
        {3, "Survival80PvEP4BiS", 5, 34549},
        {3, "Survival80PvEP4BiS", 6, 31005},
        {3, "Survival80PvEP4BiS", 7, 34570},
        {3, "Survival80PvEP4BiS", 8, 34443},
        {3, "Survival80PvEP4BiS", 9, 31001},

        // Combat80PvEP4BiS
        {4, "Combat80PvEP4BiS", 0, 31027},
        {4, "Combat80PvEP4BiS", 2, 31030},
        {4, "Combat80PvEP4BiS", 4, 31028},
        {4, "Combat80PvEP4BiS", 5, 34558},
        {4, "Combat80PvEP4BiS", 6, 31029},
        {4, "Combat80PvEP4BiS", 7, 34575},
        {4, "Combat80PvEP4BiS", 8, 34448},
        {4, "Combat80PvEP4BiS", 9, 31026},

        // Assassination80PvEP4BiS
        {4, "Assassination80PvEP4BiS", 0, 31027},
        {4, "Assassination80PvEP4BiS", 2, 31030},
        {4, "Assassination80PvEP4BiS", 4, 31028},
        {4, "Assassination80PvEP4BiS", 5, 34558},
        {4, "Assassination80PvEP4BiS", 6, 31029},
        {4, "Assassination80PvEP4BiS", 7, 34575},
        {4, "Assassination80PvEP4BiS", 8, 34448},
        {4, "Assassination80PvEP4BiS", 9, 31026},

        // Discipline80PvEP4BiSHeal
        {5, "Discipline80PvEP4BiSHeal", 0, 31063},
        {5, "Discipline80PvEP4BiSHeal", 2, 31069},
        {5, "Discipline80PvEP4BiSHeal", 4, 31066},
        {5, "Discipline80PvEP4BiSHeal", 5, 34527},
        {5, "Discipline80PvEP4BiSHeal", 6, 31068},
        {5, "Discipline80PvEP4BiSHeal", 7, 34562},
        {5, "Discipline80PvEP4BiSHeal", 8, 34435},
        {5, "Discipline80PvEP4BiSHeal", 9, 31060},

        // Shadow80PvEP4BiS
        {5, "Shadow80PvEP4BiS", 0, 31064},
        {5, "Shadow80PvEP4BiS", 2, 31070},
        {5, "Shadow80PvEP4BiS", 4, 31065},
        {5, "Shadow80PvEP4BiS", 5, 34528},
        {5, "Shadow80PvEP4BiS", 6, 31067},
        {5, "Shadow80PvEP4BiS", 7, 34563},
        {5, "Shadow80PvEP4BiS", 8, 34434},
        {5, "Shadow80PvEP4BiS", 9, 31061},

        // Restoration80PvEP4BiSHeal
        {7, "Restoration80PvEP4BiSHeal", 0, 31012},
        {7, "Restoration80PvEP4BiSHeal", 2, 31022},
        {7, "Restoration80PvEP4BiSHeal", 4, 31016},
        {7, "Restoration80PvEP4BiSHeal", 5, 34543},
        {7, "Restoration80PvEP4BiSHeal", 6, 31019},
        {7, "Restoration80PvEP4BiSHeal", 7, 34565},
        {7, "Restoration80PvEP4BiSHeal", 8, 34438},
        {7, "Restoration80PvEP4BiSHeal", 9, 31007},

        // Elemental80PvEP4BiS
        {7, "Elemental80PvEP4BiS", 0, 31014},
        {7, "Elemental80PvEP4BiS", 2, 31023},
        {7, "Elemental80PvEP4BiS", 4, 31017},
        {7, "Elemental80PvEP4BiS", 5, 34542},
        {7, "Elemental80PvEP4BiS", 6, 31020},
        {7, "Elemental80PvEP4BiS", 7, 34566},
        {7, "Elemental80PvEP4BiS", 8, 34437},
        {7, "Elemental80PvEP4BiS", 9, 31008},

        // Arcane80PvEP4BiS
        {8, "Arcane80PvEP4BiS", 0, 31056},
        {8, "Arcane80PvEP4BiS", 2, 31059},
        {8, "Arcane80PvEP4BiS", 4, 31057},
        {8, "Arcane80PvEP4BiS", 5, 34557},
        {8, "Arcane80PvEP4BiS", 6, 31058},
        {8, "Arcane80PvEP4BiS", 7, 34574},
        {8, "Arcane80PvEP4BiS", 8, 34447},
        {8, "Arcane80PvEP4BiS", 9, 31055},

        // Fire80PvEP4BiS
        {8, "Fire80PvEP4BiS", 0, 31056},
        {8, "Fire80PvEP4BiS", 2, 31059},
        {8, "Fire80PvEP4BiS", 4, 31057},
        {8, "Fire80PvEP4BiS", 5, 34557},
        {8, "Fire80PvEP4BiS", 6, 31058},
        {8, "Fire80PvEP4BiS", 7, 34574},
        {8, "Fire80PvEP4BiS", 8, 34447},
        {8, "Fire80PvEP4BiS", 9, 31055},

        // Affliction80PvEP4BiS
        {9, "Affliction80PvEP4BiS", 0, 31051},
        {9, "Affliction80PvEP4BiS", 2, 31054},
        {9, "Affliction80PvEP4BiS", 4, 31052},
        {9, "Affliction80PvEP4BiS", 5, 34541},
        {9, "Affliction80PvEP4BiS", 6, 31053},
        {9, "Affliction80PvEP4BiS", 7, 34564},
        {9, "Affliction80PvEP4BiS", 8, 34436},
        {9, "Affliction80PvEP4BiS", 9, 31050},

        // Destruction80PvEP4BiS
        {9, "Destruction80PvEP4BiS", 0, 31051},
        {9, "Destruction80PvEP4BiS", 2, 31054},
        {9, "Destruction80PvEP4BiS", 4, 31052},
        {9, "Destruction80PvEP4BiS", 5, 34541},
        {9, "Destruction80PvEP4BiS", 6, 31053},
        {9, "Destruction80PvEP4BiS", 7, 34564},
        {9, "Destruction80PvEP4BiS", 8, 34436},
        {9, "Destruction80PvEP4BiS", 9, 31050},

        // Restoration80PvEP4BiSHeal
        {11, "Restoration80PvEP4BiSHeal", 0, 31037},
        {11, "Restoration80PvEP4BiSHeal", 2, 31047},
        {11, "Restoration80PvEP4BiSHeal", 4, 31041},
        {11, "Restoration80PvEP4BiSHeal", 5, 34554},
        {11, "Restoration80PvEP4BiSHeal", 6, 31045},
        {11, "Restoration80PvEP4BiSHeal", 7, 34571},
        {11, "Restoration80PvEP4BiSHeal", 8, 34445},
        {11, "Restoration80PvEP4BiSHeal", 9, 31032},

        // Balance80PvEP4BiS
        {11, "Balance80PvEP4BiS", 0, 31040},
        {11, "Balance80PvEP4BiS", 2, 31049},
        {11, "Balance80PvEP4BiS", 4, 31043},
        {11, "Balance80PvEP4BiS", 5, 34555},
        {11, "Balance80PvEP4BiS", 6, 31046},
        {11, "Balance80PvEP4BiS", 7, 34572},
        {11, "Balance80PvEP4BiS", 8, 34446},
        {11, "Balance80PvEP4BiS", 9, 31035},
    };

    // Iconic weapons are cosmetics only, including level 60/70 legendary models.
    // A two-handed cosmetic hides an equipped offhand visually while retaining its real stats.
    // Apply HiddenItem only when an actual item occupies that slot; never add an offhand item.
    inline constexpr Row WeaponRows[] =
    {
        // Fury80PvEP4BiS
        {1, "Fury80PvEP4BiS", 15, 32837},
        {1, "Fury80PvEP4BiS", 16, 32838},

        // Arms80PvEP4BiS
        {1, "Arms80PvEP4BiS", 15, 49623},
        {1, "Arms80PvEP4BiS", 16, HiddenItem},

        // Retribution80PvEP4BiS
        {2, "Retribution80PvEP4BiS", 15, 49623},
        {2, "Retribution80PvEP4BiS", 16, HiddenItem},

        // Holy80PvEP4BiSHeal
        {2, "Holy80PvEP4BiSHeal", 15, 46017},

        // Marksmanship80PvEP4BiS
        {3, "Marksmanship80PvEP4BiS", 17, 34334},

        // Survival80PvEP4BiS
        {3, "Survival80PvEP4BiS", 17, 34334},

        // Combat80PvEP4BiS
        {4, "Combat80PvEP4BiS", 15, 32837},
        {4, "Combat80PvEP4BiS", 16, 32838},

        // Assassination80PvEP4BiS
        {4, "Assassination80PvEP4BiS", 15, 32837},
        {4, "Assassination80PvEP4BiS", 16, 32838},

        // Discipline80PvEP4BiSHeal
        {5, "Discipline80PvEP4BiSHeal", 15, 46017},

        // Shadow80PvEP4BiS
        {5, "Shadow80PvEP4BiS", 15, 22631},
        {5, "Shadow80PvEP4BiS", 16, HiddenItem},

        // Unholy80PvEP4BiS
        {6, "Unholy80PvEP4BiS", 15, 49623},
        {6, "Unholy80PvEP4BiS", 16, HiddenItem},

        // Frost80PvEP4BiS
        {6, "Frost80PvEP4BiS", 15, 49623},
        {6, "Frost80PvEP4BiS", 16, HiddenItem},

        // Restoration80PvEP4BiSHeal
        {7, "Restoration80PvEP4BiSHeal", 15, 46017},

        // Elemental80PvEP4BiS
        {7, "Elemental80PvEP4BiS", 15, 46017},

        // Arcane80PvEP4BiS
        {8, "Arcane80PvEP4BiS", 15, 22589},
        {8, "Arcane80PvEP4BiS", 16, HiddenItem},

        // Fire80PvEP4BiS
        {8, "Fire80PvEP4BiS", 15, 22589},
        {8, "Fire80PvEP4BiS", 16, HiddenItem},

        // Affliction80PvEP4BiS
        {9, "Affliction80PvEP4BiS", 15, 22630},
        {9, "Affliction80PvEP4BiS", 16, HiddenItem},

        // Destruction80PvEP4BiS
        {9, "Destruction80PvEP4BiS", 15, 22630},
        {9, "Destruction80PvEP4BiS", 16, HiddenItem},

        // Restoration80PvEP4BiSHeal
        {11, "Restoration80PvEP4BiSHeal", 15, 46017},

        // Balance80PvEP4BiS
        {11, "Balance80PvEP4BiS", 15, 22632},
        {11, "Balance80PvEP4BiS", 16, HiddenItem},
    };
}

#endif
