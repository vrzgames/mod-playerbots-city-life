-- City Life: give spots room around NPCs. Apply manually to WORLD after the location files.
-- CityLife.Behaviour.NpcClearance / FlightMasterClearance keep resident destinations away from
-- NPCs, mailboxes and flight masters. Most bundled spots are anchored exactly on such a spawn, so a
-- few default radii leave no room outside that clearance:
--  * the Dalaran and Orgrimmar fishing anchors (radius 1) only offered the trainer's own position;
--  * flight master spots (radius 2, 5 or 6) leave little or no room outside the 4 yd clearance.
-- Only the bundled default radii are changed; administrator-customized radii remain untouched.
-- Rerunnable: the radius guards no longer match after the first application.
-- Afterwards use .citylife reload, or restart worldserver.
UPDATE `city_life_spot` SET `radius`=4
WHERE `hub_id` IN (SELECT `id` FROM `city_life_hub`
    WHERE (`name`='CustomFishingDalaran' AND `map_id`=571) OR (`name`='CustomFishingOrgrimmar' AND `map_id`=1))
  AND `name`='CustomAnchor'
  AND `category`='fishing'
  AND ABS(`radius`-1)<0.01;

UPDATE `city_life_spot` SET `radius`=7
WHERE `category`='flight'
  AND (ABS(`radius`-2)<0.01 OR ABS(`radius`-5)<0.01 OR ABS(`radius`-6)<0.01);
