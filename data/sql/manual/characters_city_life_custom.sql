-- Apply to the CHARACTERS database, never WORLD.
-- Keep this table until every borrowed outfit has been recovered. Rows are transient recovery data.
-- Re-running this file preserves in-progress equipment journals.
CREATE TABLE IF NOT EXISTS `city_life_custom_equipment` (
  `bot_guid` int unsigned NOT NULL,
  `slot` tinyint unsigned NOT NULL,
  `original_guid` int unsigned NOT NULL DEFAULT 0,
  `temporary_guid` int unsigned NOT NULL DEFAULT 0,
  `temporary_entry` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`bot_guid`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
