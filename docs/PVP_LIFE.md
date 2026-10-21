# PvP Life

Included in `mod-playerbots-city-life`; formerly distributed as `mod-playerbots-pvp-life`.

Created by **iCore**.

The PvP Life component makes the open world feel active through persistent, configurable bot PvP
and ships inside City Life. All existing `PvPLife.*` settings, `.pvplife` commands and PvP database
table names are retained.

It is intentionally split into independent systems so each part can be enabled, tuned, or disabled without affecting the others.

### Included systems

- **Persistent World PvP**: keeps a configurable number of PvP hotspots active.
- **Duel Zones**: near-continuous bot-vs-bot duel activity around configured locations such as Stormwind and Orgrimmar; idle bots are repeatedly re-paired after a cooldown.
- **Bot -> real player duel challenges**: bots can challenge nearby real players with strict anti-spam cooldowns and a configurable level-difference check.
- **For the Horde**: Horde bots can push into Alliance territory/capital areas while Alliance bots act as defenders.
- **For the Alliance**: Alliance bots can push into Horde territory/capital areas while Horde bots act as defenders.
- **Natural defender chat**: optional bot `/yell` and world-chat style reactions such as `horde in sw` or `ally in org`.
- **Optional announcements**: server-wide scripted announcements exist for debugging/information but are disabled by default.
- **Randomisation**: bot counts, duration, arrival timing, movement and position jitter are varied to reduce scripted-looking behaviour.
- **Activity protection**: bots in combat, battlegrounds, instances, flight, duels, or (by default) groups are rejected.
- **Activity coordination**: shared bot reservations prevent conflicts with City Life residents and questers.
- **Small-population fallback**: activities can start with smaller teams instead of silently failing when the full random bot count is unavailable.
- **Permanent capital duel zones**: available random bots are kept near the configured Stormwind and Orgrimmar duel points, automatically paired, healed and re-paired after duels.
- **Return handling**: participants can be sent back to their previous location when their activity ends.

## Requirements

- [`mod-playerbots/azerothcore-wotlk`, Playerbot branch](https://github.com/mod-playerbots/azerothcore-wotlk/tree/Playerbot)
- [`mod-playerbots/mod-playerbots`, master branch](https://github.com/mod-playerbots/mod-playerbots)
- C++17-capable build environment used by current AzerothCore.

City Life and its PvP component are designed for the Playerbot fork of AzerothCore and must be built
together with `mod-playerbots`.

## Installation

Follow the [combined installation and update guide](../README.md#installation). For a fresh setup:

1. Install the combined folder at `azerothcore-wotlk/modules/mod-playerbots-city-life`.
2. Apply `data/sql/manual/world_pvp_life.sql` from that folder to the **WORLD** database.
3. Use `conf/mod_playerbots_city_life.conf.dist` as the single template for
   `mod_playerbots_city_life.conf`. All `PvPLife.*` keys remain unchanged inside that file.
   Apply `data/sql/manual/world_pvp_life_locations.sql` for the seven optional new hotspots.
4. Re-run CMake, rebuild the server, then start worldserver and check for `[PvPLife]` log messages.

`PvPLife.Enable = 1` remains the default. Set `PvPLife.Enable = 0` to disable all PvP Life activity
independently of `CityLife.Enable`; PvP tables are not required while PvP is disabled. Disabling city
residents and questers with `CityLife.Enable = 0` does not disable PvP.

When updating from standalone PvP Life, migrate your existing `PvPLife.*` values into the single
City Life config using the [migration guide](../README.md#updating-from-two-configuration-files).
Keep the `pvp_life_zone` and `pvp_life_chat` tables; no schema migration is needed. Do not
reapply the seed SQL over customized data. After backing up local files, move the former PvP folder
outside the core's `modules` directory or remove it, so the standalone implementation is no longer
compiled alongside City Life. Keep any archived copy outside `modules`.

## Main configuration

The most important defaults are:

```ini
PvPLife.Enable = 1
PvPLife.Announce = 0

PvPLife.World.AlwaysActive = 1
PvPLife.World.MinActiveHotspots = 1
PvPLife.World.MaxActiveHotspots = 3

# Individual zone switches (1 = automatic activity enabled, 0 = disabled)
PvPLife.Zone.STV_Nesingwary.Enable = 1
PvPLife.Zone.STV_Nesingwary.MinPopulation = 10
PvPLife.Zone.STV_Nesingwary.MaxPopulation = 20
PvPLife.Zone.STV_Gurubashi.Enable = 1
PvPLife.Zone.STV_Gurubashi.MinPopulation = 12
PvPLife.Zone.STV_Gurubashi.MaxPopulation = 24
PvPLife.Zone.DarkPortal_Azeroth.Enable = 1
PvPLife.Zone.DarkPortal_Azeroth.MinPopulation = 12
PvPLife.Zone.DarkPortal_Azeroth.MaxPopulation = 28
PvPLife.Zone.Shattrath_Outskirts.Enable = 1
PvPLife.Zone.Shattrath_Outskirts.MinPopulation = 10
PvPLife.Zone.Shattrath_Outskirts.MaxPopulation = 24
PvPLife.Zone.Wintergrasp.Enable = 1
PvPLife.Zone.Wintergrasp.MinPopulation = 10
PvPLife.Zone.Wintergrasp.MaxPopulation = 24

PvPLife.Duel.Enable = 1
PvPLife.Duel.AlwaysActive = 1
PvPLife.Duel.GuardIntervalMs = 1000
PvPLife.Duel.ChallengeRealPlayers = 1
PvPLife.Duel.MaxLevelDifference = 5
PvPLife.Duel.BotVsBotMaxLevelDifference = 5
PvPLife.Duel.PlayerChallengeCooldownSeconds = 900

PvPLife.ForTheHorde.Enable = 1
PvPLife.ForTheAlliance.Enable = 1

PvPLife.BotChat.Enable = 1

PvPLife.Bots.RespectPlayerbotActivity = 0
PvPLife.Bots.AllowPartialTeams = 1
PvPLife.Bots.MinimumPerSide = 1
PvPLife.Bots.MaxPerSide = 100
```

### Level check example

With:

```ini
PvPLife.Duel.MaxLevelDifference = 5
```

a level 10 bot can challenge only players within level 5-15. It cannot challenge a level 80 player. Bot-vs-bot duel pairing has the same idea through the separate `BotVsBotMaxLevelDifference` setting.

Duel participants are reserved from the random-playerbot teleport cycle for the duration of the
activity. While waiting, each bot is anchored near its assigned duel position; active duel and combat
movement is left untouched. `GuardIntervalMs` controls how frequently the waiting-position guard runs.

## Database-driven zones

`pvp_life_zone` stores the locations and behaviour of each activity.

Every database zone can also be controlled from the config with:

```ini
PvPLife.Zone.<zone name>.Enable = 1
PvPLife.Zone.<zone name>.MinPopulation = 10
PvPLife.Zone.<zone name>.MaxPopulation = 60
```

`MinPopulation` and `MaxPopulation` are the total bot population of the hotspot, with both sides
combined. For every activity the module randomly chooses a total inside this range, then divides it
between attackers and defenders using the side ratio stored in the database. The database counts are
also used as backward-compatible defaults when the population keys are omitted. The configured total
is capped at two times `PvPLife.Bots.MaxPerSide`.

The database `enabled` field and the config switch must both be enabled for automatic activity. A
manual `.pvplife start <name>` command can still force-start a disabled zone for testing. New custom
database zones use the same key pattern after a config reload. Reloading the config changes newly
started activities; an activity already in progress keeps its current participants until it ends.

Activity types:

- `0` = normal World PvP skirmish
- `1` = Duel Zone
- `2` = For the Horde
- `3` = For the Alliance

Team values:

- `0` = Alliance
- `1` = Horde
- `2` = Any

The SQL includes starter locations for:

- Stormwind duel area
- Orgrimmar duel area
- Stranglethorn / Nesingwary (using the custom `stvpvp` teleport)
- Gurubashi Arena
- Gadgetzan / Tanaris
- Dark Portal (Azeroth)
- Dark Portal (Outland)
- Shattrath outskirts (using the custom `shattrathpvp` teleport)
- K3 / Storm Peaks (using the custom `k3pvp` teleport)
- Wintergrasp (using the custom `wintergrasppvp` teleport)
- Horde roaming near Goldshire
- Alliance roaming near Durotar
- optional Orgrimmar zeppelin hotspot (disabled until tuned)
- For the Horde -> Stormwind
- For the Alliance -> Orgrimmar

**The seed coordinates are starting points.** Use the GM position commands below to tune exact rally/target spots on your own server/navmesh.

## GM commands

Root command:

`.pvplife`

Useful commands:

```text
.pvplife status
.pvplife start <zoneName>
.pvplife stop all
.pvplife stop <eventId>

.pvplife zone list
.pvplife zone create <name> <skirmish|duel|forthehorde|forthealliance> <alliance|horde|any> <alliance|horde|any> <minLevel> <maxLevel>
.pvplife zone rally <name>
.pvplife zone target <name>
.pvplife zone counts <name> <attackersMin> <attackersMax> <defendersMin> <defendersMax>
.pvplife zone duration <name> <minMinutes> <maxMinutes>
.pvplife zone weight <name> <value>
.pvplife zone cooldown <name> <seconds>
.pvplife zone challenge <name> <0|1>
.pvplife zone chat <name> <0|1>
.pvplife zone enable <name> <0|1>
```

`zone rally` and `zone target` save the executing GM's current map and position, making coordinate tuning quick in-game.

An explicit config Enable or population value takes precedence over corresponding database settings.
Custom zones without an Enable config key continue to follow their live database flag. Reload the
config with `.reload config`; per-zone changes affect subsequent events. The master switch
`PvPLife.Enable = 0` also stops current module activities.

## Behaviour notes

### Duel zones

The duel system is meant to look like a populated PvP-server duel area, not a duel factory. Bots arrive with staggered timing and only a limited number of pairs are scheduled at once. Real-player challenges additionally require:

- same faction as the duel-zone bot,
- the player to be nearby,
- both characters to be alive and free to duel,
- level difference within the configured limit,
- player and bot challenge cooldowns to be clear.

### For the Horde / For the Alliance

These are deliberately part of the normal world-PvP framework, with faction-specific attackers and defenders rather than a separate minigame. Server announcements are off by default; optional defender yell/world-chat messages provide the visible clue that something is happening.

### Shared bot reservations

The included PvP component uses City Life's shared in-memory bot reservation registry, so PvP,
city residents and questers never select the same bot. The integration uses only public module
code; no compatibility patch is required. Existing PvP and City population settings remain separate.

## Test priorities

For the first live-server test, verify in this order:

1. module loads and WORLD tables are detected;
2. `.pvplife status` works;
3. force-start `StormwindDuel` / `OrgrimmarDuel`;
4. verify bot-vs-bot duel requests;
5. verify real-player challenge level check and cooldown;
6. test normal hotspots, including the new `Wintergrasp` zone;
7. force-start `ForTheHorde_Stormwind` and `ForTheAlliance_Orgrimmar`;
8. tune rally/target coordinates and bot counts from actual server behaviour.

## Status

Active inside City Life, with configurable world PvP and persistent duel zones.
