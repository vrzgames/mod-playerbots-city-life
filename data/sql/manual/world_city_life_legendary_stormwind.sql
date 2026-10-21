-- City Life: move the original Stormwind Legendary group closer to the plaza tree.
-- Apply manually to WORLD after world_city_life_custom.sql.
-- In-game GM GPS, map 0 / phase 1, captured 2026-10-21; all slots share this facing.
-- Only the old default anchor is migrated. Customized anchors and other cities are preserved.
-- Rerunnable: the old-coordinate guard no longer matches after the first application.
-- Use .citylife reload or restart worldserver afterwards; this point change needs no rebuild.
UPDATE `city_life_spot` SET `x`=-8824.208,`y`=628.28284,`z`=93.914894,`o`=3.7331078
WHERE `hub_id`=(SELECT `id` FROM `city_life_hub` WHERE `name`='CustomLegendaryStormwind' AND `map_id`=0)
  AND `name`='CustomAnchor'
  AND ABS(`x`-(-8833.38))<0.02
  AND ABS(`y`-628.628)<0.02
  AND ABS(`z`-94.0066)<0.1
  AND ABS(`o`-1.06535)<0.01
  AND ABS(`radius`-5)<0.01;
