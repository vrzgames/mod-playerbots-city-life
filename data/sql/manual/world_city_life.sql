-- City Life
-- Apply to the AzerothCore WORLD database.

CREATE TABLE IF NOT EXISTS `city_life_hub` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(64) NOT NULL,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `team` TINYINT UNSIGNED NOT NULL DEFAULT 2 COMMENT '0=Alliance,1=Horde,2=Any',
  `min_level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_level` TINYINT UNSIGNED NOT NULL DEFAULT 80,
  `map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `default_population` SMALLINT UNSIGNED NOT NULL DEFAULT 10,
  `priority` SMALLINT UNSIGNED NOT NULL DEFAULT 100,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_city_life_hub_name` (`name`),
  KEY `idx_city_life_hub_enabled_priority` (`enabled`,`priority`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `city_life_spot` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `hub_id` INT UNSIGNED NOT NULL,
  `name` VARCHAR(64) NOT NULL,
  `category` VARCHAR(32) NOT NULL DEFAULT 'square',
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `x` FLOAT NOT NULL DEFAULT 0,
  `y` FLOAT NOT NULL DEFAULT 0,
  `z` FLOAT NOT NULL DEFAULT 0,
  `o` FLOAT NOT NULL DEFAULT 0,
  `radius` FLOAT UNSIGNED NOT NULL DEFAULT 4,
  `weight` SMALLINT UNSIGNED NOT NULL DEFAULT 100,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_city_life_spot_name` (`hub_id`,`name`),
  KEY `idx_city_life_spot_hub_enabled` (`hub_id`,`enabled`),
  CONSTRAINT `fk_city_life_spot_hub` FOREIGN KEY (`hub_id`) REFERENCES `city_life_hub` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Re-applicable seed hubs. Config values override default_population at runtime.
INSERT INTO `city_life_hub`
(`name`,`enabled`,`team`,`min_level`,`max_level`,`map_id`,`default_population`,`priority`)
VALUES
('Stormwind',1,0,1,80,0,25,220),
('Orgrimmar',1,1,1,80,1,25,220),
('Ironforge',1,0,1,80,0,18,190),
('Undercity',1,1,1,80,0,18,190),
('Darnassus',1,0,1,80,1,10,150),
('ThunderBluff',1,1,1,80,1,10,150),
('Exodar',1,0,1,80,530,5,120),
('Silvermoon',1,1,1,80,530,8,130),
('Shattrath',1,2,58,80,530,20,200),
('Dalaran',1,2,68,80,571,25,210),
('WintergraspAlliance',1,0,70,80,571,40,230),
('WintergraspHorde',1,1,70,80,571,40,230),
('Gadgetzan',1,2,35,80,1,8,110),
('Goldshire',1,0,1,30,0,8,100),
('HonorHold',1,0,58,80,530,6,100),
('Thrallmar',1,1,58,80,530,6,100)
ON DUPLICATE KEY UPDATE
`enabled`=VALUES(`enabled`),`team`=VALUES(`team`),`min_level`=VALUES(`min_level`),
`max_level`=VALUES(`max_level`),`map_id`=VALUES(`map_id`),
`default_population`=VALUES(`default_population`),`priority`=VALUES(`priority`);

-- Replace only the bundled spots. Custom hubs remain untouched.
DELETE `spot` FROM `city_life_spot` AS `spot`
INNER JOIN `city_life_hub` AS `hub` ON `hub`.`id`=`spot`.`hub_id`
WHERE `hub`.`name` IN
('Stormwind','Orgrimmar','Ironforge','Undercity','Darnassus','ThunderBluff','Exodar','Silvermoon',
 'Shattrath','Dalaran','WintergraspAlliance','WintergraspHorde','Gadgetzan','Goldshire','HonorHold',
 'Thrallmar');

-- Coordinates are based on AzerothCore 3.3.5 game_tele and service NPC/gameobject positions.
-- Use .citylife spot add in game to tune a point for a different navmesh or custom world database.
INSERT INTO `city_life_spot`
(`hub_id`,`name`,`category`,`enabled`,`x`,`y`,`z`,`o`,`radius`,`weight`)
SELECT `hub`.`id`,`seed`.`spot_name`,`seed`.`category`,1,`seed`.`x`,`seed`.`y`,`seed`.`z`,`seed`.`o`,
       `seed`.`radius`,`seed`.`weight`
FROM `city_life_hub` AS `hub`
INNER JOIN
(
  SELECT 'Stormwind' AS `hub_name`,'MainSquare' AS `spot_name`,'square' AS `category`,
         -8833.38 AS `x`,628.628 AS `y`,94.0066 AS `z`,1.06535 AS `o`,8 AS `radius`,100 AS `weight`
  UNION ALL SELECT 'Stormwind','AuctionHouse','auction',-8821.53,659.886,97.4645,0.401426,5,150
  UNION ALL SELECT 'Stormwind','Bank','bank',-8931.23,605.801,99.606,0.488692,5,125
  UNION ALL SELECT 'Stormwind','Mailbox','mail',-8861.5,636.744,96.1785,1.91113,5,120
  UNION ALL SELECT 'Stormwind','FlightMaster','flight',-8835.76,490.084,109.699,4.04916,5,65

  UNION ALL SELECT 'Orgrimmar','MainSquare','square',1629.85,-4373.64,31.5573,3.69762,8,100
  UNION ALL SELECT 'Orgrimmar','AuctionHouse','auction',1592.8,-4397.05,7.46388,0.139626,5,150
  UNION ALL SELECT 'Orgrimmar','Bank','bank',1627.42,-4376.04,12.0548,3.68265,5,125
  UNION ALL SELECT 'Orgrimmar','Mailbox','mail',1607.49,-4374.26,9.76904,3.28996,5,120
  UNION ALL SELECT 'Orgrimmar','FlightMaster','flight',1676.25,-4313.45,61.8944,5.25344,5,65

  UNION ALL SELECT 'Ironforge','MainSquare','square',-4918.88,-940.406,501.564,5.42347,8,100
  UNION ALL SELECT 'Ironforge','AuctionHouse','auction',-4948.01,-901.528,505.172,3.80482,5,150
  UNION ALL SELECT 'Ironforge','Bank','bank',-4877.43,-990.034,504.024,2.30383,5,125
  UNION ALL SELECT 'Ironforge','Mailbox','mail',-4910.38,-976.212,501.408,2.26893,5,120
  UNION ALL SELECT 'Ironforge','FlightMaster','flight',-4821.13,-1152.4,502.295,4.31096,5,65

  UNION ALL SELECT 'Undercity','MainSquare','square',1584.14,240.308,-52.1534,0.041793,8,100
  UNION ALL SELECT 'Undercity','AuctionHouse','auction',1542.45,255.202,-56.7948,1.01229,5,150
  UNION ALL SELECT 'Undercity','Bank','bank',1591.49,240.328,-52.0596,3.03687,5,125
  UNION ALL SELECT 'Undercity','Mailbox','mail',1554.97,235.108,-43.201,0.252436,5,120
  UNION ALL SELECT 'Undercity','FlightMaster','flight',1567.12,266.345,-43.0194,0.820305,5,65

  UNION ALL SELECT 'Darnassus','MainSquare','square',9949.56,2284.21,1341.4,1.59587,8,100
  UNION ALL SELECT 'Darnassus','AuctionHouse','auction',9872.6,2341.73,1321.67,3.52556,5,150
  UNION ALL SELECT 'Darnassus','Bank','bank',9945.15,2518.39,1317.66,3.9968,5,125
  UNION ALL SELECT 'Darnassus','Mailbox','mail',9916.29,2348.2,1330.7,3.18527,5,120

  UNION ALL SELECT 'ThunderBluff','MainRise','square',-1277.37,124.804,131.287,5.22274,8,100
  UNION ALL SELECT 'ThunderBluff','AuctionHouse','auction',-1210.21,94.8587,134.535,2.98451,5,150
  UNION ALL SELECT 'ThunderBluff','Bank','bank',-1262.78,24.0055,128.27,0.10472,5,125
  UNION ALL SELECT 'ThunderBluff','Mailbox','mail',-1263.31,44.5451,127.545,4.72984,5,120
  UNION ALL SELECT 'ThunderBluff','FlightMaster','flight',-1196.75,26.0777,177.033,1.71042,5,65

  UNION ALL SELECT 'Exodar','MainSquare','square',-3965.7,-11653.6,-138.844,0.852154,8,100
  UNION ALL SELECT 'Exodar','AuctionHouse','auction',-4027.62,-11732.4,-151.822,0.436332,5,150
  UNION ALL SELECT 'Exodar','Bank','bank',-3923.77,-11544.5,-150.193,4.58778,5,125
  UNION ALL SELECT 'Exodar','Mailbox','mail',-3975.04,-11700,-139.258,3.14157,5,120
  UNION ALL SELECT 'Exodar','FlightMaster','flight',-4057.15,-11788.6,8.87662,5.59502,5,65

  UNION ALL SELECT 'Silvermoon','MainSquare','square',9487.69,-7279.2,14.2866,6.16478,8,100
  UNION ALL SELECT 'Silvermoon','AuctionHouse','auction',9641.82,-7135.55,16.8566,3.19395,5,150
  UNION ALL SELECT 'Silvermoon','Bank','bank',9515.41,-7221.62,16.2139,1.50098,5,125
  UNION ALL SELECT 'Silvermoon','Mailbox','mail',9515.66,-7262.47,14.1913,4.72984,5,120
  UNION ALL SELECT 'Silvermoon','FlightMaster','flight',9376.4,-7164.92,9.0194,3.1765,5,65

  UNION ALL SELECT 'Shattrath','TerraceOfLight','square',-1838.16,5301.79,-12.428,5.9517,10,130
  UNION ALL SELECT 'Shattrath','Bank','bank',-2000.66,5351.82,-9.26761,3.52556,6,125
  UNION ALL SELECT 'Shattrath','Mailbox','mail',-1773.68,5171.82,-40.2134,2.24275,6,120
  UNION ALL SELECT 'Shattrath','FlightMaster','flight',-1831.95,5298.3,-12.3448,1.76278,6,85

  UNION ALL SELECT 'Dalaran','MainSquare','square',5807.98,588.487,660.94,1.66594,10,130
  UNION ALL SELECT 'Dalaran','AuctionHouse','auction',5927.63,731.576,643.253,4.69494,5,150
  UNION ALL SELECT 'Dalaran','AllianceBank','bank',5766.59,734.037,620.134,5.07891,6,110
  UNION ALL SELECT 'Dalaran','HordeBank','bank',5981.66,599.832,651.223,2.75762,6,110
  UNION ALL SELECT 'Dalaran','Mailbox','mail',5797.91,558.436,650.719,4.36332,5,120
  UNION ALL SELECT 'Dalaran','FlightMaster','flight',5813.37,453.403,658.834,4.45059,6,80

  -- Faction-separated Wintergrasp starting graveyards from BattlefieldWG.
  UNION ALL SELECT 'WintergraspAlliance','AllianceStart','square',5140.79,2179.12,390.95,1.97222,24,100
  UNION ALL SELECT 'WintergraspHorde','HordeStart','square',5032.454,3711.382,372.468,3.971623,24,100

  UNION ALL SELECT 'Gadgetzan','MainSquare','square',-7177.15,-3785.34,8.36981,6.10237,8,110
  UNION ALL SELECT 'Gadgetzan','AuctionHouse','auction',-7239.1,-3803.89,0.813843,0.017453,5,145
  UNION ALL SELECT 'Gadgetzan','Bank','bank',-7205.06,-3827.29,8.6442,0.174533,5,120
  UNION ALL SELECT 'Gadgetzan','Mailbox','mail',-7154.4,-3829.52,8.75029,4.2237,5,115
  UNION ALL SELECT 'Gadgetzan','FlightMaster','flight',-7224.87,-3738.21,8.48369,1.18682,5,75

  UNION ALL SELECT 'Goldshire','InnSquare','square',-9448.55,68.236,56.3225,2.1115,8,135
  UNION ALL SELECT 'Goldshire','Mailbox','mail',-9455.99,45.8229,56.4395,1.40499,5,120
  UNION ALL SELECT 'Goldshire','Road','travel',-9465,64,56,0,8,80

  UNION ALL SELECT 'HonorHold','KeepSquare','square',-748.211,2681.52,100.35,5.7479,8,130
  UNION ALL SELECT 'HonorHold','Mailbox','mail',-706.554,2700.94,94.538,1.4943,5,115
  UNION ALL SELECT 'HonorHold','FlightMaster','flight',-665.804,2715.48,94.1981,3.35103,5,85

  UNION ALL SELECT 'Thrallmar','MainSquare','square',156.251,2673.45,85.1587,0.382074,8,130
  UNION ALL SELECT 'Thrallmar','Mailbox','mail',172.726,2623.06,86.8361,3.56983,5,115
  UNION ALL SELECT 'Thrallmar','FlightMaster','flight',233.137,2632.3,88.3007,2.67035,5,85
) AS `seed` ON `seed`.`hub_name`=`hub`.`name`;
