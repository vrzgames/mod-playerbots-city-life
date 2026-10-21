-- City Life owns only rows with owner='CityLife-v1' in this internal registry.
-- Apply to the CHARACTERS database before enabling automatic missing-bot creation.
-- Operational settings belong in mod_playerbots_city_life.conf, not in this table.
-- Accounts/characters are reused across restarts and are never deleted by this script.
CREATE TABLE IF NOT EXISTS `city_life_custom_bot` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `owner` VARCHAR(16) NOT NULL DEFAULT 'CityLife-v1',
  `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `character_guid` INT UNSIGNED NOT NULL DEFAULT 0,
  `account_name` VARCHAR(32) NOT NULL,
  `owner_token` CHAR(32) NOT NULL,
  `hub_name` VARCHAR(64) NOT NULL,
  `hub_slot` INT UNSIGNED NOT NULL DEFAULT 0,
  `target_level` TINYINT UNSIGNED NOT NULL,
  `race` TINYINT UNSIGNED NOT NULL,
  `class` TINYINT UNSIGNED NOT NULL,
  `role` TINYINT UNSIGNED NOT NULL,
  `initialized` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_name` (`account_name`),
  UNIQUE KEY `hub_slot` (`owner`, `hub_name`, `hub_slot`),
  KEY `character_guid` (`character_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
