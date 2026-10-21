# Legendary gear data credits

The bundled Phase 4 class/spec/race gear profiles were extracted from the local
`mod-npc-talent-template` module's `data/sql/db-characters/base/43_wotlk_pve_p4_bis.sql`.
Source repository: [AzerothCore mod-npc-talent-template](https://github.com/azerothcore/mod-npc-talent-template).
Original data commit: `fefce753630fe2de19c92a8bfd12ca536b7ddff6`.
Snapshot revision: `d536079191e6ecabb18273b3e46f7583ad19e355`.
Source author: **teehtheg** (`david.tschirky@gmail.com`).

The source data includes item entries, equipment slots, race masks, permanent enchantments,
socket enchantments and socket bonuses. Original upstream authors retain their copyright;
the City Life integration and iconic weapon overrides are by iCore under GPL-2.0-or-later.

The selected profiles are bundled with City Life. No calls into the Template NPC singleton and
no additional runtime table imports are required. The Template NPC module and its source SQL
are not modified by this integration.

Each generated temporary item is checked against the running core's item/enchantment/DBC data.
Legendary weapon overrides still obey ordinary class, weapon-skill and equipment restrictions.
An old legendary weapon can have fewer sockets than the Phase 4 weapon it replaces, so only
compatible bonuses are applied; borrowed original items are never enchanted or gemmed by this feature.
