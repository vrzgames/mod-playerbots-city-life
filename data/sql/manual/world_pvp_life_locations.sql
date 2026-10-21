-- Optional PvP Life locations for the WORLD database.
-- Apply world_pvp_life.sql first. New locations are disabled in both SQL and the unified configuration.
-- Set PvPLife.Zone.<name>.Enable = 1 in the unified configuration to enable a new location.
-- Additive/idempotent: existing rows with these names, including custom coordinates, are preserved.
-- These are generic skirmishes; no capture, bombing, resource or quest objectives are automated.
-- Coordinates use existing ground NPC/waypoint data. Tune on your server before enabling.

-- Southshore_TarrenMill: Hillsbrad road between the Southshore and Tarren Mill approaches; Zixil ground patrol.
-- Horde rally: waypoint_data.sql path/entry=154240 point=11; distinct ground starts, 141.61 yd apart.
-- Alliance target: waypoint_data.sql path/entry=154240 point=20; distinct ground starts, 141.61 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'Southshore_TarrenMill',0,0,1,0,20,40,0,
  -409.217,-666.302,54.5461,2.333053,-507.007,-563.878,42.3211,5.474646,
  5,10,5,10,10,25,100,900,
  0,0,0
);

-- Halaa: Halaa outer southern/eastern land approach, away from the central guards and lake below.
-- Horde rally: creature.sql guid=59773 entry=17131; distinct ground starts, 129.26 yd apart.
-- Alliance target: creature.sql guid=59849 entry=17131; distinct ground starts, 129.26 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'Halaa',0,0,1,0,64,70,530,
  -1710.96,7719.17,-19.5969,5.848951,-1593.7,7664.79,-15.4585,2.707359,
  5,12,5,12,10,25,100,900,
  0,0,0
);

-- BlackrockMountain: Blackrock Mountain upper approach on the Grark Lorkrub ground escort route; not the lava floor.
-- Horde rally: waypoints.sql path/entry=9520 point=60; distinct ground starts, 115.57 yd apart.
-- Alliance target: waypoints.sql path/entry=9520 point=54; distinct ground starts, 115.57 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'BlackrockMountain',0,0,1,0,50,60,0,
  -7383.85,-1099.22,278.076,2.917874,-7496.54,-1073.58,264.806,6.059467,
  4,8,4,8,10,25,100,900,
  0,0,0
);

-- ArathiHighlands: Open ground between the Refuge Pointe and Hammerfall approaches; existing walking patrol.
-- Horde rally: waypoint_data.sql path/entry=145140 point=208; distinct ground starts, 145.04 yd apart.
-- Alliance target: waypoint_data.sql path/entry=145140 point=200; distinct ground starts, 145.04 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'ArathiHighlands',0,0,1,0,30,45,0,
  -1299.37,-3164.64,35.1117,1.413503,-1276.65,-3021.39,36.0018,4.555095,
  4,8,4,8,10,25,100,900,
  0,0,0
);

-- Ashenvale: Central Ashenvale forest approach between the Astranaar and Splintertree regions; Shadumbra ground path.
-- Horde rally: waypoint_data.sql path/entry=328070 point=1; distinct ground starts, 178.93 yd apart.
-- Alliance target: waypoint_data.sql path/entry=328070 point=12; distinct ground starts, 178.93 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'Ashenvale',0,0,1,0,20,35,1,
  2517.2,-1612.16,144.419,1.317725,2562.0,-1438.93,157.754,4.459318,
  4,8,4,8,10,25,100,900,
  0,0,0
);

-- Grizzly_Blackriver: Blackriver Logging Camp battlefield on the ground occupied by Conquest Hold raiders and Amberpine scouts.
-- Horde rally: creature.sql guid=118431 entry=27118; distinct ground starts, 137.54 yd apart.
-- Alliance target: creature.sql guid=118265 entry=27117; distinct ground starts, 137.54 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'Grizzly_Blackriver',0,0,1,0,73,80,571,
  3251.18,-2572.47,52.9944,5.94964,3381.14,-2617.5,52.9239,2.808047,
  5,10,5,10,10,25,100,900,
  0,0,0
);

-- Grizzly_VentureBay: Venture Bay outer street approaches from the Conquest Hold and Westfall Brigade patrols.
-- Horde rally: waypoint_data.sql path/entry=1058540 point=1; distinct ground starts, 139.26 yd apart.
-- Alliance target: waypoint_data.sql path/entry=1059320 point=1; distinct ground starts, 139.26 yd apart.
INSERT IGNORE INTO `pvp_life_zone`
(
  `name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
  `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
  `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,
  `last_start`,`challenge_players`,`bot_chat`
) VALUES (
  'Grizzly_VentureBay',0,0,1,0,73,80,571,
  2681.92,-1863.33,14.1966,4.351979,2632.81,-1993.64,8.53884,1.210387,
  5,10,5,10,10,25,100,900,
  0,0,0
);
