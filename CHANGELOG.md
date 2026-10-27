# Changelog

## 2026-10-25 - Human behavior update

- Add independently configurable, infrequent inspect, weapon sheath and owned cosmetic-companion
  behavior near real players, with bounded checks, per-bot cooldowns and lifecycle cleanup.
- Limit inspect/sheath to ordinary City Life residents and questers; companions and optional sitting
  require idle ordinary residents. Exclude Custom, Fishing and PvP Life participants and grouped or
  human-controlled bots. Disable every feature in combat, BGs, arenas, dungeons and raids.
- Preserve navigation, movement generators, MMaps and combat targets. Keep physical random jump and
  strafe unavailable even when configured on, with a warning instead of an unsafe fallback.
- Protect charged temporary weapon enchants and Hunter/Warlock combat pets. Summon only known, owned
  cosmetic MINIPET spells; do not grant pets or replace existing companions.
- Keep sitting opt-in because actual sit state can affect the first incoming melee hit; let the bot
  AI, resting and movement take priority. Keep companion summoning opt-in pending in-game verification.
  Document settings and manual verification. No SQL changes.

## 2026-10-21 — Restore the previous Legendary equipment system

- Restore the pre-transmog Legendary implementation: enchanted/gemmed BiS profiles with actual
  iconic legendary weapon overrides and the Swift Spectral Tiger mount appearance.
- Remove the experimental T6 transmog integration and its provider patch.
- Keep The GOATS guild, varied HK/achievement history, fishing, beggars and every other Life feature.
- Retain existing characters and equipment-recovery records; no database rollback is needed.

## 2026-10-21 — Major update: many fixes and new features

City Life now includes PvP Life, Fishing Life and the expanded Custom city activities in one module.
This source update includes many fixes and a single organized configuration. Reconfigure CMake,
rebuild and restart after updating; move the former standalone PvP Life folder outside `modules`.

### Movement, recovery and compatibility fixes

- Fix interrupted city relocation, repeated recovery teleports and unsafe straight-line movement.
- Validate ground height, vertical layers and navmesh paths before choosing movement destinations.
- Correct inventory transaction handling and preserve journal recovery across logout/restart.
- Coordinate reservations so city, quest, PvP, Custom and Fishing activities do not borrow the same bot.
- Preserve existing named SQL records and personal values when applying additive data/config upgrades.
- Correct Stormwind fishing clearance and align the Legendary group with its updated plaza anchor.

### New and expanded features

- Let beggars roam a bounded plaza area, pausing to beg or dance, with separate movement controls.
- Add rare timed beggar trade requests with shared per-player cooldowns and automatic cancellation.

- Vary Legendary HK and achievement-point targets by persistent character identity, with configurable ranges.
- Move the original Stormwind fishing anchor sideways so the bot leaves the trainer clear.

- Give Fishing Life its own config section and `FishingLife.*` settings, independent of Custom.
- Expand fishing to 20 waterfront locations with 46 default slots, separate populations and levels.
- Add separate fishing provisioning settings/quota while retaining existing registered identities.
- Validate water access before fishing placement, enforce spacing, and pause Wintergrasp during battles.
- Add an additive fishing-location SQL update and migrate former Custom fishing config keys.

- Add a default-on Custom city-life section with separate switches for six roles/behaviors.
- Keep legendary characters recognizable through persistent per-city/per-slot identities by default.
- Add per-city populations for legendary AFK characters, true level-1 beggars, buyer advertisements,
  practice-dummy users; reactions and conversations reuse existing residents.
- Add rerunnable WORLD locations/chat data, CHARACTERS equipment recovery and managed-bot storage.
- Provision missing Custom participants at allowed levels on module-owned accounts, with a bounded,
  reusable pool and gradual creation/login; never lower an existing character to fill a role.
- Keep Custom roles within the shared 300-bot cap; document the 16 default Custom requests and updated
  online-bot planning examples.

- Document online-bot budgeting, the default 440-bot Life planning peak, and eligible free bots
  for dungeons/BGs, including faction, level-bracket and role constraints for leveling populations.
- Consolidate all 206 existing settings into one grouped City Life configuration, preserving defaults.
- Add 14 optional towns, 17 faction quest hubs and seven PvP hotspots through rerunnable additive SQL.
- Keep the original STV/Rebel Camp, Farshire/Warsong Hold and town populations unchanged.
- Make explicit config Enable values authoritative while retaining live DB flags as custom-hub defaults.
- Add a safe Python utility for migrating personal values from the former two config files.

- Include PvP Life inside City Life, with its existing world-PvP hotspots, duel zones, real-player
  challenges and faction campaigns.
- Preserve all `CityLife.*` and `PvPLife.*` settings, `.citylife` and `.pvplife` commands, SQL
  filenames and database table names. Existing data needs no migration for this merge.
- PvP retains its enabled default and can be disabled independently
  with `PvPLife.Enable = 0`, without requiring its WORLD tables while disabled.
- Keep separate behavior managers and population settings, coordinated through shared bot
  reservations.
- Move the PvP implementation into this module and retire the standalone PvP source implementation.
- Add combined installation/update guidance and preserve the full PvP reference and earlier
  changelog under `docs/`.

### Earlier movement and questing work included in this update

- Fixed relocation being mistaken for leash escape and interrupted by recovery teleports (issue #1).
- Validate randomized destination heights and reject unsafe straight-line/pathfinding fallbacks.
- Add `.citylife races [hub|all]` to compare resident and available-candidate race distributions.
- Added city populations for Booty Bay, Darkshire, Rebel Camp, Valiance Keep and Warsong Hold.
- Added opt-in real Playerbots questing hubs around Rebel Camp, Farshire and Warsong Hold.
- Added per-hub quest-zone, local-radius, idle-timeout and level settings, with scheduled rotation.
- Keep questers separate from city stay, leash, social relocation and emote behavior.
- Added an additive manual WORLD SQL update for existing installations; no live database changes
  are applied by the module configuration.
- Documented the shared 300-bot cap and the 340-bot total requested by all seeded hubs with questing
  enabled at full time scaling.

## 0.2.0 - 2026-09-12

- Added separate Alliance and Horde Wintergrasp hubs with 40 bots per faction.
- Added per-hub time-scaling control so Wintergrasp keeps its full population all day.
- Added configurable per-hub resident rotation, with a three-hour Wintergrasp duration.

## 0.1.0 - 2026-09-07

- Initial City Life implementation.
- Added configurable visual populations for fourteen seeded social hubs.
- Added weighted AH, bank, mailbox, flight-master and square hotspots.
- Added time-of-day scaling, gradual population changes, roaming and social emotes.
- Added faction/level safety filters and a global population cap.
- Protect Wintergrasp's PvP population from City Life selection.
- Fill enabled hubs proportionally so lower-priority cities no longer remain empty during startup.
- Added configurable, stationary AH and bank crowds for Stormwind and Orgrimmar.
- Use shared reservations, rather than transient duel/stay strategies, to detect other Life activities.
- Added in-game hub and hotspot administration commands.
- Integrated an in-memory bot reservation registry with current PvP Life versions; no manual patch
  or private Playerbots API is required.
