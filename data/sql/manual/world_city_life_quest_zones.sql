-- City Life: add town residents and separate outdoor quest populations to an existing installation.
-- Apply manually to the AzerothCore WORLD database after world_city_life.sql has created the tables.
-- This additive upgrade preserves all existing hub settings and spots, including customized new hubs.

INSERT INTO `city_life_hub`
(`name`,`enabled`,`team`,`min_level`,`max_level`,`map_id`,`default_population`,`priority`)
VALUES
('BootyBay',1,2,35,80,0,10,110),
('Darkshire',1,0,18,80,0,8,105),
('RebelCamp',1,0,30,80,0,6,105),
('ValianceKeep',1,0,68,80,571,10,115),
('WarsongHold',1,1,68,80,571,10,115),
('RebelCampQuesting',1,0,30,40,0,8,180),
('FarshireQuesting',1,0,68,72,571,8,180),
('WarsongHoldQuesting',1,1,68,72,571,8,180)
ON DUPLICATE KEY UPDATE `id`=`city_life_hub`.`id`;

-- Town centers: data/sql/base/db_world/game_tele.sql, named locations matching each city hub.
-- Service/questgiver spots: creature.sql, NPC entries 15677,8123,6790,2409,733,770,25235,25279,26083.
-- Booty Bay has stacked decks; keep the seeded radii small until checking your own navmesh in game.
INSERT INTO `city_life_spot`
(`hub_id`,`name`,`category`,`enabled`,`x`,`y`,`z`,`o`,`radius`,`weight`)
SELECT `hub`.`id`,`seed`.`spot_name`,`seed`.`category`,1,`seed`.`x`,`seed`.`y`,`seed`.`z`,`seed`.`o`,
       `seed`.`radius`,`seed`.`weight`
FROM `city_life_hub` AS `hub`
INNER JOIN
(
  SELECT 'BootyBay' AS `hub_name`,'Harbor' AS `spot_name`,'square' AS `category`,
         -14297.2 AS `x`,530.993 AS `y`,8.77916 AS `z`,3.98863 AS `o`,2 AS `radius`,110 AS `weight`
  UNION ALL SELECT 'BootyBay','AuctionHouse','auction',-14398.5,406.925,9.50589,1.51844,2,120
  UNION ALL SELECT 'BootyBay','Bank','bank',-14421.9,528.898,5.24876,5.14872,2,110

  UNION ALL SELECT 'Darkshire','TownSquare','square',-10573,-1182.51,28.0148,0.309022,4,120
  UNION ALL SELECT 'Darkshire','Inn','inn',-10516,-1161.21,28.1161,4.03171,2,100
  UNION ALL SELECT 'Darkshire','FlightMaster','flight',-10513.8,-1258.79,41.5138,4.36332,2,80

  UNION ALL SELECT 'RebelCamp','CampSquare','square',-11322.4,-202.492,75.6362,0.432339,3,120
  UNION ALL SELECT 'RebelCamp','SergeantYohwa','quest',-11310.4,-205.115,75.3625,2.80998,2,100
  UNION ALL SELECT 'RebelCamp','CorporalKaleb','quest',-11309.1,-187.431,75.6492,3.78736,2,100

  UNION ALL SELECT 'ValianceKeep','KeepSquare','square',2213.95,5273.15,11.2565,5.89294,4,120
  UNION ALL SELECT 'ValianceKeep','HildaStoneforge','quest',2336.46,5266.49,7.75584,3.62193,2,100

  UNION ALL SELECT 'WarsongHold','HoldSquare','square',2741.29,6097.16,76.9055,0.731543,4,120
  UNION ALL SELECT 'WarsongHold','OverlordRazgor','quest',2784.95,6081.02,77.7719,3.49066,2,100

  -- Quest populations start at outdoor questgivers; their movement is controlled separately from town residents.
  UNION ALL SELECT 'RebelCampQuesting','SergeantYohwa','quest',-11310.4,-205.115,75.3625,2.80998,3,100
  UNION ALL SELECT 'RebelCampQuesting','CorporalKaleb','quest',-11309.1,-187.431,75.6492,3.78736,3,100

  -- Additional patrol stops use creature.sql GUIDs 117434/116919 (Medic Hawthorn/Blythe).
  UNION ALL SELECT 'FarshireQuesting','GeraldGreen','quest',2483.26,5215.05,30.2763,0.471239,3,100
  UNION ALL SELECT 'FarshireQuesting','MedicHawthorn','quest',2248.83,5400.18,1.87424,3.03687,3,100
  UNION ALL SELECT 'FarshireQuesting','RecruitmentOfficerBlythe','quest',2299.58,5236.22,11.4287,2.77916,3,100

  -- GUIDs 125535/125590: Saurfang offers the quarry introduction; Holgoth offers quest 11606.
  UNION ALL SELECT 'WarsongHoldQuesting','OverlordRazgor','quest',2784.95,6081.02,77.7719,3.49066,3,100
  UNION ALL SELECT 'WarsongHoldQuesting','HighOverlordSaurfang','quest',2835.02,6182.44,84.6827,5.82198,3,100
  UNION ALL SELECT 'WarsongHoldQuesting','QuartermasterHolgoth','quest',2769.98,6073,77.6674,4.41568,3,100
) AS `seed` ON `seed`.`hub_name`=`hub`.`name`
ON DUPLICATE KEY UPDATE `id`=`city_life_spot`.`id`;
