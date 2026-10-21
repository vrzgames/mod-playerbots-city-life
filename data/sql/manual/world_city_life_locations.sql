-- City Life: optional additional social hubs and separate faction quest populations.
-- Apply manually to WORLD after world_city_life.sql has created the existing tables.
-- Rerunnable additive data only: existing named hubs and spots are never modified or deleted.
-- New locations are opt-in in the combined configuration (Enable = 0).
-- Database enabled=0 keeps new hubs inactive until explicitly enabled in the configuration.
-- Coordinates come from existing ground NPC spawns in data/sql/base/db_world/creature.sql.
-- Quest radius evidence is recorded below; separate faction centers can have overlapping routes.

INSERT IGNORE INTO `city_life_hub`
(`name`,`enabled`,`team`,`min_level`,`max_level`,`map_id`,`default_population`,`priority`)
SELECT `seed`.`name`,0,`seed`.`team`,`seed`.`min_level`,`seed`.`max_level`,`seed`.`map_id`,
       `seed`.`population`,`seed`.`priority`
FROM
(
  SELECT 'Lakeshire' AS `name`,0 AS `team`,15 AS `min_level`,80 AS `max_level`,
         0 AS `map_id`,6 AS `population`,100 AS `priority`
  UNION ALL SELECT 'SentinelHill',0,10,80,0,6,100
  UNION ALL SELECT 'Theramore',0,35,80,1,8,100
  UNION ALL SELECT 'MenethilHarbor',0,20,80,0,6,100
  UNION ALL SELECT 'Auberdine',0,10,80,1,6,100
  UNION ALL SELECT 'Crossroads',1,10,80,1,8,100
  UNION ALL SELECT 'RazorHill',1,5,80,1,6,100
  UNION ALL SELECT 'Brill',1,5,80,0,6,100
  UNION ALL SELECT 'Stonard',1,35,80,0,6,100
  UNION ALL SELECT 'CampTaurajo',1,15,80,1,6,100
  UNION ALL SELECT 'Ratchet',2,10,80,1,8,100
  UNION ALL SELECT 'Everlook',2,50,80,1,6,100
  UNION ALL SELECT 'CenarionHold',2,55,80,1,6,100
  UNION ALL SELECT 'LightsHopeChapel',2,55,80,0,6,100
  UNION ALL SELECT 'SouthshoreQuesting',0,25,35,0,6,170
  UNION ALL SELECT 'TarrenMillQuesting',1,25,35,0,6,170
  UNION ALL SELECT 'RefugePointeQuesting',0,30,40,0,6,170
  UNION ALL SELECT 'HammerfallQuesting',1,30,40,0,6,170
  UNION ALL SELECT 'AstranaarQuesting',0,20,30,1,6,170
  UNION ALL SELECT 'SplintertreePostQuesting',1,20,30,1,6,170
  UNION ALL SELECT 'TelaarQuesting',0,64,68,530,6,170
  UNION ALL SELECT 'GaradarQuesting',1,64,68,530,6,170
  UNION ALL SELECT 'AllerianStrongholdQuesting',0,62,66,530,6,170
  UNION ALL SELECT 'StonebreakerHoldQuesting',1,62,66,530,6,170
  UNION ALL SELECT 'ValgardeQuesting',0,68,72,571,6,170
  UNION ALL SELECT 'VengeanceLandingQuesting',1,68,72,571,6,170
  UNION ALL SELECT 'AmberpineLodgeQuesting',0,73,76,571,6,170
  UNION ALL SELECT 'ConquestHoldQuesting',1,73,76,571,6,170
  UNION ALL SELECT 'GromgolQuesting',1,30,40,0,6,170
  UNION ALL SELECT 'HonorHoldQuesting',0,58,63,530,6,170
  UNION ALL SELECT 'ThrallmarQuesting',1,58,63,530,6,170
) AS `seed`;

-- Lakeshire/VernerOsgood: creature GUID 6719, entry 415.
-- Lakeshire/MarshalMarris: creature GUID 6720, entry 382.
-- SentinelHill/ScoutGaliaan: creature GUID 66973, entry 878.
-- SentinelHill/Thor: creature GUID 89522, entry 523.
-- Theramore/LieutenantAden: creature GUID 30636, entry 23951.
-- Theramore/GuardByron: creature GUID 30666, entry 4921.
-- MenethilHarbor/ShelleiBrondir: creature GUID 9448, entry 1571.
-- MenethilHarbor/ValstagIronjaw: creature GUID 9503, entry 2086.
-- Auberdine/CaylaisMoonfeather: creature GUID 36537, entry 3841.
-- Auberdine/SentinelGlyndaNalShea: creature GUID 37103, entry 2930.
-- Crossroads/Mankrik: creature GUID 14348, entry 3432.
-- Crossroads/Devrak: creature GUID 13419, entry 3615.
-- RazorHill/OrgnilSoulscar: creature GUID 3396, entry 3142.
-- RazorHill/CookTorka: creature GUID 6460, entry 3191.
-- Brill/ExecutorZygand: creature GUID 29797, entry 1515.
-- Brill/DeathguardBurgess: creature GUID 29798, entry 1652.
-- Stonard/Breyk: creature GUID 32093, entry 6026.
-- Stonard/Dar: creature GUID 33816, entry 5591.
-- CampTaurajo/KirgeSternhorn: creature GUID 14136, entry 3418.
-- CampTaurajo/OmusaThunderhorn: creature GUID 15033, entry 10378.
-- Ratchet/Bragok: creature GUID 14323, entry 16227.
-- Ratchet/MebokMizzyrix: creature GUID 14408, entry 3446.
-- Everlook/UmiRumplesnicker: creature GUID 41764, entry 10305.
-- Everlook/MalyfousDarkhammer: creature GUID 42207, entry 10637.
-- CenarionHold/Vargus: creature GUID 42757, entry 15176.
-- CenarionHold/WindcallerProudhorn: creature GUID 42918, entry 15191.
-- LightsHopeChapel/CarlinRedpath: creature GUID 53737, entry 11063.
-- LightsHopeChapel/KhaelynSteelwing: creature GUID 53177, entry 12617.
-- SouthshoreQuesting/LieutenantFarrenOrinelle: creature GUID 15305, entry 2228.
-- Quest 536: all objective/reward POIs fit 800 yards (maximum 570.6); zone 267.
-- TarrenMillQuesting/Krusk: creature GUID 15291, entry 2229.
-- Quest 533: all objective/reward POIs fit 700 yards (maximum 440.6); zone 267.
-- RefugePointeQuesting/CaptainNials: creature GUID 11805, entry 2700.
-- Quest 681: all objective/reward POIs fit 1000 yards (maximum 795.8); zone 45.
-- HammerfallQuesting/Torgan: creature GUID 11291, entry 2706.
-- Quest 671: all objective/reward POIs fit 1800 yards (maximum 1652.3); zone 45.
-- AstranaarQuesting/ShindrellSwiftfire: creature GUID 32328, entry 3845.
-- Quest 1008: all objective/reward POIs fit 2300 yards (maximum 2198.4); zone 331.
-- SplintertreePostQuesting/Pixel: creature GUID 33165, entry 12724.
-- Quest 6441: all objective/reward POIs fit 1000 yards (maximum 798.0); zone 331.
-- TelaarQuesting/HuntressBintook: creature GUID 65721, entry 18353.
-- Quest 9917: all objective/reward POIs fit 700 yards (maximum 449.6); zone 3518.
-- GaradarQuesting/FarseerKurkush: creature GUID 63910, entry 18066.
-- Quest 9863: all objective/reward POIs fit 1500 yards (maximum 1361.8); zone 3518.
-- AllerianStrongholdQuesting/Bertelm: creature GUID 65792, entry 18387.
-- Quest 9998: all objective/reward POIs fit 900 yards (maximum 725.2); zone 3519.
-- StonebreakerHoldQuesting/Rakoria: creature GUID 65790, entry 18385.
-- Quest 9993: all objective/reward POIs fit 1400 yards (maximum 1248.0); zone 3519.
-- ValgardeQuesting/Macalroy: creature GUID 133903, entry 23547.
-- ValgardeQuesting/ViceAdmiralKeller: creature GUID 119573, entry 23546.
-- Quest 11243: all objective/reward POIs fit 700 yards (maximum 341.7); zone 495.
-- Quest 11243 follows 11228 (the short Valgarde introduction); no quest progress is granted.
-- VengeanceLandingQuesting/ApothecaryLysander: creature GUID 101906, entry 24126.
-- Quest 11167: all objective/reward POIs fit 700 yards (maximum 328.9); zone 495.
-- AmberpineLodgeQuesting/MasterWoodsmanAnderhol: creature GUID 108840, entry 27277.
-- Quest 12215: all objective/reward POIs fit 1200 yards (maximum 1064.4); zone 394.
-- ConquestHoldQuesting/SergeantNazgrim: creature GUID 120095, entry 27388.
-- Quest 12257: all objective/reward POIs fit 1100 yards (maximum 858.7); zone 394.
-- GromgolQuesting/CommanderAggrogosh: creature GUID 721, entry 2464.
-- Quest 568: all objective/reward POIs fit 900 yards (maximum 614.7); zone 33.
-- HonorHoldQuesting/LieutenantAmadi: creature GUID 57877, entry 16820.
-- HonorHoldQuesting/HonorGuardWesilow: creature GUID 57885, entry 16827.
-- Quest 10482: all objective/reward POIs fit 1100 yards (maximum 732.3); zone 3483.
-- Quest 10050: all objective/reward POIs fit 1100 yards (maximum 933.7); zone 3483.
-- ThrallmarQuesting/VurtokAxebreaker: creature GUID 74493, entry 21256.
-- Quest 10450: all objective/reward POIs fit 1000 yards (maximum 752.5); zone 3483.
INSERT IGNORE INTO `city_life_spot`
(`hub_id`,`name`,`category`,`enabled`,`x`,`y`,`z`,`o`,`radius`,`weight`)
SELECT `hub`.`id`,`seed`.`spot_name`,`seed`.`category`,1,`seed`.`x`,`seed`.`y`,`seed`.`z`,`seed`.`o`,
       `seed`.`radius`,`seed`.`weight`
FROM `city_life_hub` AS `hub`
INNER JOIN
(
  SELECT 'Lakeshire' AS `hub_name`,'VernerOsgood' AS `spot_name`,'craft' AS `category`,
         -9259.44 AS `x`,-2243.22 AS `y`,64.0584 AS `z`,3.14159 AS `o`,
         2.0 AS `radius`,100 AS `weight`
  UNION ALL SELECT 'Lakeshire','MarshalMarris','square',
                   -9284.03,-2298.16,67.5772,1.13446,2.0,100
  UNION ALL SELECT 'SentinelHill','ScoutGaliaan','square',
                   -10636.3,1127.26,33.8767,5.91667,2.0,100
  UNION ALL SELECT 'SentinelHill','Thor','flight',
                   -10628.3,1037.27,34.1915,2.28638,2.0,100
  UNION ALL SELECT 'Theramore','LieutenantAden','square',
                   -3682.86,-4391.41,10.7306,0.645772,2.0,100
  UNION ALL SELECT 'Theramore','GuardByron','square',
                   -3645.7,-4448.1,15.3483,3.21141,2.0,100
  UNION ALL SELECT 'MenethilHarbor','ShelleiBrondir','flight',
                   -3793.2,-782.052,9.09773,5.2709,2.0,100
  UNION ALL SELECT 'MenethilHarbor','ValstagIronjaw','square',
                   -3716.14,-807.058,10.0639,5.91667,2.0,100
  UNION ALL SELECT 'Auberdine','CaylaisMoonfeather','flight',
                   6343.2,561.651,16.1047,5.0091,2.0,100
  UNION ALL SELECT 'Auberdine','SentinelGlyndaNalShea','square',
                   6438.62,471.896,8.12515,1.91474,2.0,100
  UNION ALL SELECT 'Crossroads','Mankrik','square',
                   -520.983,-2641.41,95.8708,4.43314,2.0,100
  UNION ALL SELECT 'Crossroads','Devrak','flight',
                   -437.137,-2596.0,95.8708,3.7001,2.0,100
  UNION ALL SELECT 'RazorHill','OrgnilSoulscar','square',
                   287.265,-4724.88,13.2142,5.2709,2.0,100
  UNION ALL SELECT 'RazorHill','CookTorka','square',
                   312.099,-4664.95,16.4254,4.17134,2.0,100
  UNION ALL SELECT 'Brill','ExecutorZygand','square',
                   2278.08,295.587,35.3301,1.93731,2.0,100
  UNION ALL SELECT 'Brill','DeathguardBurgess','square',
                   2270.7,279.998,35.3226,2.70526,2.0,100
  UNION ALL SELECT 'Stonard','Breyk','flight',
                   -10459.2,-3279.76,21.5445,0.733038,2.0,100
  UNION ALL SELECT 'Stonard','Dar','square',
                   -10495.7,-3248.14,21.1127,4.72984,2.0,100
  UNION ALL SELECT 'CampTaurajo','KirgeSternhorn','square',
                   -2347.39,-1925.27,95.8728,2.6529,2.0,100
  UNION ALL SELECT 'CampTaurajo','OmusaThunderhorn','flight',
                   -2384.08,-1880.94,95.9336,6.05629,2.0,100
  UNION ALL SELECT 'Ratchet','Bragok','flight',
                   -898.246,-3769.65,11.7932,5.41052,2.0,100
  UNION ALL SELECT 'Ratchet','MebokMizzyrix','square',
                   -928.982,-3697.23,8.06642,3.08923,2.0,100
  UNION ALL SELECT 'Everlook','UmiRumplesnicker','square',
                   6752.67,-4639.33,721.837,2.21657,2.0,100
  UNION ALL SELECT 'Everlook','MalyfousDarkhammer','craft',
                   6697.55,-4647.11,721.089,3.12414,2.0,100
  UNION ALL SELECT 'CenarionHold','Vargus','craft',
                   -6860.93,753.044,42.5555,1.20428,2.0,100
  UNION ALL SELECT 'CenarionHold','WindcallerProudhorn','square',
                   -6847.81,755.68,42.2256,0.558505,2.0,100
  UNION ALL SELECT 'LightsHopeChapel','CarlinRedpath','square',
                   2257.31,-5340.87,86.8037,1.48353,2.0,100
  UNION ALL SELECT 'LightsHopeChapel','KhaelynSteelwing','flight',
                   2269.85,-5345.39,87.0242,1.76278,2.0,100
  UNION ALL SELECT 'SouthshoreQuesting','LieutenantFarrenOrinelle','quest',
                   -845.604,-580.169,11.7881,4.69494,2.0,100
  UNION ALL SELECT 'TarrenMillQuesting','Krusk','quest',
                   -40.6782,-956.998,56.2796,2.05949,2.0,100
  UNION ALL SELECT 'RefugePointeQuesting','CaptainNials','quest',
                   -1274.69,-2516.63,21.4792,5.27122,2.0,100
  UNION ALL SELECT 'HammerfallQuesting','Torgan','quest',
                   -1004.31,-3556.5,57.2718,2.89725,2.0,100
  UNION ALL SELECT 'AstranaarQuesting','ShindrellSwiftfire','quest',
                   2795.75,-299.374,107.183,1.71042,2.0,100
  UNION ALL SELECT 'SplintertreePostQuesting','Pixel','quest',
                   2309.79,-2513.19,105.394,5.20108,2.0,100
  UNION ALL SELECT 'TelaarQuesting','HuntressBintook','quest',
                   -2556.34,7254.19,15.2034,1.76278,2.0,100
  UNION ALL SELECT 'GaradarQuesting','FarseerKurkush','quest',
                   -1409.9,7269.19,25.5191,2.6529,2.0,100
  UNION ALL SELECT 'AllerianStrongholdQuesting','Bertelm','quest',
                   -2971.17,3935.18,3.86593,1.69297,2.0,100
  UNION ALL SELECT 'StonebreakerHoldQuesting','Rakoria','quest',
                   -2615.73,4376.06,35.5522,1.5708,2.0,100
  UNION ALL SELECT 'ValgardeQuesting','Macalroy','quest',
                   593.145,-5088.97,5.63663,4.47322,2.0,100
  UNION ALL SELECT 'ValgardeQuesting','ViceAdmiralKeller','quest',
                   654.667,-5057.84,1.79658,5.62547,2.0,100
  UNION ALL SELECT 'VengeanceLandingQuesting','ApothecaryLysander','quest',
                   1948.54,-6146.63,24.2777,2.89725,2.0,100
  UNION ALL SELECT 'AmberpineLodgeQuesting','MasterWoodsmanAnderhol','quest',
                   3417.8,-2795.63,201.604,2.09439,2.0,100
  UNION ALL SELECT 'ConquestHoldQuesting','SergeantNazgrim','quest',
                   3274.94,-2213.25,117.112,2.53073,2.0,100
  UNION ALL SELECT 'GromgolQuesting','CommanderAggrogosh','quest',
                   -12398.4,167.963,3.12308,3.27453,2.0,100
  UNION ALL SELECT 'HonorHoldQuesting','LieutenantAmadi','quest',
                   -591.543,2910.22,59.2956,4.76475,2.0,100
  UNION ALL SELECT 'HonorHoldQuesting','HonorGuardWesilow','quest',
                   -597.186,2912.03,59.2966,5.42797,2.0,100
  UNION ALL SELECT 'ThrallmarQuesting','VurtokAxebreaker','quest',
                   228.036,2692.46,90.8036,0.488692,2.0,100
) AS `seed` ON `seed`.`hub_name`=`hub`.`name`;
