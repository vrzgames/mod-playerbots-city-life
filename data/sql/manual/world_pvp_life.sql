-- PvP Life
-- Apply to the AzerothCore WORLD database.

CREATE TABLE IF NOT EXISTS `pvp_life_zone` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(64) NOT NULL,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `activity_type` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Skirmish,1=Duel,2=ForTheHorde,3=ForTheAlliance',
  `attacker_team` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '0=Alliance,1=Horde,2=Any',
  `defender_team` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Alliance,1=Horde,2=Any',
  `min_level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_level` TINYINT UNSIGNED NOT NULL DEFAULT 80,
  `map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `rally_x` FLOAT NOT NULL DEFAULT 0,
  `rally_y` FLOAT NOT NULL DEFAULT 0,
  `rally_z` FLOAT NOT NULL DEFAULT 0,
  `rally_o` FLOAT NOT NULL DEFAULT 0,
  `target_x` FLOAT NOT NULL DEFAULT 0,
  `target_y` FLOAT NOT NULL DEFAULT 0,
  `target_z` FLOAT NOT NULL DEFAULT 0,
  `target_o` FLOAT NOT NULL DEFAULT 0,
  `attackers_min` SMALLINT UNSIGNED NOT NULL DEFAULT 3,
  `attackers_max` SMALLINT UNSIGNED NOT NULL DEFAULT 8,
  `defenders_min` SMALLINT UNSIGNED NOT NULL DEFAULT 3,
  `defenders_max` SMALLINT UNSIGNED NOT NULL DEFAULT 8,
  `duration_min` SMALLINT UNSIGNED NOT NULL DEFAULT 10 COMMENT 'minutes',
  `duration_max` SMALLINT UNSIGNED NOT NULL DEFAULT 25 COMMENT 'minutes',
  `weight` INT UNSIGNED NOT NULL DEFAULT 100,
  `cooldown_seconds` INT UNSIGNED NOT NULL DEFAULT 900,
  `last_start` INT UNSIGNED NOT NULL DEFAULT 0,
  `challenge_players` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `bot_chat` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_pvp_life_zone_name` (`name`),
  KEY `idx_pvp_life_zone_activity` (`enabled`,`activity_type`,`last_start`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `pvp_life_chat` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `activity_type` TINYINT UNSIGNED NOT NULL COMMENT '2=ForTheHorde,3=ForTheAlliance',
  `channel` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Yell,1=World',
  `speaker_team` TINYINT UNSIGNED NOT NULL DEFAULT 2 COMMENT '0=Alliance,1=Horde,2=Any',
  `text` VARCHAR(255) NOT NULL,
  `weight` INT UNSIGNED NOT NULL DEFAULT 100,
  PRIMARY KEY (`id`),
  KEY `idx_pvp_life_chat_pick` (`enabled`,`activity_type`,`channel`,`speaker_team`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Re-applicable seed data.
DELETE FROM `pvp_life_zone` WHERE `name` IN (
  'StormwindDuel','OrgrimmarDuel','STV_Nesingwary','STV_Gurubashi','Tanaris_Gadgetzan',
  'DarkPortal_Azeroth','DarkPortal_Outland','Shattrath_Outskirts','K3_StormPeaks',
  'Goldshire_Roaming','Durotar_Roaming','Orgrimmar_Zeppelin',
  'ForTheHorde_Stormwind','ForTheAlliance_Orgrimmar','Wintergrasp'
);

INSERT INTO `pvp_life_zone`
(`name`,`enabled`,`activity_type`,`attacker_team`,`defender_team`,`min_level`,`max_level`,`map_id`,
 `rally_x`,`rally_y`,`rally_z`,`rally_o`,`target_x`,`target_y`,`target_z`,`target_o`,
 `attackers_min`,`attackers_max`,`defenders_min`,`defenders_max`,`duration_min`,`duration_max`,`weight`,`cooldown_seconds`,`last_start`,`challenge_players`,`bot_chat`)
VALUES
-- Duel zones: intentionally level 1-80. Real-player challenge still obeys MaxLevelDifference.
-- Both teams use the matching game_tele location as their center; PvPLife adds small placement jitter.
('StormwindDuel',1,1,0,0,1,80,0, -9122.49,350.862,93.7893,4.19046, -9122.49,350.862,93.7893,4.19046, 4,8,4,8,20,40,220,30,0,1,0), -- allyduelzone
('OrgrimmarDuel',1,1,1,1,1,80,1, 1282.04,-4395.88,26.3002,3.63561, 1282.04,-4395.88,26.3002,3.63561, 4,8,4,8,20,40,220,30,0,1,0), -- hordeduelzone

-- Normal world-PvP hotspots. The marked rows use the server's custom game_tele points.
('STV_Nesingwary',1,0,1,0,25,50,0, -11490.5,-302.536,35.7362,3.57, -11490.5,-302.536,35.7362,3.57, 5,10,5,10,12,25,120,600,0,0,0), -- stvpvp
('STV_Gurubashi',1,0,0,1,30,55,0, -13290.0,-274.0,20.0,0.0, -13245.0,-266.0,21.0,0.0, 6,12,6,12,12,25,130,600,0,0,0),
('Tanaris_Gadgetzan',1,0,1,0,40,65,1, -7200.0,-3860.0,9.0,0.0, -7165.0,-3805.0,9.0,0.0, 5,10,5,10,12,25,110,720,0,0,0),
('DarkPortal_Azeroth',1,0,0,1,55,80,0, -11820.0,-3200.0,-30.0,0.0, -11905.0,-3204.0,-14.0,0.0, 6,14,6,14,15,30,135,900,0,0,0),
('DarkPortal_Outland',1,0,1,0,58,80,530, -320.0,930.0,84.0,0.0, -248.0,922.0,84.0,0.0, 6,14,6,14,15,30,135,900,0,0,0),
('Shattrath_Outskirts',1,0,0,1,60,80,530, -1878.6,4927.39,-10.3383,4.33574, -1878.6,4927.39,-10.3383,4.33574, 5,12,5,12,12,25,100,900,0,0,0), -- shattrathpvp
('K3_StormPeaks',1,0,1,0,70,80,571, 6197.19,-1001.47,408.121,4.65582, 6197.19,-1001.47,408.121,4.65582, 5,12,5,12,12,25,105,900,0,0,0), -- k3pvp
('Wintergrasp',1,0,1,0,70,80,571, 4700.1,2867.58,387.683,3.3566, 4700.1,2867.58,387.683,3.3566, 5,12,5,12,12,25,115,900,0,0,0), -- wintergrasppvp

-- Cross-faction roaming near classic social/PvP areas.
('Goldshire_Roaming',1,0,1,0,5,30,0, -9520.0,55.0,58.0,0.0, -9465.0,64.0,56.0,0.0, 2,5,2,5,8,18,85,720,0,0,0),
('Durotar_Roaming',1,0,0,1,5,30,1, 1375.0,-4550.0,24.0,0.0, 1450.0,-4418.0,25.0,0.0, 2,5,2,5,8,18,85,720,0,0,0),

-- Optional extra hotspot near the Orgrimmar zeppelin area. Disabled until its exact point is tuned in-game.
('Orgrimmar_Zeppelin',0,0,0,1,10,80,1, 1320.0,-4630.0,25.0,0.0, 1360.0,-4590.0,25.0,0.0, 2,6,2,6,8,18,60,900,0,0,0),

-- For the Horde / For the Alliance. These are world-PvP faction pressure zones, not server-announced events.
('ForTheHorde_Stormwind',1,2,1,0,70,80,0, -9465.0,64.0,56.0,0.0, -8830.0,622.0,94.0,0.0, 6,14,6,14,15,25,100,3600,0,0,1),
('ForTheAlliance_Orgrimmar',1,3,0,1,70,80,1, 1915.0,-4755.0,40.0,0.0, 1500.0,-4415.0,22.0,0.0, 6,14,6,14,15,25,100,3600,0,0,1);

DELETE FROM `pvp_life_chat` WHERE `activity_type` IN (2,3);

INSERT INTO `pvp_life_chat`
(`enabled`,`activity_type`,`channel`,`speaker_team`,`text`,`weight`)
VALUES
-- For the Horde: Alliance defenders reacting.
(1,2,0,0,'Horde in Stormwind!',100),
(1,2,0,0,'Horde at the gates!',80),
(1,2,0,0,'Defend Stormwind!',80),
(1,2,0,0,'Horde in SW!',100),
(1,2,1,0,'horde in sw',120),
(1,2,1,0,'horde at sw gates',90),
(1,2,1,0,'need help sw',100),
(1,2,1,0,'horde raid sw go def',75),
(1,2,1,0,'horde in stormwind',90),

-- For the Alliance: Horde defenders reacting.
(1,3,0,1,'Alliance in Orgrimmar!',100),
(1,3,0,1,'Alliance at the gates!',80),
(1,3,0,1,'Defend Orgrimmar!',80),
(1,3,0,1,'Alliance in Org!',100),
(1,3,1,1,'ally in org',120),
(1,3,1,1,'alliance in org',90),
(1,3,1,1,'need help org',100),
(1,3,1,1,'ally raid org go def',75),
(1,3,1,1,'alliance at org gates',90);
