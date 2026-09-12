# City Life

City Life populates capitals, towns and popular social hubs with configurable AzerothCore
Playerbots. It is a visual population module: residents gather near auction houses, banks,
mailboxes, flight masters and public squares, but the module does not trade or manipulate the
auction house economy.

## Features

- Per-hub population settings, including values such as 50 bots in Stormwind and 5 in Exodar.
- Weighted hotspot distribution instead of stacking every bot at one coordinate.
- Extra stationary residents around Stormwind and Orgrimmar auction houses and banks.
- Gradual arrivals and departures.
- Morning, daytime, evening and nighttime population multipliers.
- Occasional movement between service and social hotspots.
- Sparse, configurable social emotes.
- Faction and level filtering for every hub.
- Global population safety cap with priority-based allocation.
- Runtime GM tools for adding hubs and recording exact hotspot coordinates in game.
- Bundled AzerothCore 3.3.5 seed data for sixteen popular locations.

Bundled hubs:

- Stormwind, Ironforge, Darnassus and Exodar
- Orgrimmar, Undercity, Thunder Bluff and Silvermoon
- Shattrath and Dalaran
- Alliance and Horde Wintergrasp starting camps
- Gadgetzan and Goldshire
- Honor Hold and Thrallmar

## Requirements

- AzerothCore `azerothcore-wotlk` Playerbot branch
- `mod-playerbots`
- A configured pool of online random playerbots

City Life borrows already-online random bots. It does not create characters and does not increase
the number of random bots logged in by `mod-playerbots`. Ensure the Playerbots population is large
enough to cover normal world activity plus the requested City Life population.

Bots already present in Wintergrasp are protected and never borrowed for other City Life hubs.
The bundled faction-specific Wintergrasp hubs maintain their own reserved populations.

## Installation

1. Copy `mod-playerbots-city-life` into the core's `modules` directory.
2. Apply `data/sql/manual/world_city_life.sql` to the WORLD database.
3. Copy `conf/mod_playerbots_city_life.conf.dist` to the generated worldserver configuration area
   in the same way as other AzerothCore module configuration files.
4. Configure the desired population for each hub.
5. Reconfigure and rebuild AzerothCore, then restart worldserver.

The module disables itself and prints a clear server-log error when its two WORLD tables are
missing.

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

`hub population` changes the database default. An explicit `.conf` population value still has
precedence. Use `.citylife reload` after direct database edits; GM commands reload data
automatically.

## Behaviour and safety

Residents receive a non-combat `stay` strategy and have grinding, travel and autonomous RPG
movement disabled while reserved. They can still defend themselves if attacked. City Life never
interrupts combat and never borrows bots that are dead, grouped, in an instance, in a battleground,
in flight, controlled by a real player or already using the Playerbots duel strategy.

The random-playerbot teleport timer is held while a bot is a resident. When released, the bot's
normal strategies and random teleport schedule are restored. By default it is also returned to the
position from which City Life borrowed it.

City Life and current versions of `mod-playerbots-pvp-life` use a shared in-memory bot reservation
registry, so the two modules never borrow the same bot. The registry uses only public module code;
there is no compatibility patch to apply and no terminal command is required. Copy both current
module folders into `modules`, configure them and build AzerothCore normally.

## Scope

City Life intentionally does not:

- create auction listings or buy items;
- simulate banking or mailbox transactions;
- create or log in random-bot characters;
- replace Playerbots combat AI;
- teleport or control real players and player-owned bots.

Its job is focused: make important World of Warcraft locations look inhabited without altering the
server economy.
