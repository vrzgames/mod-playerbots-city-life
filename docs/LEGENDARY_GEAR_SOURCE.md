# Legendary gear data credits

The bundled Phase 4 class/spec/race gear profiles were extracted from the local
`mod-npc-talent-template` module's `data/sql/db-characters/base/43_wotlk_pve_p4_bis.sql`.
Source repository: [AzerothCore mod-npc-talent-template](https://github.com/azerothcore/mod-npc-talent-template).
Original data commit: `fefce753630fe2de19c92a8bfd12ca536b7ddff6`.
Snapshot revision: `d536079191e6ecabb18273b3e46f7583ad19e355`.
Source author: **teehtheg** (`david.tschirky@gmail.com`).

The source data includes item entries, equipment slots, race masks, permanent enchantments,
socket enchantments and socket bonuses. Original upstream authors retain their copyright;
the City Life integration and separate cosmetic profile mapping are by iCore under GPL-2.0-or-later.

The selected profiles are bundled with City Life. No calls into the Template NPC singleton and
no additional runtime table imports are required. The Template NPC module and its source SQL
are not modified by this integration.

Each generated temporary item is checked against the running core's item/enchantment/DBC data.
The default TemplateGear profiles retain their actual Phase 4 weapons and compatible bonuses;
the older legendary weapons are separate appearances, not weaker replacement items. Equipment
still obeys class, weapon-skill and equipment restrictions. Borrowed original items are never
enchanted, gemmed or given temporary skins by this feature.

T6 armor and legendary weapon appearance entries are bundled separately in
`src/CityLifeLegendarySkins.h`, verified against local WORLD item data and `ItemSet.dbc`.
T6 includes the eight set slots, including the Sunwell belt, boots and bracers. Death Knights
retain their BiS armor appearance because no T6 set exists for that class.
The Black Qiraji mount uses creature display `15676`, verified from legendary item `21176`,
spell `26656` and creature `15711`. See [Legendary skins](LEGENDARY_SKINS.md) for requirements.
