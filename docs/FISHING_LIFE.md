# Fishing Life

Fishing Life is a separate component inside the City Life module. Its settings are in section
**11 - FISHING LIFE** of the single `mod_playerbots_city_life.conf` file, under `FishingLife.*`.
`CityLife.Custom.Enable = 0` leaves fishing running. Fishing requires `CityLife.Enable = 1`,
the shared scheduler, and `FishingLife.Enable = 1`; PvP has its own independent switch.

## Locations and populations

All these fishing locations default on. Each has its own `Enable`, `Population`, `MinLevel`
and `MaxLevel`; `Population = 0` also disables a location. Fishing settings do not enable
ordinary residents or questers at that same place.

| Config location | Waterfront | Default population | Default level range |
| --- | --- | --- | --- |
| Stormwind | Canals and harbor | 3 | 1-80 |
| Orgrimmar | Valley of Honor | 3 | 1-80 |
| Dalaran | Fountain | 3 | 68-80 |
| Ironforge | Forlorn Cavern | 2 | 1-80 |
| ThunderBluff | Pond | 2 | 1-80 |
| Undercity | Canals | 2 | 1-80 |
| Darnassus | Lake | 2 | 1-80 |
| BootyBay | Waterfront | 2 | 30-80 |
| Ratchet | Waterfront | 2 | 10-80 |
| Theramore | Waterfront | 2 | 35-80 |
| Auberdine | Waterfront | 2 | 10-80 |
| MenethilHarbor | Waterfront | 2 | 20-80 |
| CrystalLake | Elwynn Forest | 2 | 5-80 |
| ValianceKeep | Shore | 3 | 68-80 |
| WarsongHold | Garrosh's Landing coast below the hold | 2 | 68-80 |
| Wintergrasp | Lakeside | 2 | 70-80 |
| MoakiHarbor | Tuskarr waterfront | 3 | 68-80 |
| Kamagua | Tuskarr waterfront | 3 | 68-80 |
| Unupe | Tuskarr waterfront | 2 | 68-80 |
| DalaranUnderbelly | Underbelly water | 2 | 68-80 |
| **Total** | **20 locations** | **46** | |

For example:

```ini
FishingLife.Enable = 1
FishingLife.Ironforge.Enable = 1
FishingLife.Ironforge.Population = 4
FishingLife.Ironforge.MinLevel = 20
FishingLife.Ironforge.MaxLevel = 80
FishingLife.Wintergrasp.Enable = 0
```

Global `FishingLife.MinLevel` / `MaxLevel` constrain the per-location range. A newly provisioned
fisher starts at the location's allowed minimum. Existing characters are never lowered to match.
Neutral locations accept both factions; faction cities use their own faction. Coordinates,
anchor radii, faction assignments and weights remain in WORLD as location data.

Populations are upper targets, not guaranteed counts. Fishers spread between anchors and keep
`FishingLife.MinimumSpacing = 2` yards apart. Each destination must have safe, dry, navigable
ground, nearby real water and line of sight. If safe space is exhausted, the module leaves
slots empty rather than placing fishers on top of one another. Increasing Population may
also require more suitable anchors recorded in game with the existing `.citylife` tools.

## Fishing behavior

Participants equip the configured pole and periodically cast a real fishing spell/channel.
Default settings are `PoleItem = 6256`, `Spell = 7620`, and a 20-35 second cast interval.
These scenes do not collect loot, generate items or farm fishing skill. Borrowed bots must
already qualify to equip the pole; newly provisioned fishing characters receive fishing ability.
The original equipment is journaled and restored on release, including after interrupted sessions.

`FishingLife.MaximumWaterHeightDifference = 12` allows casting from docks above the water.
Water depth and line-of-sight checks still apply. Core fishing randomizes the bobber position,
so an occasional rejected cast near a narrow pool can be retried on the next interval.
The supplied anchors were checked against local WORLD/terrain data; final placement also
depends on your map, VMap and MMap files. These locations still need in-game verification.

`FishingLife.Wintergrasp.PauseDuringBattle = 1` temporarily requests zero Wintergrasp fishers
during an active battle. Existing assignments are released through normal equipment recovery;
the scheduler can fill them again when the battle ends. An outside attack also releases the
fishing role, allowing the bot's normal defensive behavior after recovery.

`TimeScaling = 0` keeps fishing populations constant. Set it to 1 for the City Life time
multipliers. `ResidenceSeconds = 0` keeps an available participant assigned; positive seconds
request rotation. Changes are gradual, using the shared City Life population tick and change limit.

Fishing chat has separate `Chat.Range`, `ChatMinSeconds`, `ChatMaxSeconds` and `YellChancePct`
settings. Defaults are 40 yards, 180-420 seconds and ordinary say. A nearby real player of the
same faction, map and phase is required. Text remains in the historical `city_life_custom_chat`
table with `group_name = fishing`; that table name does not make fishing depend on Custom.

## Separate managed fishing pool

```ini
FishingLife.CreateMissing.Enable = 1
FishingLife.CreateMissing.MaxBots = 46
FishingLife.CreateMissing.CreateIntervalSeconds = 30
FishingLife.CreateMissing.AccountPrefix = "fishlife"
```

The component first uses eligible available participants. When one is missing, it can create
and log in a module-owned character. Creation is gradual and may fail or retry. MaxBots limits
persistent fishing records, including pending work and inactive registered identities; it is
separate from each waterfront's population and the Custom pool's default quota of 30.
Disabling CreateMissing stops new character creation; registered eligible identities can still
be reused. Reducing a quota does not delete existing characters. Account prefixes must use
1-8 letters/digits and must not overlap the Custom or random-Playerbots account prefixes.

Existing original fishing identities are retained even if their accounts use the historical
CITYLIFE prefix. The same equipment journal and managed-character tables are reused; fishing
records count only against the fishing quota. Registry ownership validation remains mandatory.

Managed sessions are additional to Playerbots random online targets. Count all actually online
managed fishers in the total server population. Normal residents, questers, Custom and Fishing
share `CityLife.Population.MaxTotal` (default 300); fishing does not bypass this cap. With the
distributed settings, 16 Custom + 46 Fishing slots contribute to targets of 286/300/300/205
in morning/day/evening/night. PvP can request another 140, so the default Life planning peak
remains 440. At 1,000 total online bots that leaves 560 nominally free; at 2,000, 1,560.
Check faction, BG level bracket and dungeon role availability separately, especially for leveling
bots. These examples describe population budgets, not measured hardware requirements.

## Installation and migration

1. Keep the existing City Life base WORLD tables.
2. Apply `world_city_life_custom.sql` to WORLD for the historical fishing hubs/chat, and both
   `characters_city_life_custom.sql` and `characters_city_life_custom_bots.sql` to CHARACTERS
   if these were not installed previously.
3. Apply `data/sql/manual/world_city_life_fishing.sql` to WORLD. It adds the expanded locations
   and named anchors without replacing existing named settings or character records.
4. Apply `world_city_life_fishing_spacing.sql` to WORLD to move an older default Stormwind
   trainer anchor 3 yards sideways. Customized positions are preserved. Point-only changes
   need `.citylife reload` or a restart, without rebuilding the module.
5. Use the matching rebuilt module and the updated single configuration file, then restart
   worldserver. For later config edits use `.reload config`; `.citylife reload` reloads WORLD
   location/chat data with settings already loaded into memory.

The three original internal hub names (`CustomFishingStormwind`, `CustomFishingOrgrimmar`,
`CustomFishingDalaran`) are preserved so registered characters and equipment recovery still
match. New hubs use `Fishing<Location>`. Every user-facing fishing option uses `FishingLife.*`.

The included `tools/merge_life_config.py` migrates old `CityLife.Custom.Fishing.*` values to
`FishingLife.*`, retaining explicit new keys when both are present. It also carries old shared
Custom time scaling, residence duration, chat range, provisioning toggle and creation interval
into the independent fishing settings when migrating an old fishing configuration. The old
shared account prefix and quota remain with Custom; Fishing Life gets a separate prefix/quota.
Review new location populations before use.
