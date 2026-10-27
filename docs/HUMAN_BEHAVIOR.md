# Human behavior

Human behavior adds rare cosmetic/social actions to ordinary City Life bots. Configure it in
section **02** of `mod_playerbots_city_life.conf`, under `CityLife.HumanBehavior.*`.
Both `CityLife.Enable` and `CityLife.HumanBehavior.Enable` must be on. Each feature has an
independent switch, chance and cooldown where supported. No SQL or schema update is required.

## Eligibility and scheduling

Inspect and weapon sheath can run for ordinary City Life residents and City Life questers.
Sitting and companion summoning require an ordinary idle city resident; they never run for
questers. Custom roles, Fishing Life and PvP Life participants are excluded, as are grouped bots
and bots controlled by a real player. The component does not manage the wider random-bot pool.

Actions require an alive, out-of-combat bot in the normal open world and a nearby real-player
observer on the same map and phase. BGs, arenas, dungeons and raids are excluded. Casting,
transport/vehicle use, flight and other incompatible states prevent an action. Mounting,
resting, active movement and assigned tasks take priority over idle-only behavior.

The default scan interval is 10 seconds with at most 32 bots examined per scan, rotating through
the resident list. Cheap idle bookkeeping and owned-state cleanup visit current candidates once
per second. Each bot and feature has its own randomized cooldown. A chance is evaluated
only when that cooldown permits an attempt; it is not a chance per world tick. Short active
posture/companion cleanup is serviced separately. The component does not run pathfinding,
perform database queries or alter AI strategies to add an action.

| General setting | Default | Meaning |
| --- | --- | --- |
| `Enable` | `1` | Component switch; also requires `CityLife.Enable`. |
| `CheckIntervalSeconds` | `10` | Interval between bounded action scans. |
| `MaxBotsPerCheck` | `32` | Maximum bots examined in each scan. |
| `ObserverRange` | `60` | Required real-player observer range, in yards. |
| `IdleSeconds` | `15` | Minimum continuous idle time for idle-only actions. |

All keys below also start with `CityLife.HumanBehavior.`.

## Random jump and strafe

`RandomJump.Enable = 0` and `RandomStrafe.Enable = 0` are reserved safety switches. Both physical
movement features remain unavailable even if set to `1`; the server logs a warning. No jump,
strafe, movement-packet or animation fallback is substituted.

The available jump APIs replace the active spline or movement generator. Sideways displacement
also requires a new path/spline, and raw movement flags affect other movement consumers. These
operations cannot guarantee preservation of City Life's current route and spline ownership.
The component therefore leaves MMaps, movement generators and ongoing navigation unchanged.

## Inspect

Inspect uses the normal server inspect handler for a nearby eligible player or bot, without
changing the bot's selection, combat target or facing. It has no visible inspect animation,
chat or client inspect window. Eligibility includes range and the core's target checks; the
configured range cannot exceed the core's inspect-distance limit.

| Setting | Default |
| --- | --- |
| `Inspect.Enable` | `1` |
| `Inspect.ChancePct` | `15` |
| `Inspect.MinSeconds` | `120` |
| `Inspect.MaxSeconds` | `300` |
| `Inspect.Range` | `20` yards |

## Weapon sheath

Rare weapon draw/sheath changes imitate a player's Z-key behavior. This does not equip or
unequip items, change the combat target, or schedule a forced sheath change during combat.
Charged temporary weapon enchants are guarded so cosmetic draws cannot consume poisons or
other temporary enchantment charges. The bot's normal combat behavior retains control.

| Setting | Default |
| --- | --- |
| `WeaponSheath.Enable` | `1` |
| `WeaponSheath.ChancePct` | `10` |
| `WeaponSheath.MinSeconds` | `120` |
| `WeaponSheath.MaxSeconds` | `300` |

## Sitting - opt-in

`Sit.Enable` defaults to `0`. Sitting uses the real stand-state API and has a gameplay consequence:
the core can turn the first incoming non-miss melee hit against a sitting player into a critical
hit. Waking on combat entry cannot undo a sitting state already cached for that hit. Keep this
feature off when preserving combat outcomes is a strict requirement.

When enabled, only an ordinary resident that has remained idle may sit. It needs a finalized
spline and idle movement generator, with no conflicting task or imminent scheduled movement.
The component never pauses a route or AI action to make time for sitting. The AI may stand the
bot up before the randomized duration expires; its posture wins and sitting is not reasserted.
Food/drink or another owner of the posture also takes priority.

| Setting | Default |
| --- | --- |
| `Sit.Enable` | `0` |
| `Sit.ChancePct` | `10` |
| `Sit.MinSeconds` | `300` |
| `Sit.MaxSeconds` | `900` |
| `Sit.DurationMinSeconds` | `8` |
| `Sit.DurationMaxSeconds` | `20` |

## Cosmetic companion

`Companion.Enable` defaults to `0`, pending in-game verification. An idle ordinary resident may
summon one of its own already-known cosmetic companions. Only an instant, nonchanneled spell with
exactly one nonempty effect, a pure cosmetic `MINIPET` summon, is eligible. Combat-pet categories,
guardian properties and spells requiring another pet's dismissal are rejected. No spells or pets
are granted, and a bot without a suitable known spell simply skips the attempt. An existing
companion is not replaced.
Hunter pets, Warlock demons, guardians and other combat summons are never used or dismissed.

The validated summon is triggered so it does not occupy the bot's normal spell slot or consume
its global cooldown. Spell hooks and creature scripts still run; retain the disabled default until
your server's cosmetic summons have been checked in game.

Cleanup tracks only the companion created by this component and checks its identity before
despawning it. If another action replaces it, cleanup leaves that replacement alone. Summoning
and expiry do not take ownership of the bot's combat-pet state.

| Setting | Default |
| --- | --- |
| `Companion.Enable` | `0` |
| `Companion.ChancePct` | `10` |
| `Companion.MinSeconds` | `300` |
| `Companion.MaxSeconds` | `900` |
| `Companion.DurationMinSeconds` | `60` |
| `Companion.DurationMaxSeconds` | `180` |

## Reload and lifecycle

Use `.reload config` or restart after changing these settings. `.citylife reload` reloads hub
and spot data only. Configuration reload, logout, resident release and shutdown clear owned
temporary behavior state; ordinary City Life movement first releases its cosmetic sit.
Cleanup relinquishes posture or companion ownership when the AI or another action takes over.
Disabling this component stops new attempts without changing the city population.

## Manual in-game verification

These are checks to perform on a server running the updated module, not claims of completed tests.

1. With a real player nearby, observe ordinary residents and enabled questers through several
   cooldowns. Check sheath changes and companions; inspect is server-side and has no animation.
2. Toggle each feature and the master switch with `.reload config`. Confirm the documented scope
   and that owned temporary state is cleared. Move observers out of range and check no new actions start.
3. Follow a resident during relocation and a quester around obstacles. Confirm their route and task
   continue, with no unexpected jump, sideways displacement, stopping or return-position changes.
4. Attack an eligible bot and check the normal defensive AI takes control. Repeat in BG, arena,
   dungeon and raid areas; no new Human Behavior action should start there.
5. Verify Custom characters, fishers, PvP Life bots, grouped bots and human-controlled bots are skipped.
   Check food/drink, casts, mounted states and transports are not interrupted.
6. Check an owned cosmetic companion expires, survives no replacement ownership mistake, and clears
   on release/logout/reload. Verify Hunter and Warlock combat pets and pre-existing companions remain.
7. Check temporary weapon enchant charges before/after several sheath opportunities; they must not
   be consumed by this feature. On a separate opt-in sit check, verify early AI wake and normal movement;
   remember the documented first-hit critical-strike consequence of real sitting.
8. Set jump/strafe switches to `1`, reload, and check the warning and absence of physical fallback motion.
