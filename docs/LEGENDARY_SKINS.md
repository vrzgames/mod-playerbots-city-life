# Legendary skins

Legendary residents wear the bundled level-80 Phase 4 BiS items, including their actual weapons,
enchants and gems. The optional mod-transmog integration changes their visible armor to T6 and
their weapon appearance to class/profile-specific legendaries. Real item entries and statistics
stay unchanged. Fixed character identities, The GOATS guild and veteran history keep working.

## Installation and activation

Install City Life and apply the [included mod-transmog patch](TRANSMOG_INTEGRATION.md).
Both modules must be enabled and built
statically in AzerothCore's common `modules` library. Run CMake Configure/Generate, rebuild and
restart worldserver. No new SQL import is needed for these cosmetics.

Missing, older, disabled or dynamically linked transmog builds leave the true BiS equipment visible.
The same applies when transmog's global switch is off. No physical older weapon replaces a BiS
weapon as a fallback. Normal players keep their existing transmog restrictions and collections.

```ini
CityLife.Custom.Legendary.TemplateGear.Enable = 1
CityLife.Custom.Legendary.Skins.Enable = 1
CityLife.Custom.Legendary.Skins.Armor.Enable = 1
CityLife.Custom.Legendary.Skins.Weapons.Enable = 1
CityLife.Custom.Legendary.Skins.HideOffhand = 1
CityLife.Custom.Legendary.MountDisplayId = 21974
CityLife.Custom.Legendary.Mount.Randomize = 1
CityLife.Custom.Legendary.Mount.DisplayIds = "21974,15676"
```

Armor and weapon skins can be switched independently. `Skins.Enable = 0` shows the real BiS gear.
For a two-handed weapon appearance over a real one-handed weapon, `HideOffhand = 1` hides only the
extra offhand visual; its item and bonuses stay equipped. It does not hide dual-wielded glaives.
Config changes take effect on config reload or restart through the normal City Life lifecycle.

Only the bundled default TemplateGear profiles get these skins. Explicit custom class `Outfits`
remain physical item lists. Setting `TemplateGear.Enable = 0` selects the older physical Tier 10
and legendary outfits. See [Custom city life](CUSTOM_LIFE.md) for those item lists and populations.

## Appearances

| Class | Armor appearance | Weapon appearance by profile |
| --- | --- | --- |
| Warrior | Onslaught T6 | Warglaives / Shadowmourne |
| Paladin | Lightbringer T6 | Shadowmourne / Val'anyr |
| Hunter | Gronnstalker T6 | Thori'dal ranged appearance |
| Rogue | Slayer T6 | Warglaives |
| Priest | Absolution T6 | Val'anyr / Atiesh |
| Death Knight | Original BiS armor | Shadowmourne |
| Shaman | Skyshatter T6 | Val'anyr |
| Mage | Tempest T6 | Atiesh |
| Warlock | Malefic T6 | Atiesh |
| Druid | Thunderheart T6 | Val'anyr / Atiesh |

T6 covers head, shoulders, chest, hands, legs, belt, boots and bracers. Other slots retain their
BiS appearance. Death Knights have no T6 set, so their armor stays as it is. Weapon and armor
choices follow the same stable class/spec profile as the real gear. A Warrior Fury profile needs
the character's real dual-wield and Titan's Grip capabilities; otherwise an eligible Arms profile
is used.

Each character gets a stable selection from `Mount.DisplayIds`: by default Swift Spectral Tiger
(`21974`) or Black Qiraji Battle Tank (`15676`). Restarting keeps that character's selection;
changing the list can change it. Display `15676` was verified through legendary item `21176`
(Black Qiraji Resonating Crystal). These are visual mounts and do not permanently teach a mount.
`Mount.Randomize = 0` uses the fixed `MountDisplayId`; an empty or invalid list also uses that
fallback. `MountDisplayId = 0` disables every Legendary mount visual independently of the skins.

## Equipment recovery

Cosmetics are held only in the transmog memory cache for exact temporary item GUIDs already
recorded in City Life's equipment journal. They do not create saved transmog rows, appearance
collection unlocks or extra legendary source items. Original items keep their saved appearances.
Release, logout and recovery remove temporary cosmetics; the existing equipment journal restores
the original inventory. Keep the journal tables while any outfit is awaiting recovery.

Keep a backup of the module, active config and matching databases before updating. To restore a full
snapshot, follow its restore instructions and restore matching databases and files together.
