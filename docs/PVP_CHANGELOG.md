# Changelog

## Included in City Life - 2026-10-21

- Move the PvP Life implementation into `mod-playerbots-city-life`.
- Preserve existing PvP settings, commands, configuration filename, SQL filename and database tables.
- Keep `PvPLife.Enable = 1` as the default; `0` disables PvP independently of `CityLife.Enable`.
- Retain existing behavior and population settings with shared reservations for city and PvP bots.
- Keep the earlier standalone release history below. See the [City Life changelog](../CHANGELOG.md)
  for changes to the combined module.

## Current

- Updated the K3, STV/Nesingwary, and Shattrath outskirts seed coordinates from
  the server's `k3pvp`, `stvpvp`, and `shattrathpvp` teleport points.
- Added a Wintergrasp PvP hotspot using the saved `wintergrasppvp` teleport coordinates.
- Added shared in-memory bot reservations for conflict-free City Life compatibility without private
  Playerbots API calls.
- Added configurable total minimum and maximum bot population for every hotspot.
- Added per-zone configuration switches while keeping GM force-start available for testing.
- Added partial-team fallback and clearer selection diagnostics for smaller bot populations.
- Updated the Stormwind and Orgrimmar duel-zone seed locations.
- Reserved duel participants from random teleportation and anchored idle bots near their assigned duel positions.
- Added AzerothCore Playerbot and `mod-playerbots` dependency metadata and links.

## v0.1.1-test

- Fixed compatibility with the current mod-playerbots `ActivityType` enum by fully qualifying PvP Life activity values.
- Updated real-player checks for the current Playerbots API (`IsRealPlayer(Player*)` and `HasGameClientMaster()`).
- Added explicit game-time conversion to avoid MSVC narrowing errors in queued chat events.

## v0.1.0-test

- Initial PvP Life module.
- Persistent normal World PvP hotspots.
- Persistent Stormwind/Orgrimmar duel-zone framework.
- Bot-vs-bot duel scheduling with repeated pairing, cooldowns, and configurable maximum level difference.
- Bot-to-real-player duel challenges with level check and anti-spam cooldowns.
- For the Horde / For the Alliance faction activity types.
- Optional randomized defender yell/world chat.
- Optional server announcements, disabled by default.
- DB-driven zones and chat lines.
- Staggered arrivals, movement jitter and randomized activity duration/counts.
- Playerbots activity-state protection to avoid taking bots that are not available for general activity.
- GM status/start/stop/zone-edit commands.
