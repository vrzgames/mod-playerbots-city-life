-- Fishing Life: leave Stormwind's fishing trainer clear. Apply manually to WORLD.
-- Move only the original bundled CustomAnchor, 3 yards sideways from Arnold Leland.
-- Administrator-customized coordinates/radii and all NPC spawns remain untouched.
-- Rerunnable: the old-coordinate guard no longer matches after the first application.
-- Afterwards use .citylife reload, or restart worldserver. No rebuild is needed for this point change.
UPDATE `city_life_spot` SET `x`=-8799.571827,`y`=767.567690
WHERE `hub_id`=(SELECT `id` FROM `city_life_hub` WHERE `name`='CustomFishingStormwind' AND `map_id`=0)
  AND `name`='CustomAnchor'
  AND ABS(`x`-(-8802.57))<0.02
  AND ABS(`y`-767.463)<0.02
  AND ABS(`z`-96.4213)<0.1
  AND ABS(`radius`-1)<0.01;
