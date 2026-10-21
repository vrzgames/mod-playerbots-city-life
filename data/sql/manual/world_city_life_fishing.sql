-- City Life: Fishing Life shore locations. Created by iCore.
-- Apply manually to WORLD after world_city_life.sql (and world_city_life_custom.sql on existing installs).
-- Additive and rerunnable: existing named hubs, populations, enabled values and coordinates are preserved.
-- No core WORLD tables, CHARACTER data or AUTH data are changed.
-- FishingLife config controls routine switches, populations, levels, timers and the dedicated bot pool.
-- New hubs start with database enabled=0; the FishingLife config activates them for this module version.
-- The three legacy CustomFishing names and their CustomAnchor rows preserve registered character identity.
-- New Fishing hubs belong exclusively to Fishing Life; the Custom master switch does not control them.
-- NPC anchors copy local WORLD positions, except the documented Stormwind trainer clearance offset.
-- Terrain anchors were calculated from the local extracted .map terrain/liquid data, not guessed heights.
-- These data checks do not replace runtime ground, collision/LOS, deep-water and navmesh validation.
-- New anchors have radius 4 so Population can spread several anglers without editing SQL.
-- Each randomized stand is validated at runtime; an unsafe part of the radius is rejected.
-- Wintergrasp fishing is suspended during battle by the runtime FishingLife setting.
-- WarsongHold uses the real Garrosh's Landing coast, because the hold's elevated plateau has no water.
-- INSERT IGNORE is deliberate: deleting/replacing rows would destroy administrator-customized locations.

INSERT IGNORE INTO `city_life_hub`
(`name`,`enabled`,`team`,`min_level`,`max_level`,`map_id`,`default_population`,`priority`)
VALUES
('CustomFishingStormwind',0,0,1,80,0,1,350),
('CustomFishingOrgrimmar',0,1,1,80,1,1,350),
('CustomFishingDalaran',0,2,1,80,571,1,350),
('FishingIronforge',0,0,1,80,0,2,350),
('FishingThunderBluff',0,1,1,80,1,2,350),
('FishingUndercity',0,1,1,80,0,2,350),
('FishingDarnassus',0,0,1,80,1,2,350),
('FishingBootyBay',0,2,30,80,0,2,350),
('FishingRatchet',0,2,10,80,1,2,350),
('FishingTheramore',0,0,35,80,1,2,350),
('FishingAuberdine',0,0,10,80,1,2,350),
('FishingMenethilHarbor',0,0,20,80,0,2,350),
('FishingCrystalLake',0,0,5,80,0,2,350),
('FishingValianceKeep',0,0,68,80,571,3,350),
('FishingWarsongHold',0,1,68,80,571,2,350),
('FishingWintergrasp',0,2,70,80,571,2,350),
('FishingMoakiHarbor',0,2,68,80,571,3,350),
('FishingKamagua',0,2,68,80,571,3,350),
('FishingUnupe',0,2,68,80,571,2,350),
('FishingDalaranUnderbelly',0,2,68,80,571,2,350);

-- Land/shore provenance for each named spot (creature GUID / entry or terrain tile and area).
-- CustomFishingStormwind:
--   CustomAnchor: creature 79836 / 5493 (Arnold Leland), shifted 3 yards sideways along his facing tangent.
--   CanalSupplies: creature 79835 / 5494 (Catherine Leland); land, bank or pier anchor.
--   HarborAngler: creature 49649 / 29296 (Justin Boehm); land, bank or pier anchor.
-- CustomFishingOrgrimmar:
--   CustomAnchor: creature 4673 / 3332 (Lumak); land, bank or pier anchor.
--   PondSupplies: creature 4674 / 3333 (Shankys); land, bank or pier anchor.
--   NorthernPondBank: creature 6492 / 7951 (Zas'Tysh); land, bank or pier anchor.
-- CustomFishingDalaran:
--   CustomAnchor: creature 102399 / 28742 (Marcia Chase); land, bank or pier anchor.
--   FountainElder: creature 209029 / 38294 (Elder Clearwater); land, bank or pier anchor.
-- FishingIronforge:
--   ForlornSupplies: creature 1787 / 5162 (Tansy Puddlefizz); land, bank or pier anchor.
--   ForlornBank: creature 1791 / 2943 (Ransin Donner); land, bank or pier anchor.
-- FishingThunderBluff:
--   MistrunnerPond: creature 26651 / 3028 (Kah Mistrunner); land, bank or pier anchor.
--   PondSupplies: creature 26653 / 3029 (Sewa Mistrunner); land, bank or pier anchor.
-- FishingUndercity:
--   CanalTrainer: creature 38412 / 4573 (Armand Cromwell); land, bank or pier anchor.
--   CanalSupplies: creature 31861 / 4574 (Lizbeth Cromwell); land, bank or pier anchor.
-- FishingDarnassus:
--   TempleLake: creature 46239 / 4156 (Astaia); land, bank or pier anchor.
--   LakeSupplies: creature 46481 / 4222 (Voloren); land, bank or pier anchor.
-- FishingBootyBay:
--   HemingDock: creature 6 / 2626 (Old Man Heming); land, bank or pier anchor.
--   SouthernDock: creature 649 / 4624 (Booty Bay Bruiser); land, bank or pier anchor.
-- FishingRatchet:
--   KilxxShore: creature 17440 / 3497 (Kilxx); land, bank or pier anchor.
--   WharfmasterPier: creature 14419 / 3453 (Wharfmaster Dizzywig); land, bank or pier anchor.
-- FishingTheramore:
--   NatPagleIsland: creature 8019 / 12919 (Nat Pagle); land, bank or pier anchor.
--   EasternShore: 0013840.map, area 518; dry, locally flat bank beside verified deep water.
-- FishingAuberdine:
--   BlumpBeach: creature 39062 / 10216 (Gubber Blump); land, bank or pier anchor.
--   NorthernPier: creature 37087 / 4456 (Fiora Longears); land, bank or pier anchor.
-- FishingMenethilHarbor:
--   HarborTrainer: creature 9458 / 3179 (Harold Riggs); land, bank or pier anchor.
--   HarborSupplies: creature 9562 / 3178 (Stuart Fleming); land, bank or pier anchor.
-- FishingCrystalLake:
--   BrownDock: creature 81094 / 1651 (Lee Brown); land, bank or pier anchor.
--   LakeBank: 0004932.map, area 18; dry, locally flat bank beside verified deep water.
-- FishingValianceKeep:
--   HarborWest: creature 95647 / 27393 (Valiance Keep Fisherman); land, bank or pier anchor.
--   HarborCenter: creature 95781 / 27393 (Valiance Keep Fisherman); land, bank or pier anchor.
--   HarborEast: creature 95665 / 27393 (Valiance Keep Fisherman); land, bank or pier anchor.
-- FishingWarsongHold:
--   GarroshLandingBank: 5712619.map, area 4107; dry, locally flat bank beside verified deep water.
--   GarroshLandingGuardBank: creature 110552 / 25244 (Warsong Marksman); land, bank or pier anchor.
-- FishingWintergrasp:
--   SoutheasternLakeBank: 5712327.map, area 4197; dry, locally flat bank beside verified deep water.
--   SouthernLakeBank: 5712326.map, area 4197; dry, locally flat bank beside verified deep water.
-- FishingMoakiHarbor:
--   HarborSouth: creature 106616 / 27251 (Moa'ki Fisherman); land, bank or pier anchor.
--   HarborMiddle: creature 106618 / 27251 (Moa'ki Fisherman); land, bank or pier anchor.
--   HarborNorth: creature 106635 / 27251 (Moa'ki Fisherman); land, bank or pier anchor.
-- FishingKamagua:
--   LongtuskWest: creature 103594 / 23638 (Longtusk Fisherman); land, bank or pier anchor.
--   LongtuskEast: creature 103595 / 23638 (Longtusk Fisherman); land, bank or pier anchor.
--   LongtuskMiddle: creature 103599 / 23638 (Longtusk Fisherman); land, bank or pier anchor.
-- FishingUnupe:
--   TupitShore: creature 100841 / 27190 (Tupit); land, bank or pier anchor.
--   VillageShore: creature 112382 / 27566 (Unu'pe Villager); land, bank or pier anchor.
-- FishingDalaranUnderbelly:
--   SewerAngler: creature 1823 / 33031 (Sebastian Bower); land, bank or pier anchor.
--   SewerSupplies: creature 99184 / 28707 (Angelo Pescatore); land, bank or pier anchor.

INSERT IGNORE INTO `city_life_spot`
(`hub_id`,`name`,`category`,`enabled`,`x`,`y`,`z`,`o`,`radius`,`weight`)
SELECT `hub`.`id`,`seed`.`spot_name`,'fishing',1,`seed`.`x`,`seed`.`y`,
       `seed`.`z`,`seed`.`o`,`seed`.`radius`,100
FROM `city_life_hub` AS `hub`
INNER JOIN
(
  SELECT 'CustomFishingStormwind' AS `hub_name`,'CustomAnchor' AS `spot_name`,
         -8799.571827 AS `x`,767.567690 AS `y`,96.4213 AS `z`,1.6057 AS `o`,1 AS `radius`
  UNION ALL SELECT 'CustomFishingStormwind','CanalSupplies',-8803.94,765.689,96.4213,3.1765,4
  UNION ALL SELECT 'CustomFishingStormwind','HarborAngler',-8432.93,1319.45,6.83619,3.03687,4
  UNION ALL SELECT 'CustomFishingOrgrimmar','CustomAnchor',2000.62,-4659.66,26.7387,5.3058,1
  UNION ALL SELECT 'CustomFishingOrgrimmar','PondSupplies',1995.41,-4662.32,27.1342,1.36136,4
  UNION ALL SELECT 'CustomFishingOrgrimmar','NorthernPondBank',1991.32,-4652.35,27.3799,0.10472,4
  UNION ALL SELECT 'CustomFishingDalaran','CustomAnchor',5707.31,612.214,646.791,1.69297,1
  UNION ALL SELECT 'CustomFishingDalaran','FountainElder',5699.04,617.319,646.797,0.855211,4
  UNION ALL SELECT 'FishingIronforge','ForlornSupplies',-4603.59,-1094.51,505.063,1.43117,4
  UNION ALL SELECT 'FishingIronforge','ForlornBank',-4636.22,-1110,501.411,0.628319,4
  UNION ALL SELECT 'FishingThunderBluff','MistrunnerPond',-1172.76,-69.2332,162.284,3.26377,4
  UNION ALL SELECT 'FishingThunderBluff','PondSupplies',-1177.19,-65.616,162.312,4.43314,4
  UNION ALL SELECT 'FishingUndercity','CanalTrainer',1677.83,99.0023,-61.9458,2.11185,4
  UNION ALL SELECT 'FishingUndercity','CanalSupplies',1681.1,95.7271,-62.0858,0.610865,4
  UNION ALL SELECT 'FishingDarnassus','TempleLake',9838.5,2431.55,1315.81,5.58505,4
  UNION ALL SELECT 'FishingDarnassus','LakeSupplies',9836.05,2441.62,1315.85,2.25148,4
  UNION ALL SELECT 'FishingBootyBay','HemingDock',-14451.3,471.164,4.28487,5.72468,4
  UNION ALL SELECT 'FishingBootyBay','SouthernDock',-14437.5,462.809,3.90781,3.56047,4
  UNION ALL SELECT 'FishingRatchet','KilxxShore',-971.109,-3737.66,5.89569,0.506145,4
  UNION ALL SELECT 'FishingRatchet','WharfmasterPier',-985.599,-3796.71,5.20934,0.244346,4
  UNION ALL SELECT 'FishingTheramore','NatPagleIsland',-4139.29,-4060.02,2.12276,5.044,4
  UNION ALL SELECT 'FishingTheramore','EasternShore',-3550,-4360,1.586334,3.141593,4
  UNION ALL SELECT 'FishingAuberdine','BlumpBeach',6371.34,577.405,0.162417,0.959931,4
  UNION ALL SELECT 'FishingAuberdine','NorthernPier',6482.97,729.743,8.40589,1.8326,4
  UNION ALL SELECT 'FishingMenethilHarbor','HarborTrainer',-3762.84,-723.912,8.20166,4.43314,4
  UNION ALL SELECT 'FishingMenethilHarbor','HarborSupplies',-3755.63,-720.676,8.28918,3.68849,4
  UNION ALL SELECT 'FishingCrystalLake','BrownDock',-9382.08,-117.005,58.9841,4.43314,4
  UNION ALL SELECT 'FishingCrystalLake','LakeBank',-9380,-140,58.372289,1.178097,4
  UNION ALL SELECT 'FishingValianceKeep','HarborWest',2279.11,5117.86,0.302484,4.4544,4
  UNION ALL SELECT 'FishingValianceKeep','HarborCenter',2333.35,5124.88,1.17605,1.6598,4
  UNION ALL SELECT 'FishingValianceKeep','HarborEast',2390.28,5189.7,3.37645,5.77704,4
  UNION ALL SELECT 'FishingWarsongHold','GarroshLandingBank',2755,6885,0.681212,0,4
  UNION ALL SELECT 'FishingWarsongHold','GarroshLandingGuardBank',2845.33,6834.82,0.464371,2.75762,4
  UNION ALL SELECT 'FishingWintergrasp','SoutheasternLakeBank',4355,2185,353.88862,3.141593,4
  UNION ALL SELECT 'FishingWintergrasp','SouthernLakeBank',4415,2690,354.563197,3.926991,4
  UNION ALL SELECT 'FishingMoakiHarbor','HarborSouth',2707.34,868.421,1.72821,4.43314,4
  UNION ALL SELECT 'FishingMoakiHarbor','HarborMiddle',2700.36,905.681,4.33248,1.76278,4
  UNION ALL SELECT 'FishingMoakiHarbor','HarborNorth',2718.05,955.846,0.049769,1.43117,4
  UNION ALL SELECT 'FishingKamagua','LongtuskWest',716.934,-2837.46,1.25059,1.15192,4
  UNION ALL SELECT 'FishingKamagua','LongtuskEast',831.402,-2839.78,1.15337,1.78024,4
  UNION ALL SELECT 'FishingKamagua','LongtuskMiddle',860.456,-2846.24,1.15336,1.37881,4
  UNION ALL SELECT 'FishingUnupe','TupitShore',2872.49,4062.85,2.67978,5.93412,4
  UNION ALL SELECT 'FishingUnupe','VillageShore',2922.53,4017.33,0.557778,3.56047,4
  UNION ALL SELECT 'FishingDalaranUnderbelly','SewerAngler',5743.38,672.059,612.487,5.42797,4
  UNION ALL SELECT 'FishingDalaranUnderbelly','SewerSupplies',5737.19,672.851,613.326,1.309,4
) AS `seed` ON `seed`.`hub_name`=`hub`.`name`;
