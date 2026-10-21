# Custom city life

The Custom section adds six independently switchable features to City Life. All are enabled in
the distributed template. They require `CityLife.Enable = 1` and `CityLife.Custom.Enable = 1`;
the PvP switch does not control them. [Fishing Life](FISHING_LIFE.md) has its own
section and switch, independent of `CityLife.Custom.Enable`.

## Installation and updates

Apply `data/sql/manual/world_city_life_custom.sql` to WORLD after `world_city_life.sql`, and apply
both `data/sql/manual/characters_city_life_custom.sql` and
`data/sql/manual/characters_city_life_custom_bots.sql` to CHARACTERS. Use the matching updated binary
and the sole `mod_playerbots_city_life.conf` file. The additive seeds preserve existing named
records and Custom chat IDs when rerun. Back up your configuration and databases before updating.

The historical WORLD file adds 13 role hubs, including the three original fishing hubs,
their anchors and ambient chat data. Those fishing hubs now belong to Fishing Life; their
internal database names are preserved to retain existing characters and recovery records. It does not
add creatures or player characters at import time. The CHARACTERS files provide equipment
recovery and a managed-character registry; importing them does not modify existing inventories.
Missing Custom data disables the affected Custom functions without preventing normal City Life or PvP from operating.

The Legendary and beggar upgrades add C++ source files. Run CMake Configure/Generate once before
rebuilding, so the persona, beggar-trade and optional transmog bridge implementations are collected.
For T6/legendary skins, apply the [included mod-transmog patch](TRANSMOG_INTEGRATION.md) and build both modules
statically. Existing Custom tables need no additional SQL import for these behaviors.

`world_city_life_legendary_stormwind.sql` moves an older default Stormwind Legendary anchor
closer to the plaza tree and aligns the group with the captured GM facing. Customized positions
are preserved. This location-only update needs `.citylife reload` or a restart, without a rebuild.

## Switches and populations

| Feature | Stormwind | Orgrimmar | Dalaran | Extra role slots |
| --- | --- | --- | --- | --- |
| Legendary AFK character | 3 | 3 | 3 | 9 |
| Level-1 beggar | 1 | 1 | — | 2 |
| Buyer advertisement | 1 | 1 | 1 | 3 |
| Training-dummy practice | 1 | 1 | — | 2 |
| Player emote reactions | Existing residents | Existing residents | Existing residents | 0 |
| Conversations | Existing residents | Existing residents | Existing residents | 0 |

Every feature has `CityLife.Custom.<Feature>.Enable`. Set the master switch to `0` to stop the
whole Custom component, or disable individual features while keeping the others active.
Use `Population = 0` to disable one role in one city. For example:

```ini
CityLife.Custom.Legendary.Enable = 1
CityLife.Custom.Legendary.Stormwind.Population = 3
CityLife.Custom.Legendary.Orgrimmar.Population = 1
CityLife.Custom.Legendary.Dalaran.Population = 0
```

The feature names are `Legendary`, `Beggar`, `Buyer`, `TrainingDummy`, `PlayerReactions`
and `Conversations`. Only the first four need separate population settings. Reaction and
conversation settings control chance, cooldown, range and timing instead of borrowing extra bots.

Dedicated roles share `CityLife.Population.MaxTotal` with normal residents and questers. Their
bundled priority is 350 so their small populations receive room inside a full cap. At the default
300 cap, filling Custom roles can reduce lower-priority normal residents. Increasing a role count
increases requested slots and does not bypass this cap. CreateMissing can provision missing
Custom participants separately from the normal Playerbots online population.

`CityLife.Custom.TimeScaling = 0` keeps requested Custom populations constant throughout the day.
Set it to `1` to apply the normal city time multipliers. `CityLife.Custom.ResidenceSeconds = 0`
keeps an assignment until it becomes unavailable, is disabled or its target shrinks; a positive
duration requests rotation. Participants are selected from eligible online playerbots first;
missing Custom participants can be provisioned through the managed pool below.

## Creating missing participants

```ini
CityLife.Custom.CreateMissing.Enable = 1
CityLife.Custom.CreateMissing.MaxBots = 30
CityLife.Custom.CreateMissing.CreateIntervalSeconds = 30
CityLife.Custom.CreateMissing.AccountPrefix = "citylife"
```

When a Custom role has no eligible candidate, the module can create a character at an allowed
level and log it in on a separate module-owned account. Correctly leveled, matching-faction
managed characters are reused when available. Existing characters are never lowered to another
level. Fishing Life has its own provisioning toggle, quota, interval and account prefix.
Existing fishing identities count only against that separate fishing quota. Only records registered to this
facility are treated as its managed accounts/characters.

Creation/login is gradual, subject to the configured maximum of 30 persistent managed slots,
including pending work. A quota or account/character/login failure can leave requested role slots
unfilled; logs explain these cases. Disabling provisioning stops creation of new characters;
registered identities can still be logged in and reused. It does not turn the 30-slot quota into a guaranteed
number of active bots or increase any role's configured population.

These managed sessions are additional to `Playerbots.MinRandomBots`/`MaxRandomBots`. Include
actually online managed participants in total online-bot and server-capacity budgets. They are
still counted inside the shared City Life reservation cap while assigned. Normal city, quest and
PvP population settings retain their existing borrowing behavior.

## Character roles

### Legendary AFK characters

By default, three level-80 bots stand at each city's plaza anchors in complete Phase 4 BiS outfits,
with their real BiS weapons, enchants and gems. Temporary T6 armor and legendary weapon appearances
sit over those items when the matching `mod-transmog` is available. Their default mount appearance
is a stable per-character choice between Spectral Tiger and Black Qiraji Battle Tank.
The bundled class/spec/race profiles come from
`mod-npc-talent-template`; that module is not needed at runtime. See
[gear data credits](LEGENDARY_GEAR_SOURCE.md) and [Legendary skins](LEGENDARY_SKINS.md).
Mount appearance, item profiles, skins and enhancement switches are configurable in the Custom section.

`CityLife.Custom.Legendary.FixedIdentity = 1` keeps the same registered module-managed character
for each city and population slot, including after restart. This makes each legendary character
recognizable by name. Fixed slots use their own managed identity even when another normal random
bot is available. New slots require CreateMissing and room in its quota; existing registered
identities can be reused and logged in when provisioning is disabled. Increasing a city population adds separate
identities for the additional slots. Each keeps a deterministic position around its plaza anchor.
Turning FixedIdentity off permits normal eligible candidates.

`CityLife.Custom.Legendary.Classes = "Warrior,Rogue,Paladin"` controls newly created identities.
The least represented eligible class in that city is preferred, with ties following list order.
Names or numeric class IDs are accepted. Existing characters retain their original class and name;
the list does not delete or replace a registered character. Increase the city population if you want
three new classes alongside an earlier character of another class.

Owned Legendary characters share the guild **The GOATS**. By default, each character gets a stable,
individual target of **11,000-25,000 lifetime honorable kills** and **5,100-6,000 achievement points**.
Restarting does not reroll the targets or keep adding random amounts. These are minimum targets;
existing higher progress stays, and achievement totals can overshoot by an achievement's point value.
This means real achievement records, not thousands of separate achievements or spendable honor.
Most default history comes from dungeons
and raids, with GUID-stable variation in the selected records. Realm-first, hidden/statistic entries,
and item/mail reward achievements are excluded. Initial history is populated without mass achievement
announcements. If the eligible pool cannot reach the configured point target, the available history is retained.

```ini
CityLife.Custom.Legendary.Honor.Enable = 1
CityLife.Custom.Legendary.Honor.Randomize = 1
CityLife.Custom.Legendary.HonorableKills = 10000
CityLife.Custom.Legendary.HonorableKills.Min = 11000
CityLife.Custom.Legendary.HonorableKills.Max = 25000
CityLife.Custom.Legendary.Achievements.Enable = 1
CityLife.Custom.Legendary.Achievements.Randomize = 1
CityLife.Custom.Legendary.AchievementPoints = 5000
CityLife.Custom.Legendary.AchievementPoints.Min = 5100
CityLife.Custom.Legendary.AchievementPoints.Max = 6000
CityLife.Custom.Legendary.AchievementIds = "default"
CityLife.Custom.Legendary.Guild.Enable = 1
CityLife.Custom.Legendary.Guild.Name = "The GOATS"
```

These persistent additions apply only to verified module-owned Legendary characters. Borrowed random bots
and real-player characters do not receive them. Raising configured values can add history; lowering or disabling
the settings does not delete already saved kills, achievements, reputation or guild membership. An existing
guild belonging to other characters is left untouched, and another guild is never forcibly replaced.
All cities/factions can share the module's guild through the core's membership API. No custom tabard,
rank layout or fixed companion pet is added.

Set `Honor.Randomize = 0` to use the fixed `HonorableKills` value, and
`Achievements.Randomize = 0` to use the fixed `AchievementPoints` value. The Min/Max settings
control variation independently for HK and points. Existing achievements and kills are never removed.

`TemplateGear.ExaltedReputation = 1` grants owned Legendary characters the Ashen Verdict reputation needed
for the bundled BiS rings. `TemplateGear.Enable = 0` keeps the earlier Tier 10/legendary outfits;
`Enchants.Enable` and `Gems.Enable` independently control generated item bonuses. All these keys use
the `CityLife.Custom.Legendary.` prefix. Explicit custom class `Outfits` lists remain supported.

The role equips actual temporary items, so inspection shows its outfit. Original items are kept
in the character's general inventory and restored when the assignment ends. Enough free general
bag slots and the normal item/class/level/weapon requirements are necessary. Characters currently
mounted, shapeshifted or otherwise busy are not borrowed for an equipment change.

The equipment journal in CHARACTERS supports recovery after logout/restart; retain it until all
pending outfits have been restored. Temporary items are bound and are not a reward for players.
Assigned Custom characters reject group acceptance, BG/LFG recruitment, item transfers, selling and mail.
The beggar can initiate its own limited flavor trade request when enabled; other trades stay blocked.
Their normal autonomous AI is paused while the module performs their role; normal city residents
retain their existing defensive AI. An outside attack releases the Custom assignment and restores
the character before normal AI resumes.

```ini
CityLife.Custom.Legendary.MountDisplayId = 21974
CityLife.Custom.Legendary.Mount.Randomize = 1
CityLife.Custom.Legendary.Mount.DisplayIds = "21974,15676"
CityLife.Custom.Legendary.Skins.Enable = 1
CityLife.Custom.Legendary.Skins.Armor.Enable = 1
CityLife.Custom.Legendary.Skins.Weapons.Enable = 1
CityLife.Custom.Legendary.Skins.HideOffhand = 1
CityLife.Custom.Legendary.Warrior.Outfits = "default"
```

`MountDisplayId` is a creature display, not a spell/item ID. It displays the mount without teaching
it permanently. `Mount.Randomize = 1` selects from `Mount.DisplayIds` by fixed character identity;
`0` uses `MountDisplayId`. Empty or invalid lists fall back to that fixed value, and
`MountDisplayId = 0` disables all Legendary mount appearances. Each class has its own `Outfits` option.
`"default"` uses the bundled profiles;
an empty string disables that class. A custom value is a comma-separated list of item entries;
use semicolons between alternative outfits. Slots are inferred from the items. Profiles must
satisfy ordinary item/class restrictions, including dual wielding and weapon skills.

With `TemplateGear.Enable = 1`, default profiles use the complete bundled Phase 4 BiS gear.
The following older physical Tier 10/legendary item lists apply only with `TemplateGear.Enable = 0`;
explicit custom `Outfits` values also remain physical item lists. Each line is one alternative:

- `Warrior`:

```text
51227,51229,51225,51226,51228,32837,32838
51227,51229,51225,51226,51228,49623
```

- `Paladin`:

```text
51277,51279,51275,51276,51278,49623
51272,51273,51274,51270,51271,46017
```

- `Hunter`:

```text
51286,51288,51289,51285,51287,34334
51153,51151,51150,51154,51152,34334
```

- `Rogue`:

```text
51252,51254,51250,51251,51253,32837,32838
51187,51185,51189,51188,51186,32837,32838
```

- `Priest`:

```text
51261,51264,51263,51260,51262,46017
51256,51257,51259,51255,51258,22631
```

- `DeathKnight`:

```text
51312,51314,51310,51311,51313,49623
51127,51125,51129,51128,51126,49623
```

- `Shaman`:

```text
51247,51245,51249,51248,51246,46017
51192,51194,51190,51191,51193,46017
```

- `Mage`:

```text
51281,51284,51283,51280,51282,22589
51158,51155,51156,51159,51157,22589
```

- `Warlock`:

```text
51231,51234,51233,51230,51232,22630
51208,51205,51206,51209,51207,22630
```

- `Druid`:

```text
51290,51292,51294,51291,51293,46017
51302,51304,51300,51301,51303,22632
```

The chosen usable profile is stable for that character GUID, so a fixed legendary identity
keeps its appearance across restarts. Editing its profile options can change that appearance.
Only listed equipment slots are replaced; other gear remains the character's own equipment.

### Level-1 beggars

By default, the beggar is a real level-1 playerbot wearing underwear, with occasional begging/dancing emotes
and varied local chat. Idle emotes occur every 45–120 seconds by default, with a 25% chance of an
eight-second dance; the dance duration is bounded and separate from the chat timer.
The default range requires a real level-1 candidate. Set both `MinLevel` and `MaxLevel` to the
same other value to request another exact level; existing characters are not lowered to match.
If the normal online pool has
none, CreateMissing can provision a level-1 character of the appropriate faction. Existing level-80
characters are never lowered to fill the role. If provisioning is disabled or its quota is full,
provide eligible online level-1 bots or reduce the requested population. The role protects its
assignment from normal random-bot randomization while held.

Beggars roam within 10 yards of their original plaza spot by default, using checked ground and
navigable paths. They run by default (`Wander.Walk = 0`), pause 10-25 seconds between trips, and
stop to beg or dance. The radius stays centered on the original spot; it does not move with the bot.
Combat, trade, transit and ongoing emotes pause roaming. Disabling Wander keeps the original idle behavior.

```ini
CityLife.Custom.Beggar.Wander.Enable = 1
CityLife.Custom.Beggar.Wander.Radius = 10
CityLife.Custom.Beggar.Wander.MinSeconds = 10
CityLife.Custom.Beggar.Wander.MaxSeconds = 25
CityLife.Custom.Beggar.Wander.Walk = 0
CityLife.Custom.Beggar.Trade.Enable = 1
CityLife.Custom.Beggar.Trade.MinSeconds = 180
CityLife.Custom.Beggar.Trade.MaxSeconds = 420
CityLife.Custom.Beggar.Trade.ChancePct = 25
CityLife.Custom.Beggar.Trade.Range = 8
CityLife.Custom.Beggar.Trade.DurationSeconds = 15
CityLife.Custom.Beggar.Trade.PlayerCooldownSeconds = 600
```

At each random 180-420 second interval, a 25% roll may request a normal trade from an available
nearby real player. Requests are limited to the same faction, map and phase with line of sight;
ignore lists, AFK/DND status and the core's trade restrictions are respected. A player has a shared
10-minute cooldown across all beggars. The normal client trade prompt can be declined; the bot
cancels its own request/window after 15 seconds, or when released. The beggar does not automatically
accept or complete transfers of gold or items. Trade needs the core's configured trade level;
the normal default allows level 1. Roaming and trade use the existing beggar population without
reserving extra bots, and each behavior has its own switch.

### Buyers

Buyers advertise level 70-80 green equipment for 10 gold. These messages are ambient flavor;
they do not implement item acceptance, gold payment or automatic trade. Text is stored in
`city_life_custom_chat`, so it can be rewritten without changing C++.

### Training-dummy practice

Practice participants use nearby existing training dummies, alternating a configurable practice
period and rest period. The bundled level range is 80–80 to match these high-level dummies.
They do not attack an arbitrary player, guard or creature as a substitute.
Stormwind and Orgrimmar have bundled sites. The local base WORLD data has no corresponding city
dummy in Dalaran, so no Dalaran practice site is included.

## Local chat and interactions

Beggar and buyer messages have separate minimum/maximum intervals and yell chances.
The default interval is 180–420 seconds, with a 20% yell chance for beggars and ordinary say for
buyers. Fishing messages use the separate `FishingLife.*` settings. Role messages require a nearby real player within `CityLife.Custom.Chat.Range`.
Multiple characters use independent schedules instead of announcing every line simultaneously.

Target an eligible city resident with `/wave`, `/bow` or `/dance` for a possible matching response.
Defaults are a 70% chance, a 30-second per-player/per-bot cooldown and a 20-yard range. The bot must
be available in the same map/phase; combat, transit or other activity can prevent a response.

Conversations consist of a question, another resident's reply, and thanks from the first speaker.
The default start interval is 180–420 seconds; individual replies wait 2–5 seconds. Participants
must be close to one another, with a real player nearby. Unavailable or moved participants cancel
the remaining exchange. At most two exchanges run concurrently by default.

The table `city_life_custom_chat` stores role groups `beggar`, `buyer`, `fishing`, and `conversation`.
Conversation rows share a `conversation_id` and have unique steps `0`, `1`, and `2`; incomplete
groups are skipped. `speaker_team` is 0 Alliance, 1 Horde or 2 either. Conversation `channel`
is 0 say or 1 yell; role-message channels follow the role's configured `YellChancePct`.
Chat data is cached on reload, rather than queried for each message. Reload the module data after
editing it, and reload the server config after changing `.conf` values.

## Online-bot budget

The default Custom roles request 16 slots. Fishing Life separately requests 46 slots, for 62
dedicated participants in the distributed configuration. With the normal hub settings and both
components enabled, morning/day/evening/night targets are 286/300/300/205 after the shared cap.
Actual filled slots can be lower when eligible bots or safe spots are unavailable. The city cap
remains 300, so the 440-bot automatic Life planning peak including PvP remains unchanged.

With all city/quest hubs enabled at full time scaling, 532 normal slots plus 16 Custom and 46
Fishing slots request 594. Raising the shared cap to at least 594 and retaining the 140-bot PvP
allowance yields 734 Life slots, leaving 1,266 nominally free out of 2,000. Count all actually
online managed sessions in that total. Keep free bots by faction, BG bracket and dungeon role;
see the main README. These are population budgets, not performance measurements.
