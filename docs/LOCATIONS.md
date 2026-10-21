# Locations

The complete package provides 35 city/social hubs, 20 faction quest hubs and 22 PvP zones.
All controls are in `mod_playerbots_city_life.conf`; data and coordinates remain in WORLD SQL.

## Existing requested locations

| Location | Resident hub | Quest hub |
| --- | --- | --- |
| Booty Bay | `BootyBay` | — |
| Duskwood / Darkshire | `Darkshire` | — |
| Rebel Camp / STV | `RebelCamp` | `RebelCampQuesting` (Alliance) |
| Valiance Keep / Farshire | `ValianceKeep` | `FarshireQuesting` (Alliance) |
| Warsong Hold | `WarsongHold` | `WarsongHoldQuesting` (Horde) |

Those original hubs and populations are retained. The questing master switch defaults to `0`;
set it to `1` to activate enabled quest hubs. Town residents and questers are independent.

## Additional towns

Apply `data/sql/manual/world_city_life_locations.sql`. These new hubs default to Enable = 0.

| Config hub name | Faction | Requested residents |
| --- | --- | --- |
| `Lakeshire` | Alliance | 6 |
| `SentinelHill` | Alliance | 6 |
| `Theramore` | Alliance | 8 |
| `MenethilHarbor` | Alliance | 6 |
| `Auberdine` | Alliance | 6 |
| `Crossroads` | Horde | 8 |
| `RazorHill` | Horde | 6 |
| `Brill` | Horde | 6 |
| `Stonard` | Horde | 6 |
| `CampTaurajo` | Horde | 6 |
| `Ratchet` | Either | 8 |
| `Everlook` | Either | 6 |
| `CenarionHold` | Either | 6 |
| `LightsHopeChapel` | Either | 6 |

## Additional quest hubs

Each faction has a distinct starting hub and its own Enable/Population settings.
Their routes and objectives can still overlap naturally in the shared zone.
The proposed level intervals and radii are supported by local quest/POI data;
they do not guarantee every quest in a zone is suitable for autonomous bots.

| Config hub name | Faction | Bot levels | Requested questers | Radius (yards) |
| --- | --- | --- | --- | --- |
| `SouthshoreQuesting` | Alliance | 25–35 | 6 | 800 |
| `TarrenMillQuesting` | Horde | 25–35 | 6 | 700 |
| `RefugePointeQuesting` | Alliance | 30–40 | 6 | 1000 |
| `HammerfallQuesting` | Horde | 30–40 | 6 | 1800 |
| `AstranaarQuesting` | Alliance | 20–30 | 6 | 2300 |
| `SplintertreePostQuesting` | Horde | 20–30 | 6 | 1000 |
| `TelaarQuesting` | Alliance | 64–68 | 6 | 700 |
| `GaradarQuesting` | Horde | 64–68 | 6 | 1500 |
| `AllerianStrongholdQuesting` | Alliance | 62–66 | 6 | 900 |
| `StonebreakerHoldQuesting` | Horde | 62–66 | 6 | 1400 |
| `ValgardeQuesting` | Alliance | 68–72 | 6 | 700 |
| `VengeanceLandingQuesting` | Horde | 68–72 | 6 | 700 |
| `AmberpineLodgeQuesting` | Alliance | 73–76 | 6 | 1200 |
| `ConquestHoldQuesting` | Horde | 73–76 | 6 | 1100 |
| `GromgolQuesting` | Horde | 30–40 | 6 | 900 |
| `HonorHoldQuesting` | Alliance | 58–63 | 6 | 1100 |
| `ThrallmarQuesting` | Horde | 58–63 | 6 | 1000 |

## Additional PvP hotspots

Apply `data/sql/manual/world_pvp_life_locations.sql`. These new zones default to Enable = 0.
MinPopulation/MaxPopulation are totals for both teams. Rally and target are separate
ground positions chosen from existing NPC patrols or spawns.

| Config zone name | Bot levels | Total bots |
| --- | --- | --- |
| `Southshore_TarrenMill` | 20–40 | 10–20 |
| `Halaa` | 64–70 | 10–24 |
| `BlackrockMountain` | 50–60 | 8–16 |
| `ArathiHighlands` | 30–45 | 8–16 |
| `Ashenvale` | 20–35 | 8–16 |
| `Grizzly_Blackriver` | 73–80 | 10–20 |
| `Grizzly_VentureBay` | 73–80 | 10–20 |

These are generic skirmishes. Halaa/Grizzly objective capture, bombing and other
special objectives are not added by these location definitions.
Movement routes have not been validated on a running server or against local navmesh files.
The new locations are opt-in so they can be activated and tuned for your installation.
