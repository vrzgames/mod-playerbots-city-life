# City Life

Created by **iCore**.

My goal is to make bots feel like the population of a busy **1,000–2,000-player server**.
Populations, locations and features are configurable, so you can manually tune the activity
to fit your own server.

## Major update — many fixes and new features

This update brings **City Life and PvP Life together**, expands city activity and fishing, and adds
many movement, equipment-recovery and configuration fixes.

- **PvP Life is included:** world PvP, duel zones, real-player duel challenges and faction campaigns.
- **One organized config:** 653 options, with separate city, quest-zone, PvP, Custom and Fishing sections.
- **Human behavior:** rare inspect requests and weapon sheath changes; optional owned cosmetic
  companions and idle sitting, with movement features held off for navigation safety.
- **More places to explore:** additional towns, faction quest hubs and world-PvP locations.
- **Fishing Life:** 20 waterfront locations, 46 default slots, per-location populations and a separate bot pool.
- **Custom city activity:** Legendary residents, roaming beggars with occasional trade requests,
  buyers, training scenes, player reactions and conversations.
- **Recognizable Legendary characters:** class variety, The GOATS guild, varied HK/achievement history,
  enchanted/gemmed BiS profiles, actual legendary weapons and a Swift Spectral Tiger mount.
- **Many fixes:** checked movement paths and ground positions, equipment recovery and database
  transaction fixes, bot-reservation handling, fishing clearance and Stormwind plaza placement.
- **Easier updates:** additive location SQL, a config migration tool and clear online-bot budget notes.

See the [full changelog](CHANGELOG.md) and [upgrade instructions](#installation).

> [!IMPORTANT]
> PvP Life is now part of this module. Move the old standalone `mod-playerbots-pvp-life` folder
> outside `modules` before building. Keep its existing database tables and migrate your config values.
> Run CMake Configure/Generate, rebuild and restart worldserver for this source update.

City Life now includes PvP Life in the same module. Cities, questing and PvP keep their existing
settings, database tables and GM commands. All options now live in one clearly grouped
`mod_playerbots_city_life.conf` file. See [locations](docs/LOCATIONS.md) for the full location list
and [PvP Life](docs/PVP_LIFE.md) for its full configuration
and command reference, and [PvP history](docs/PVP_CHANGELOG.md) for its earlier releases.
See [Custom city life](docs/CUSTOM_LIFE.md) for the six additional city roles and social behaviors.
Legendary residents now have full enchanted/gemmed Phase 4 BiS profiles, persistent veteran history,
and the shared **The GOATS** guild. Configure class variety and each city's population independently.
Their veteran history varies by fixed character identity: 11,000-25,000 HK and 5,100-6,000
achievement-point targets by default, with separate range and fixed-value options in the config.
Legendary residents use the previous equipment system: enchanted/gemmed BiS armor and accessories,
actual iconic legendary weapons and a Swift Spectral Tiger mount appearance. No transmog integration
is required. Original equipment is restored when the role ends.

## Human behavior

Ordinary City Life residents and questers can occasionally inspect nearby characters or change
their weapon sheath state while out of combat and near a real player. Only idle ordinary city
residents can summon their own cosmetic companions or use optional sitting. Custom characters,
fishers, PvP Life participants, grouped bots and human-controlled bots are excluded.

Use `CityLife.HumanBehavior.Enable` and each feature's independent switch in config section **02**.
The component requires `CityLife.Enable = 1`, uses bounded rotating checks and per-bot cooldowns,
and does not change MMaps, movement generators, paths or combat targets. Inspect has no visible
animation. Companion handling never touches Hunter or Warlock combat pets.

Physical random jump and strafe remain disabled even if configured on: the current APIs replace
active movement. Sitting defaults off because a sitting player can receive a critical first melee
hit; AI posture, rest and movement take priority. Companions are also opt-in pending in-game
verification. No SQL/schema migration is needed for this update.
See [Human behavior settings, safety limits and verification](docs/HUMAN_BEHAVIOR.md).

## Fishing Life

Fishing has its own component, config section **11** and `FishingLife.*` options. Set
`FishingLife.Enable = 0` to stop it, or adjust each waterfront's `Population` independently.
It remains active when `CityLife.Custom.Enable = 0`; the shared `CityLife.Enable` scheduler
must still be enabled. The default profile requests 46 fishers across 20 waterfront locations,
including Ironforge, Dalaran, faction capitals, ports, tuskarr villages and Wintergrasp.

Fishers spread between waterfront anchors, check reachable water before placement/casting,
and pause at Wintergrasp during battles by default. They display a fishing pole and cast;
they do not farm loot. Missing participants can use a separate, bounded Fishing Life bot pool.
Apply the additive `data/sql/manual/world_city_life_fishing.sql` update once and use the new
binary/config. Existing fishing characters and recovery records are retained.
See [Fishing Life locations and settings](docs/FISHING_LIFE.md).

## Movement bug fixes — issue #1

Fixed City Life movement handling behind town-entrance rubber-banding, straight-line shortcuts and
mid-air running reported in [issue #1](https://github.com/vrzgames/mod-playerbots-city-life/issues/1).

- Normal relocation is no longer interrupted by distance-based recovery teleports.
- Randomized destinations are checked against ground height and the intended vertical layer.
- Roaming uses validated, non-forced navmesh paths; unsafe or missing paths are rejected instead
  of falling back to a straight line through obstacles or the air.
- Interrupted movement is retried with bounded recovery rather than a repeated teleport loop.

Rebuild and restart worldserver to use the fix. Valid map, VMap and MMap data are required, with
`MoveMaps.Enable = 1`; this change does not generate or repair navigation data. In-game confirmation
at the affected entrances is still required before closing the issue.

## About

City Life populates capitals, towns and popular social hubs with configurable AzerothCore
Playerbots. City residents gather near auction houses, banks, mailboxes, flight masters and public
squares. Optional questing hubs instead reserve real playerbots that accept quests, perform their
objectives and turn in completed quests through Playerbots. Beggars can send occasional flavor
trade requests; they do not complete exchanges or manipulate the auction house economy.

The included PvP Life component adds persistent world-PvP hotspots, capital duel zones, optional
real-player duel challenges and For the Horde / For the Alliance activities. PvP is enabled by
default and can be disabled independently with `PvPLife.Enable = 0`.

## Features

- Per-hub population settings, including values such as 50 bots in Stormwind and 5 in Exodar.
- Weighted hotspot distribution instead of stacking every bot at one coordinate.
- Extra stationary residents around Stormwind and Orgrimmar auction houses and banks.
- Gradual arrivals and departures.
- Morning, daytime, evening and nighttime population multipliers.
- Occasional movement between service and social hotspots.
- Sparse, configurable social emotes.
- Faction and level filtering for every hub.
- Opt-in real questing around configurable zone hubs, with a local quest radius and idle rotation.
- Global population safety cap with priority-based allocation.
- Runtime GM tools for adding hubs and recording exact hotspot coordinates in game.
- Bundled AzerothCore 3.3.5 data for 35 city/social hubs, 20 faction questing hubs and 22 PvP zones.
- One configuration file, grouped by cities, quest zones, duels and world-PvP locations.
- Optional new towns and separate-faction quest starts; new locations default to disabled.
- Included PvP Life systems with their existing zones, settings and `.pvplife` GM commands.
- Custom legendary AFK characters, level-1 beggars, buyer advertisements and training scenes.
- Legendary BiS profiles include enchants, gems and class-specific legendary weapon overrides.
- Beggars roam their plaza, pause to beg/dance, and occasionally request a short trade with nearby players.
- Separate Fishing Life component with 20 waterfronts, per-location populations and its own provisioning pool.
- Reactions to real-player emotes and short, coherent conversations between existing residents.
- A separate default-on Custom config section with per-role switches and per-city populations.

Bundled hubs:

- Stormwind, Ironforge, Darnassus and Exodar
- Orgrimmar, Undercity, Thunder Bluff and Silvermoon
- Shattrath and Dalaran
- Alliance and Horde Wintergrasp starting camps
- Gadgetzan and Goldshire
- Honor Hold and Thrallmar
- Booty Bay, Darkshire and Rebel Camp
- Valiance Keep and Warsong Hold
- Optional Lakeshire, Sentinel Hill, Theramore, Menethil Harbor and Auberdine
- Optional Crossroads, Razor Hill, Brill, Stonard and Camp Taurajo
- Optional Ratchet, Everlook, Cenarion Hold and Light's Hope Chapel

Bundled questing hubs:

- `RebelCampQuesting`: Alliance, Stranglethorn Vale (zone 33), levels 30-40
- `FarshireQuesting`: Alliance, Borean Tundra (zone 3537), levels 68-72
- `WarsongHoldQuesting`: Horde, Borean Tundra (zone 3537), levels 68-72
- Optional faction pairs for Hillsbrad, Arathi, Ashenvale, Hellfire, Nagrand, Terokkar,
  Howling Fjord and Grizzly Hills; optional Grom'gol complements Rebel Camp in STV.

See [all locations and populations](docs/LOCATIONS.md). Questing remains globally opt-in: set
`CityLife.Questing.Enable = 1` and enable the individual quest hubs you want.

## Requirements

- AzerothCore `azerothcore-wotlk` Playerbot branch
- `mod-playerbots`
- A configured pool of online random playerbots

Normal City Life and PvP borrow already-online playerbots. Custom roles can additionally create
and log in module-owned participants when no eligible bot is available and
`CityLife.Custom.CreateMissing.Enable = 1` (the default). Provisioning is capped at 30 managed
characters and one request per 30 seconds by default. These sessions are additional to the normal
Playerbots online pool; include them when counting total online bots and server capacity.
Fishing Life has its own provisioning settings and a separate default quota of 46 persistent
characters. Both components prefer reusable eligible participants before creating new characters.
Existing characters are never lowered to another level. See [Custom city life](docs/CUSTOM_LIFE.md)
and [Fishing Life](docs/FISHING_LIFE.md).
PvP activities borrow from that same online pool, using shared reservations to avoid selecting
residents or questers. Existing City Life and PvP population settings remain separate.

Bots already present in Wintergrasp are protected and never borrowed for other City Life hubs.
The bundled faction-specific Wintergrasp hubs maintain their own reserved populations.

## Installation

1. Install `mod-playerbots-city-life` in the core's `modules` directory. Move the former standalone
   `mod-playerbots-pvp-life` folder outside `modules`, so it is not compiled a second time.
2. For a fresh installation, apply `data/sql/manual/world_city_life.sql` to the WORLD database.
   Apply `data/sql/manual/world_pvp_life.sql` for PvP, which is enabled by default. To run without
   PvP, set `PvPLife.Enable = 0`; its tables are not required while that component is disabled.
3. Apply `data/sql/manual/world_city_life_locations.sql` to add the optional city/quest locations.
   If you installed the PvP base tables, also apply `data/sql/manual/world_pvp_life_locations.sql`.
   Skip the latter on installations without PvP tables. These additive files preserve existing
   named records and customizations and can be rerun safely.
4. For the default-on Custom features, apply `data/sql/manual/world_city_life_custom.sql` to WORLD
   and both `data/sql/manual/characters_city_life_custom.sql` and
   `data/sql/manual/characters_city_life_custom_bots.sql` to CHARACTERS. These add Custom data,
   equipment recovery and a managed-character registry; installation does not modify inventories.
   `world_city_life_legendary_stormwind.sql` updates the older default Stormwind Legendary plaza position/facing.
5. Apply `data/sql/manual/world_city_life_fishing.sql` to WORLD for the expanded Fishing Life
   locations. It preserves the original fishing hub identities and adds new named anchors.
   Apply `world_city_life_fishing_spacing.sql` for clearance around the original Stormwind trainer point.
6. Copy `conf/mod_playerbots_city_life.conf.dist` to your worldserver modules configuration area as
   `mod_playerbots_city_life.conf`. This is the only distributed module config. All old `CityLife.*`
   and `PvPLife.*` names and values are retained, except former `CityLife.Custom.Fishing.*`
   options now use `FishingLife.*`. The merge utility migrates those fishing values.
7. Reconfigure CMake, rebuild, then restart worldserver so the new binary discovers the sources and
   the single configuration filename.

SQL files install location data; routine on/off and population adjustments are made in the config.
Coordinates, faction assignments, hotspot weights and chat texts remain in the existing WORLD tables.
City and PvP validate their tables independently. `CityLife.Enable` controls residents and questers;
`PvPLife.Enable` independently controls PvP. The latter remains enabled by default.
`CityLife.Custom.Enable` controls six Custom features; `FishingLife.Enable` independently
controls fishing. Both default to `1` and require `CityLife.Enable = 1`. Each Custom role also
has a separate switch. See [Custom city life](docs/CUSTOM_LIFE.md) for setup and limits.

### Updating from two configuration files

Keep backups of your active `.conf` files. The server will load only
`mod_playerbots_city_life.conf` after rebuilding; the old `mod_playerbots_pvp_life.conf` is no longer
automatically read. Carry your customized values into the combined file before restarting.

The included Python 3 utility uses the new template, copies your existing values, and preserves
custom `CityLife.*`, `PvPLife.*` and `FishingLife.*` keys and notes. It also moves old
`CityLife.Custom.Fishing.*` values to `FishingLife.*`; explicit new keys take precedence.
It writes a separate file and never edits inputs:

```sh
python tools/merge_life_config.py --city-config OLD_CITY.conf --pvp-config OLD_PVP.conf --output NEW_CITY.conf
```

Run it from the module directory, then review `NEW_CITY.conf` and place it in the worldserver modules
config directory as `mod_playerbots_city_life.conf`. Either input argument can be omitted if you had
only one component. Output directories must exist; replacing an existing output needs `--force`.
Only Life-module settings are accepted. If an old module config also contains unrelated core or
Playerbots overrides, move those to their normal configuration file before using the utility.
Duplicate keys follow the core's existing precedence: first occurrence within a source file wins;
the former PvP file takes precedence over the former City file. Values are not printed by the tool.

Existing `pvp_life_zone`, `pvp_life_chat`, `city_life_hub` and `city_life_spot` data remain compatible.
If you previously used only City Life, apply `world_pvp_life.sql` once before the PvP location update,
or set `PvPLife.Enable = 0` and skip both PvP SQL files. If you previously used only PvP Life,
apply `world_city_life.sql` once to create the missing City tables before the City updates.
Apply a component's base seed only when its tables do not exist; preserve the installed component's data.
Apply the two `*_locations.sql` files for optional geographic additions,
`world_city_life_custom.sql` and `characters_city_life_custom*.sql` for Custom features, and
`world_city_life_fishing.sql` for Fishing Life. Do not reapply full base seed scripts over
customized data. Installations predating the three original quest hubs can also
apply `world_city_life_quest_zones.sql` once before the new locations file.

### Configuration precedence and activation

- An explicit `CityLife.Hub.<name>.Enable` or `PvPLife.Zone.<name>.Enable` value takes precedence
  over the database enabled flag. You can enable or disable a seeded place without SQL editing.
- Locations without an explicit Enable key continue to use their live database flag, including GM
  changes. GM commands remain available and save location data to WORLD; explicit config values
  take precedence over database enable/population settings.
- Optional city/quest additions are initially disabled; Custom features and Fishing Life default on.
- Fishing Life uses `FishingLife.<Location>.Population` (0 disables that location), independent of
  the historical DB enabled flag and `CityLife.Custom.Enable`. Set its master to 0 to stop all fishers.
  The 300-bot City cap remains unchanged. Enabling many locations increases requested population; keep enough eligible
  online bots and adjust the cap or individual counts to fit your server.
- Questing additionally needs `CityLife.Questing.Enable = 1`; its original default remains `0`.
- Use `.reload config` or restart after changing the config file. `.citylife reload` only reloads
  database hub/spot data using the already-loaded config values.
- Per-zone PvP changes apply when the next event starts. `PvPLife.Enable = 0` stops active PvP
  activities, releases reservations and cancels pending/active module duels on config reload.

## Population configuration

Each seeded hub has an explicit setting:

```ini
CityLife.Hub.Stormwind.Population = 50
CityLife.Hub.Ironforge.Population = 50
CityLife.Hub.Exodar.Population = 5
CityLife.Hub.Goldshire.Population = 10
```

Every database hub, including a custom one, automatically supports these keys:

```ini
CityLife.Hub.<HubName>.Enable = 1
CityLife.Hub.<HubName>.Population = 20
CityLife.Hub.<HubName>.TimeScaling = 1
CityLife.Hub.<HubName>.ResidenceSeconds = 0
```

If a custom hub has no population key, `default_population` from `city_life_hub` is used. The
configured number is multiplied by the current time-of-day percentage. With a population of 40
and a nighttime multiplier of 25, the nighttime target is 10 residents.

`TimeScaling = 0` keeps the configured population constant all day. `ResidenceSeconds` rotates a
resident after the configured time; `0` keeps the same resident until it becomes unavailable or
the target population decreases. The bundled Wintergrasp hubs use 40 bots per faction, ignore
time scaling and rotate residents every three hours.

The worldserver machine's local time is used:

```ini
CityLife.Time.MorningMultiplierPct = 60
CityLife.Time.DayMultiplierPct = 80
CityLife.Time.EveningMultiplierPct = 100
CityLife.Time.NightMultiplierPct = 25
```

Changes happen gradually according to `CityLife.Population.MaxChangesPerTick`; bots do not appear
or disappear as one large group.

City residents and questers share `CityLife.Population.MaxTotal`, whose template default remains 300.
The original hubs request 340 bots with their three quest hubs enabled at full time scaling; enabling
all optional towns and quest hubs raises that request to 532. The 16 default Custom slots and
46 Fishing slots bring the request to 594 before the shared cap. Both components share this cap. Adjust the
cap or individual populations to fit your available eligible online bots. PvP has separate limits
and does not count toward this cap.

### Leave free bots for dungeons and battlegrounds

Plan `online bots >= Life reservations + concurrent dungeon/BG bot slots + spare bots`. Count
normal Playerbots sessions and additional Custom/Fishing-managed sessions in the online total. Count Life
reservations as unavailable when sizing that free pool. City residents and questers suppress normal
BG/LFG joining, and the local managed BG recruitment path skips shared Life reservations. This is
not a blanket LFG prohibition for every PvP bot. External/manual recruitment can release a city or
quest assignment; City Life does not automatically restore that same quest-hub assignment afterward.

With Custom and Fishing defaults, targets are 286 in the morning, 300 by day, 300 in the evening
after the cap, and 205 at night. These include 16 Custom and 46 Fishing slots; missing eligible bots can reduce
actual reservations. Questing and optional geographic additions are initially off. Automatic PvP
can request another 140 bots: 32 in the two duel zones, up to 80 across three distinct normal hotspots,
and 28 for one faction campaign. Use **440 reserved bots** as a default planning peak. It is not a
universal hard limit: modified settings and GM force-starts can exceed it, while actual reservations
can be lower when bots are unavailable or events are inactive.

| Total online bots | Life planning peak | Nominally free bots |
| --- | --- | --- |
| 500 | 440 | 60 |
| 1,000 | 440 | 560 |
| 2,000 | 440 | 1,560 |

Use **1,000 online bots** as an illustrative budget for the current profile; **2,000** leaves more
room in the bot budget to enable additional locations. These are population examples, not measured
minimums or server-performance guarantees. If online population varies, plan against
`Playerbots.MinRandomBots` plus the actually online Custom/Fishing-managed participants, rather than
the maximum or total stored characters. `CreateMissing.MaxBots` is a separate quota. Increasing counts also
needs sufficient server capacity; reducing Life populations is another way to leave more free bots.

For an expansion example at 2,000 online bots, all city/quest hubs plus default Custom and Fishing
request 594 bots at full time scaling. If `CityLife.Population.MaxTotal` is raised to at least 594,
and other default PvP limits stay unchanged, budget up to **734 Life bots**, leaving
**1,266 nominally free**. This is a planning example only; the distributed cap remains 300, Custom
features default on, and optional geographic additions remain disabled.

Check the free pool by **faction, level bracket and role**, especially when bots are still leveling.
Low-level bots cannot fill level-80 heroic dungeon slots. A five-player dungeon containing one real
player needs four eligible bots in the required roles; battleground demand includes open slots on
both teams. A large total free count does not guarantee enough level-80 tanks/healers or bots in a
particular BG bracket. The module does not enforce a minimum free pool, so adjust the hub counts,
PvP populations and simultaneous-hotspot limit to leave the capacity your players need.

## Questelo playerbotok / real questing by zone

Questing is opt-in in the distributed template:

```ini
CityLife.Questing.Enable = 1
CityLife.Hub.RebelCampQuesting.Enable = 1
CityLife.Hub.RebelCampQuesting.Population = 8
CityLife.Hub.RebelCampQuesting.Questing = 1
CityLife.Hub.RebelCampQuesting.QuestZoneId = 33
CityLife.Hub.RebelCampQuesting.QuestRadius = 1200
CityLife.Hub.RebelCampQuesting.QuestIdleSeconds = 300
CityLife.Hub.RebelCampQuesting.MinLevel = 30
CityLife.Hub.RebelCampQuesting.MaxLevel = 40
CityLife.Hub.RebelCampQuesting.TimeScaling = 0
CityLife.Hub.RebelCampQuesting.ResidenceSeconds = 3600
```

The template sets the global switch to `0`; set it to `1` as above to enable questing hubs.
`QuestZoneId` is the zone ID, not the map ID. `QuestRadius` is the radius in yards around the
database hub center. The selector uses valid local quest givers, objectives and turn-ins inside
that radius and in the same zone. Nearby NPC interactions may also accept a chain or travel quest,
but distant goals are not selected for this local activity.

These bots actually use Playerbots quest pickup, objective and reward actions; the module does not
spawn cosmetic quester NPCs, fabricate quest progress or automatically complete their quests.
Questers are not held by the city `stay` strategy, social-spot leash, relocation or emote behavior.
The bundled Farshire and Warsong Hold hubs each request eight level 68-72 bots; Rebel Camp requests
eight level 30-40 bots. Keep enough eligible, already-online random bots of the correct faction and
level available. `QuestIdleSeconds` tracks actual quest movement, objective progress, pickup and
reward activity. Merely keeping a quest selected or patrolling without an active quest does not
refresh the idle timer. Idle bots are released, while `ResidenceSeconds` provides scheduled rotation.

Questers always require full Playerbots AI activity, even when
`CityLife.Bots.RespectPlayerbotActivity = 0`; that bypass is only for city residents. Activity is
rechecked every 30 seconds after arrival, and inactive questers can then be released and replaced
when safe. City Life does not force global AI activity on. This installation keeps
`Playerbots.BotActiveAlone = 10` unchanged, so the eligible pool can be smaller away from real
players; player proximity and the other Playerbots activity rules also affect availability.

To add another quest zone, create a database hub and its safe starting spot with the GM commands
below, then add the same `CityLife.Hub.<HubName>` questing keys to the config. The hub's faction
and the configured level interval determine which bots can be borrowed. Config keys alone cannot
create a hub or increase the Playerbots online population.

## Hotspots

Each hub contains one or more weighted spots. Higher weights receive proportionally more
residents. The bundled cities use categories such as `auction`, `bank`, `mail`, `flight`, `square`
and `travel`. Categories are descriptive and can be extended freely.

A small random offset inside the spot radius prevents character stacking. Keep radii conservative
inside buildings. Hotspot coordinates can differ on custom maps or databases, so tune them from a
safe player-accessible position with the GM commands below.

Stormwind and Orgrimmar auction-house and bank spots receive 2.5 times their normal selection
weight by default. Eighty percent of residents initially assigned there remain stationary instead
of periodically moving to another hotspot. The multiplier, chance, hubs and service categories are
all configurable under `CityLife.Behaviour`.

## GM commands

```text
.citylife status
.citylife races [hub|all]
.citylife reload
.citylife hub list
.citylife hub add <name> <alliance|horde|any> <minLevel> <maxLevel> <population> [priority]
.citylife hub population <name> <count>
.citylife hub enable <name>
.citylife hub disable <name>
.citylife spot list <hub>
.citylife spot add <hub> <name> <category> [weight] [radius]
.citylife spot delete <hub> <name>
```

`hub add` saves the GM's current map and position and creates an initial `MainSquare` spot when the
hub has no spots. `spot add` saves the GM's exact current position. Reusing the same hub/spot name
updates it, which makes in-game coordinate tuning quick.

`hub population` and `hub enable|disable` change database defaults. Explicit `.conf` population
and Enable values take precedence. Use `.citylife reload` after direct database edits; GM commands reload data
automatically.

## Race distribution diagnostics

Use `.citylife races Stormwind` for one hub or `.citylife races all` (also the default) for all hubs.
The command reports each race's resident count and percentage alongside the currently eligible,
online, unreserved candidate count and percentage. Candidate counts are taken before
`CityLife.Bots.QueryLimit`; global results count each candidate once across enabled hubs with spots.
A named hub uses its own faction, level and safety filters, even if that hub is disabled.

Residents are already reserved, so they are not included in the available-candidate column. These
numbers are not the complete server-wide online population. Normal selection does not prefer
Dwarves or force equal race quotas; it depends on the eligible Playerbots pool. The optional
Custom CreateMissing facility can also supply module-owned characters for missing Custom roles.
This diagnostic helps investigate the reported Dwarf-heavy population without changing selection.
The command is GM/console-only and does not alter bot behavior or consume recruitment randomness.

## Behaviour and safety

For normal residents, City Life owns non-combat relocation and idle anchors while reserved. Competing stay,
follow, grinding, loot/gather, travel and autonomous RPG movement are disabled; combat strategies
are not replaced. Questing hubs instead allow local quest movement and interaction.
Questing strategies suppress autonomous BG and LFG joining. If a quester is recruited through
another path and becomes grouped, BG-queued or instanced, City Life releases its hub assignment
without clearing its quest log. There is no saved automatic return to that same quest hub.
Both can still defend themselves if attacked. Normal city/quest handling never interrupts combat and never borrows
bots that are dead, grouped, in an instance, in a battleground,
in flight, controlled by a real player or already using the Playerbots duel strategy.

The random-playerbot teleport timer is held while a bot is a resident. When released, the bot's
normal strategies and random teleport schedule are restored. By default it is also returned to the
position from which City Life borrowed it, but only if that destination passes the same safety
checks. A refused return leaves the released bot at its current position without a forced fallback.

Custom characters and Fishing Life participants pause normal autonomous actions and perform their own activity.
They reject group/BG/LFG recruitment and item transfers while assigned. An outside attack releases
the role; equipment recovery completes before normal AI resumes. Practice-dummy combat is owned
by the Custom activity and is stopped when that assignment ends.

Beggars have independent roaming and trade-request settings under `CityLife.Custom.Beggar`.
Their short trade requests use the normal client protocol, expire automatically, and do not complete exchanges.

City residents, questers and the included PvP Life component use a shared in-memory bot reservation
registry, so they never borrow the same bot. The registry uses only public module code; no
compatibility patch is required. Their existing population limits and behavior remain separate.

## Scope

City Life intentionally does not:

- create auction listings or buy items;
- simulate banking or mailbox transactions;
- alter normal Playerbots random-population targets (Custom and Fishing provisioning have separate bounded pools);
- replace normal residents' or questers' Playerbots combat AI;
- teleport or control real players and player-owned bots.

Its job is to populate important World of Warcraft locations with city residents, optional local
questers and configurable PvP activity without altering the server economy.
