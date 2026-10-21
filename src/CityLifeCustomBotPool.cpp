/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeCustomBotPool.h"
#include "CityLifeCustomActivities.h"
#include "CityLifeCustomAppearance.h"
#include "CityLifeLegendaryPersona.h"

#include "AccountMgr.h"
#include "CharacterCache.h"
#include "Config.h"
#include "CryptoRandom.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "LifeBotReservation.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "Random.h"
#include "SharedDefines.h"
#include "World.h"
#include "WorldSession.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <memory>
#include <sstream>

namespace
{
    char const* PoolOwner = "CityLife-v1";

    uint32 PoolTime()
    {
        return static_cast<uint32>(GameTime::GetGameTime().count());
    }

    std::string RandomHex(std::size_t bytes)
    {
        std::vector<uint8> random(bytes);
        Acore::Crypto::GetRandomBytes(random);
        std::string result;
        char const* digits = "0123456789ABCDEF";
        result.reserve(bytes * 2);
        for (uint8 value : random)
        {
            result += digits[value >> 4];
            result += digits[value & 15];
        }
        return result;
    }

    std::string Upper(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return value;
    }

    bool IsSafeAccountName(std::string const& value)
    {
        return !value.empty() && value.size() <= MAX_ACCOUNT_STR &&
            std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isalnum(c); });
    }

    std::string NewCharacterName()
    {
        char const* vowels = "aeiou";
        char const* consonants = "bcdfghjklmnprstvwz";
        for (uint32 attempt = 0; attempt < 20; ++attempt)
        {
            std::string name;
            uint32 const length = urand(3, 5);
            for (uint32 i = 0; i < length; ++i)
            {
                name += consonants[urand(0, 16)];
                name += vowels[urand(0, 4)];
            }
            name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
            if (ObjectMgr::CheckPlayerName(name) != CHAR_NAME_SUCCESS ||
                !sCharacterCache->GetCharacterGuidByName(name).IsEmpty())
                continue;
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
            stmt->SetData(0, name);
            if (!CharacterDatabase.Query(stmt))
                return name;
        }
        return {};
    }
}

namespace CityLife
{
    CustomBotPool& CustomBotPool::Instance()
    {
        static CustomBotPool instance;
        return instance;
    }

    CustomBotPool::~CustomBotPool()
    {
        Shutdown();
    }

    void CustomBotPool::LoadConfig()
    {
        _customSettings.Enabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Enable", true);
        _fishingSettings.Enabled = sConfigMgr->GetOption<bool>("FishingLife.Enable", true);
        _enabled = sConfigMgr->GetOption<bool>("CityLife.Enable", true) &&
            sPlayerbotAIConfig.Enabled && (_customSettings.Enabled || _fishingSettings.Enabled);
        std::string const randomPrefix = Upper(sPlayerbotAIConfig.RandomBotAccountPrefix);
        auto loadCreationSettings = [&randomPrefix](PoolSettings& settings, std::string const& key,
            uint32 defaultMaxBots, std::string const& defaultPrefix)
        {
            settings.CreationEnabled = sConfigMgr->GetOption<bool>(key + ".Enable", true);
            settings.MaxBots = std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(key + ".MaxBots", defaultMaxBots), 0, 200);
            settings.CreateIntervalSeconds = std::clamp<uint32>(
                sConfigMgr->GetOption<uint32>(key + ".CreateIntervalSeconds", 30), 5, 3600);
            settings.AccountPrefix = Upper(
                sConfigMgr->GetOption<std::string>(key + ".AccountPrefix", defaultPrefix));
            if (settings.AccountPrefix.size() > 8 || !IsSafeAccountName(settings.AccountPrefix) ||
                (!randomPrefix.empty() && (settings.AccountPrefix.rfind(randomPrefix, 0) == 0 ||
                    randomPrefix.rfind(settings.AccountPrefix, 0) == 0)))
            {
                LOG_ERROR("module", "[CityLife] {}.AccountPrefix must contain 1-8 letters/digits "
                    "and must not overlap the Playerbots random-account prefix; creation disabled.", key);
                settings.CreationEnabled = false;
            }
            settings.NextWarningAt = 0;
        };
        loadCreationSettings(_customSettings, "CityLife.Custom.CreateMissing", 30, "citylife");
        loadCreationSettings(_fishingSettings, "FishingLife.CreateMissing", 46, "fishlife");
        if (_customSettings.Enabled && _fishingSettings.Enabled && _customSettings.CreationEnabled &&
            _fishingSettings.CreationEnabled &&
            (_customSettings.AccountPrefix.rfind(_fishingSettings.AccountPrefix, 0) == 0 ||
                _fishingSettings.AccountPrefix.rfind(_customSettings.AccountPrefix, 0) == 0))
        {
            LOG_ERROR("module", "[CityLife] Custom and FishingLife CreateMissing.AccountPrefix must not overlap; "
                "new character creation disabled for both pools.");
            _customSettings.CreationEnabled = false;
            _fishingSettings.CreationEnabled = false;
        }
        _requests.clear();
        _desiredHubs.clear();
        _lastWorkAt = 0;

        _legendaryClasses.clear();
        std::string classes = sConfigMgr->GetOption<std::string>(
            "CityLife.Custom.Legendary.Classes", "Warrior,Rogue,Paladin");
        std::replace(classes.begin(), classes.end(), ',', ' ');
        std::istringstream classList(classes);
        std::array<std::pair<char const*, uint8>, 10> const supported = {{{"WARRIOR", CLASS_WARRIOR},
            {"ROGUE", CLASS_ROGUE}, {"PALADIN", CLASS_PALADIN}, {"HUNTER", CLASS_HUNTER},
            {"PRIEST", CLASS_PRIEST}, {"DEATHKNIGHT", CLASS_DEATH_KNIGHT}, {"SHAMAN", CLASS_SHAMAN},
            {"MAGE", CLASS_MAGE}, {"WARLOCK", CLASS_WARLOCK}, {"DRUID", CLASS_DRUID}}};
        for (std::string token; classList >> token;)
        {
            std::string const value = Upper(token);
            auto found = std::find_if(supported.begin(), supported.end(), [&value](auto const& entry)
            {
                return value == entry.first || value == std::to_string(entry.second);
            });
            if (found == supported.end())
            {
                LOG_WARN("module", "[CityLife] Ignored unknown Legendary class '{}'.", token);
                continue;
            }
            if (std::find(_legendaryClasses.begin(), _legendaryClasses.end(), found->second) ==
                _legendaryClasses.end())
                _legendaryClasses.push_back(found->second);
        }
        sCityLifeLegendaryPersona.LoadConfig();
        for (auto& [guid, session] : _sessions)
            session.PersonaApplied = false;
    }

    bool CustomBotPool::LoadData()
    {
        _databaseReady = CharacterDatabase.Query("SHOW TABLES LIKE 'city_life_custom_bot'") != nullptr;
        if (!_databaseReady)
        {
            if (_enabled)
                LOG_WARN("module", "[CityLife] Apply data/sql/manual/characters_city_life_custom_bots.sql "
                    "to CHARACTERS for automatic Custom/Fishing bot creation.");
            return false;
        }
        QueryResult count = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM city_life_custom_bot WHERE owner='CityLife-v1'");
        if (!count)
        {
            _databaseReady = false;
            return false;
        }
        uint64 const expectedRecords = count->Fetch()[0].Get<uint64>();
        QueryResult result = CharacterDatabase.Query(
            "SELECT id,account_id,character_guid,account_name,owner_token,hub_name,hub_slot,target_level,"
            "race,class,role,initialized FROM city_life_custom_bot WHERE owner='CityLife-v1' ORDER BY id");
        if (!result && expectedRecords != 0)
        {
            _databaseReady = false;
            LOG_ERROR("module", "[CityLife] Custom registry read failed; "
                "retaining known ownership and creation quota.");
            return false;
        }
        std::vector<Record> records;
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                Record entry;
                entry.Id = fields[0].Get<uint32>();
                entry.AccountId = fields[1].Get<uint32>();
                entry.Guid = fields[2].Get<uint32>();
                entry.AccountName = fields[3].Get<std::string>();
                entry.OwnerToken = fields[4].Get<std::string>();
                entry.HubName = fields[5].Get<std::string>();
                entry.Slot = fields[6].Get<uint32>();
                entry.TargetLevel = fields[7].Get<uint8>();
                entry.Race = fields[8].Get<uint8>();
                entry.Class = fields[9].Get<uint8>();
                entry.Role = static_cast<CustomRole>(fields[10].Get<uint8>());
                entry.Initialized = fields[11].Get<uint8>() != 0;
                bool const tokenValid = entry.OwnerToken.size() == 32 &&
                    std::all_of(entry.OwnerToken.begin(), entry.OwnerToken.end(),
                        [](unsigned char c) { return std::isxdigit(c); });
                if (!IsSafeAccountName(entry.AccountName) || !tokenValid || entry.TargetLevel < 1 ||
                    entry.TargetLevel > 80 || !sObjectMgr->GetPlayerInfo(entry.Race, entry.Class) ||
                    entry.Role == CustomRole::None || entry.Role > CustomRole::Fishing)
                {
                    entry.Blocked = true;
                    LOG_ERROR("module", "[CityLife] Invalid internal Custom bot registry row {}; left untouched.",
                        entry.Id);
                }
                if (Record const* old = FindRecord(entry.Guid))
                {
                    entry.AccountVerified = old->AccountVerified;
                    entry.AccountQueuedAt = old->AccountQueuedAt;
                    entry.RetryAt = old->RetryAt;
                    entry.Initialized = entry.Initialized || old->Initialized;
                }
                records.push_back(std::move(entry));
            } while (result->NextRow());
        }
        _records.swap(records);
        return true;
    }

    void CustomBotPool::SyncHubs(std::vector<Hub> const& hubs)
    {
        _desiredHubs.clear();
        if (!_enabled)
        {
            _requests.clear();
            return;
        }
        for (Hub const& hub : hubs)
        {
            if (!IsRoleEnabled(hub.Custom) || !hub.Enabled || !hub.ConfiguredPopulation)
                continue;
            Request request;
            request.HubName = hub.Name;
            request.MinLevel = hub.MinLevel;
            request.MaxLevel = hub.MaxLevel;
            request.Team = hub.Team;
            request.Role = hub.Custom;
            request.Population = hub.ConfiguredPopulation;
            request.Until = std::numeric_limits<uint32>::max();
            _desiredHubs[hub.Name] = request;
        }
        for (auto request = _requests.begin(); request != _requests.end();)
        {
            auto const desired = _desiredHubs.find(request->first);
            if (desired == _desiredHubs.end())
            {
                request = _requests.erase(request);
                continue;
            }
            uint32 const until = request->second.Until;
            request->second = desired->second;
            request->second.Until = until;
            ++request;
        }
    }

    CustomBotPool::PoolSettings& CustomBotPool::GetPoolSettings(CustomRole role)
    {
        return role == CustomRole::Fishing ? _fishingSettings : _customSettings;
    }

    CustomBotPool::PoolSettings const& CustomBotPool::GetPoolSettings(CustomRole role) const
    {
        return role == CustomRole::Fishing ? _fishingSettings : _customSettings;
    }

    bool CustomBotPool::IsRoleEnabled(CustomRole role) const
    {
        return _enabled && role != CustomRole::None && GetPoolSettings(role).Enabled;
    }

    uint32 CustomBotPool::CountRecords(CustomRole role) const
    {
        // Keep old CityLife-v1 fishing identities in the fishing quota, regardless of their account prefix.
        bool const fishing = role == CustomRole::Fishing;
        return static_cast<uint32>(std::count_if(_records.begin(), _records.end(), [fishing](Record const& record)
        {
            return (record.Role == CustomRole::Fishing) == fishing;
        }));
    }

    CustomBotPool::Record* CustomBotPool::FindRecord(uint32 guidLow)
    {
        if (!guidLow)
            return nullptr;
        auto found = std::find_if(_records.begin(), _records.end(),
            [guidLow](Record const& entry) { return entry.Guid == guidLow; });
        return found == _records.end() ? nullptr : &*found;
    }

    CustomBotPool::Record const* CustomBotPool::FindRecord(uint32 guidLow) const
    {
        if (!guidLow)
            return nullptr;
        auto found = std::find_if(_records.begin(), _records.end(),
            [guidLow](Record const& entry) { return entry.Guid == guidLow; });
        return found == _records.end() ? nullptr : &*found;
    }

    bool CustomBotPool::IsManagedBot(uint32 guidLow) const
    {
        Record const* record = FindRecord(guidLow);
        return record && !record->Blocked && record->AccountVerified;
    }

    bool CustomBotPool::IsManagedLegendaryBot(uint32 guidLow)
    {
        Record* record = FindRecord(guidLow);
        if (!record || record->Blocked || record->Role != CustomRole::Legendary ||
            !record->Guid || !record->AccountId ||
            sCharacterCache->GetCharacterAccountIdByGuid(ObjectGuid::Create<HighGuid::Player>(guidLow)) !=
                record->AccountId)
            return false;
        // An offline guild member must pass the same account ownership check as an online bot.
        return VerifyAccount(*record);
    }

    bool CustomBotPool::IsAssignedTo(uint32 guidLow, Hub const& hub) const
    {
        Record const* record = FindRecord(guidLow);
        auto session = _sessions.find(guidLow);
        if (record && record->Role == CustomRole::Legendary &&
            (session == _sessions.end() || !session->second.PersonaApplied))
            return false;
        return record && !record->Blocked && record->AccountVerified && record->Initialized &&
            record->HubName == hub.Name &&
            record->Role == hub.Custom && record->TargetLevel >= hub.MinLevel && record->TargetLevel <= hub.MaxLevel &&
            IsDesired(*record);
    }

    uint32 CustomBotPool::GetSlot(uint32 guidLow) const
    {
        Record const* record = FindRecord(guidLow);
        return record ? record->Slot : 0;
    }

    bool CustomBotPool::Matches(Record const& record, Request const& request) const
    {
        TeamSide const team = Player::TeamIdForRace(record.Race) == TEAM_ALLIANCE ?
            TeamSide::Alliance : TeamSide::Horde;
        return IsRoleEnabled(request.Role) && !record.Blocked &&
            record.HubName == request.HubName && record.Role == request.Role &&
            record.TargetLevel >= request.MinLevel && record.TargetLevel <= request.MaxLevel &&
            (request.Team == TeamSide::Any || request.Team == team);
    }

    bool CustomBotPool::RequestBot(Hub const& hub)
    {
        if (!IsRoleEnabled(hub.Custom) || !_databaseReady || _stopping ||
            !hub.Enabled || !hub.ConfiguredPopulation)
            return false;
        Request request;
        request.HubName = hub.Name;
        request.MinLevel = hub.MinLevel;
        request.MaxLevel = hub.MaxLevel;
        request.Team = hub.Team;
        request.Role = hub.Custom;
        request.Population = hub.ConfiguredPopulation;
        request.Until = PoolTime() + 120;
        _requests[hub.Name] = request;
        _desiredHubs[hub.Name] = request;
        return true;
    }

    bool CustomBotPool::IsDesired(Record const& record) const
    {
        if (!_enabled)
            return false;
        auto const hub = _desiredHubs.find(record.HubName);
        if (hub == _desiredHubs.end() || !Matches(record, hub->second))
            return false;
        uint32 earlierSlots = 0;
        for (Record const& other : _records)
            if (other.Slot < record.Slot && Matches(other, hub->second))
                ++earlierSlots;
        return earlierSlots < hub->second.Population;
    }

    bool CustomBotPool::ReserveRecord(Request const& request)
    {
        PoolSettings& settings = GetPoolSettings(request.Role);
        if (!IsRoleEnabled(request.Role) || !settings.CreationEnabled ||
            CountRecords(request.Role) >= settings.MaxBots || PoolTime() < settings.NextCreateAt)
            return false;
        uint8 const targetLevel = request.Role == CustomRole::Legendary ? request.MaxLevel : request.MinLevel;
        if (targetLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            settings.NextCreateAt = PoolTime() + settings.CreateIntervalSeconds;
            return false;
        }
        Record record;
        record.HubName = request.HubName;
        record.TargetLevel = targetLevel;
        record.Role = request.Role;
        std::vector<std::pair<uint8, uint8>> combinations;
        std::array<uint8, 10> const races = {RACE_HUMAN, RACE_ORC, RACE_DWARF, RACE_NIGHTELF,
            RACE_UNDEAD_PLAYER, RACE_TAUREN, RACE_GNOME, RACE_TROLL, RACE_BLOODELF, RACE_DRAENEI};
        std::array<uint8, 9> const classes = {CLASS_WARRIOR, CLASS_ROGUE, CLASS_HUNTER, CLASS_PALADIN,
            CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_DRUID};
        std::vector<uint8> const allowedClasses = request.Role == CustomRole::Legendary ?
            _legendaryClasses : std::vector<uint8>(classes.begin(), classes.end());
        for (uint8 race : races)
        {
            TeamSide const team = Player::TeamIdForRace(race) == TEAM_ALLIANCE ?
                TeamSide::Alliance : TeamSide::Horde;
            if ((request.Team != TeamSide::Any && request.Team != team) ||
                (sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK) & (1u << (race - 1))) ||
                (sWorld->getIntConfig(CONFIG_EXPANSION) < EXPANSION_THE_BURNING_CRUSADE &&
                    (race == RACE_BLOODELF || race == RACE_DRAENEI)))
                continue;
            for (uint8 classId : allowedClasses)
                if (!(sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK) & (1u << (classId - 1))) &&
                    sObjectMgr->GetPlayerInfo(race, classId) &&
                    (request.Role != CustomRole::Legendary ||
                        sCityLifeCustomAppearance.CanCreateLegendary(classId, record.TargetLevel)))
                    combinations.emplace_back(race, classId);
        }
        if (combinations.empty())
        {
            settings.NextCreateAt = PoolTime() + settings.CreateIntervalSeconds;
            return false;
        }
        if (request.Role == CustomRole::Legendary)
        {
            // Only new identities use this class mix; registered characters are never recreated.
            uint8 selectedClass = 0;
            std::size_t minimum = std::numeric_limits<std::size_t>::max();
            for (uint8 classId : _legendaryClasses)
            {
                if (std::none_of(combinations.begin(), combinations.end(), [classId](auto const& value)
                    { return value.second == classId; }))
                    continue;
                std::size_t count = std::count_if(_records.begin(), _records.end(), [&](Record const& other)
                {
                    return !other.Blocked && other.Role == CustomRole::Legendary &&
                        other.HubName == request.HubName && other.Class == classId;
                });
                if (count < minimum)
                {
                    minimum = count;
                    selectedClass = classId;
                }
            }
            std::erase_if(combinations, [selectedClass](auto const& value) { return value.second != selectedClass; });
        }
        auto const& combination = combinations[urand(0, static_cast<uint32>(combinations.size() - 1))];
        record.Race = combination.first;
        record.Class = combination.second;
        for (Record const& entry : _records)
            if (entry.HubName == record.HubName)
                record.Slot = std::max(record.Slot, entry.Slot + 1);
        record.OwnerToken = RandomHex(16);
        record.AccountName = settings.AccountPrefix + RandomHex(4);
        if (AccountMgr::GetId(record.AccountName))
            return false;
        std::string hubName = record.HubName;
        CharacterDatabase.EscapeString(hubName);
        CharacterDatabase.DirectExecute(
            "INSERT INTO city_life_custom_bot (owner,account_name,owner_token,hub_name,hub_slot,target_level,"
            "race,class,role) VALUES ('{}','{}','{}','{}',{},{},{},{},{})",
            PoolOwner, record.AccountName, record.OwnerToken, hubName, record.Slot,
            record.TargetLevel, record.Race, record.Class, static_cast<uint8>(record.Role));
        QueryResult saved = CharacterDatabase.Query(
            "SELECT id FROM city_life_custom_bot WHERE owner='CityLife-v1' AND account_name='{}' AND owner_token='{}'",
            record.AccountName, record.OwnerToken);
        if (!saved)
            return false;
        record.Id = saved->Fetch()[0].Get<uint32>();
        _records.push_back(std::move(record));
        settings.NextCreateAt = PoolTime() + settings.CreateIntervalSeconds;
        return true;
    }

    bool CustomBotPool::VerifyAccount(Record& record)
    {
        if (record.AccountVerified)
            return true;
        QueryResult account = LoginDatabase.Query("SELECT id,email FROM account WHERE username='{}'",
            record.AccountName);
        if (!account)
        {
            if (record.AccountId)
            {
                record.Blocked = true;
                LOG_ERROR("module", "[CityLife] Managed account missing for registry row {}; no replacement takeover.",
                    record.Id);
                return false;
            }
            uint32 const now = PoolTime();
            if (record.AccountQueuedAt && now < record.AccountQueuedAt + 60)
                return false;
            // The password is unpredictable and intentionally never retained or logged.
            AccountOpResult const result = sAccountMgr->CreateAccount(record.AccountName, RandomHex(8),
                "CITYLIFE+" + record.OwnerToken + "@INVALID");
            record.AccountQueuedAt = now;
            if (result != AOR_OK && result != AOR_NAME_ALREADY_EXIST)
            {
                record.RetryAt = now + 60;
                LOG_ERROR("module", "[CityLife] Could not create managed account for row {} (result {}).",
                    record.Id, static_cast<uint32>(result));
            }
            return false;
        }
        Field* fields = account->Fetch();
        uint32 const accountId = fields[0].Get<uint32>();
        std::string const expected = "CITYLIFE+" + Upper(record.OwnerToken) + "@INVALID";
        if ((record.AccountId && record.AccountId != accountId) || Upper(fields[1].Get<std::string>()) != expected ||
            sPlayerbotAIConfig.IsInRandomAccountList(accountId))
        {
            record.Blocked = true;
            LOG_ERROR("module", "[CityLife] Ownership check failed for Custom registry row {}; account untouched.",
                record.Id);
            return false;
        }
        record.AccountId = accountId;
        record.AccountVerified = true;
        CharacterDatabase.Execute(
            "UPDATE city_life_custom_bot SET account_id={} WHERE id={} AND owner='CityLife-v1'",
            accountId, record.Id);
        return true;
    }

    bool CustomBotPool::CreateCharacter(Record& record)
    {
        // A previous commit may have succeeded even if its confirmation read failed.
        // Consult the durable binding before allocating another character GUID.
        QueryResult existing = CharacterDatabase.Query(
            "SELECT character_guid FROM city_life_custom_bot WHERE id={} AND owner='CityLife-v1'", record.Id);
        if (!existing)
        {
            record.RetryAt = PoolTime() + 30;
            return false;
        }
        uint32 const existingGuid = existing->Fetch()[0].Get<uint32>();
        if (existingGuid)
            return ConfirmCharacter(record, existingGuid);
        std::string const name = NewCharacterName();
        if (name.empty())
        {
            record.RetryAt = PoolTime() + 60;
            return false;
        }
        auto session = std::make_unique<WorldSession>(record.AccountId, "", 0, nullptr, SEC_PLAYER,
            EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), sWorld->GetDefaultDbcLocale(), 0, false, false, 0);
        auto player = std::make_unique<Player>(session.get());
        player->GetMotionMaster()->Initialize();
        CharacterCreateInfo createInfo(name, record.Race, record.Class, GENDER_MALE);
        uint32 const guid = sObjectMgr->GetGenerator<HighGuid::Player>().Generate();
        if (!player->Create(guid, &createInfo))
        {
            player->CleanupsBeforeDelete();
            record.RetryAt = PoolTime() + 60;
            return false;
        }
        player->setCinematic(2);
        player->SetAtLoginFlag(AT_LOGIN_NONE);
        // Only this newly allocated character is leveled; existing characters are never changed here.
        if (player->GetLevel() != record.TargetLevel)
            player->GiveLevel(record.TargetLevel);
        player->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        player->SaveToDB(trans, true, false);
        trans->Append("UPDATE city_life_custom_bot SET account_id={},character_guid={} "
            "WHERE id={} AND owner='CityLife-v1' AND character_guid=0", record.AccountId, guid, record.Id);
        // SaveToDB uses async-only core statements; await the worker before checking the saved identity.
        bool const committed = CharacterDatabase.AsyncCommitTransaction(trans).m_future.get();
        player->CleanupsBeforeDelete();
        if (!committed || !ConfirmCharacter(record, guid))
        {
            record.RetryAt = PoolTime() + 30;
            return false;
        }
        LOG_INFO("module", "[CityLife] Created dedicated Custom character {} for {} slot {} (level {}).",
            name, record.HubName, record.Slot + 1, record.TargetLevel);
        return true;
    }

    bool CustomBotPool::ConfirmCharacter(Record& record, uint32 guidLow)
    {
        QueryResult saved = CharacterDatabase.Query(
            "SELECT c.guid,c.account,c.name,c.gender,c.race,c.class,c.level,r.initialized "
            "FROM city_life_custom_bot r INNER JOIN characters c ON c.guid=r.character_guid "
            "WHERE r.id={} AND r.owner='CityLife-v1' AND r.character_guid={}", record.Id, guidLow);
        if (!saved)
        {
            record.RetryAt = PoolTime() + 30;
            LOG_WARN("module", "[CityLife] Custom character creation for registry row {} "
                "not confirmed; retry deferred.", record.Id);
            return false;
        }
        Field* fields = saved->Fetch();
        if (fields[1].Get<uint32>() != record.AccountId || fields[4].Get<uint8>() != record.Race ||
            fields[5].Get<uint8>() != record.Class || fields[6].Get<uint8>() != record.TargetLevel)
        {
            record.Blocked = true;
            LOG_ERROR("module", "[CityLife] Invalid committed Custom character binding for registry row {}.",
                record.Id);
            return false;
        }
        record.Guid = fields[0].Get<uint32>();
        record.Initialized = fields[7].Get<uint8>() != 0;
        sCharacterCache->AddCharacterCacheEntry(ObjectGuid::Create<HighGuid::Player>(record.Guid), record.AccountId,
            fields[2].Get<std::string>(), fields[3].Get<uint8>(), record.Race, record.Class, record.TargetLevel);
        record.RetryAt = PoolTime() + 1;
        return true;
    }

    bool CustomBotPool::StartLogin(Record& record)
    {
        ObjectGuid const guid = ObjectGuid::Create<HighGuid::Player>(record.Guid);
        if (_sessions.count(record.Guid) || ObjectAccessor::FindConnectedPlayer(guid))
            return false;
        QueryResult character = CharacterDatabase.Query(
            "SELECT c.account,c.level,c.race,c.class,r.initialized FROM characters c "
            "INNER JOIN city_life_custom_bot r ON r.character_guid=c.guid "
            "WHERE c.guid={} AND r.id={} AND r.owner='CityLife-v1'", record.Guid, record.Id);
        if (!character)
        {
            record.RetryAt = PoolTime() + 5;
            return false;
        }
        Field* fields = character->Fetch();
        if (fields[0].Get<uint32>() != record.AccountId || fields[1].Get<uint8>() != record.TargetLevel ||
            fields[2].Get<uint8>() != record.Race || fields[3].Get<uint8>() != record.Class)
        {
            record.Blocked = true;
            LOG_ERROR("module", "[CityLife] Custom character changed outside its registry row {}; no reset performed.",
                record.Id);
            return false;
        }
        // A confirmed previous initialization must never be randomized again on a retry.
        record.Initialized = fields[4].Get<uint8>() != 0;
        auto holder = std::make_shared<LoginQueryHolder>(record.AccountId, guid);
        if (!holder->Initialize())
        {
            record.RetryAt = PoolTime() + 30;
            return false;
        }
        Session entry;
        entry.Value = std::make_unique<WorldSession>(record.AccountId, "", 0, nullptr, SEC_PLAYER,
            EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), sWorld->GetDefaultDbcLocale(), 0, false, false, 0);
        entry.StartedAt = PoolTime();
        WorldSession* session = entry.Value.get();
        uint32 const guidLow = record.Guid;
        _sessions.emplace(guidLow, std::move(entry));
        session->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete(
            [this, guidLow](SQLQueryHolderBase const& value)
            {
                auto found = _sessions.find(guidLow);
                if (found == _sessions.end())
                    return;
                Session& current = found->second;
                Record const* record = FindRecord(guidLow);
                if (_stopping || !_enabled || !record || !IsDesired(*record))
                {
                    current.Failed = true;
                    return;
                }
                current.Value->HandlePlayerLoginFromDB(static_cast<LoginQueryHolder const&>(value));
                Player* player = current.Value->GetPlayer();
                if (!player)
                {
                    current.Failed = true;
                    return;
                }
                current.Pending = false;
                // The general holder login queues an asynchronous baseline SaveToDB. It could race
                // the first gear/skills initialization below, so owned role characters use this path.
                if (!GET_PLAYERBOT_AI(player))
                    sPlayerbotsMgr.AddPlayerbotData(player, true);
                playerBots[player->GetGUID()] = player;
                OnBotLoginInternal(player);
                sCityLifeCustomActivities.RecoverPending();
                InitializeNewCharacter(player);
                if (!current.Failed)
                {
                    if (PlayerbotAI* ai = GET_PLAYERBOT_AI(player))
                    {
                        ai->ResetStrategies(false);
                        ai->AllowActivity(ALL_ACTIVITY, true);
                        ai->SetNextCheckDelay(2000);
                    }
                    HoldIdle(player);
                }
                FinishLogin(guidLow);
            });
        return true;
    }

    bool CustomBotPool::AdvanceRecord(Record& record)
    {
        PoolSettings const& settings = GetPoolSettings(record.Role);
        if (record.Blocked || record.RetryAt > PoolTime() || !IsDesired(record) ||
            (!record.Guid && (!settings.CreationEnabled || !settings.MaxBots)) || !VerifyAccount(record))
            return false;
        if (!record.Guid)
            return CreateCharacter(record);
        return StartLogin(record);
    }

    void CustomBotPool::OnBotLoginInternal(Player* const bot)
    {
        bot->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
        if (sCityLifeCustomAppearance.HasPendingRecovery(bot))
            sCityLifeCustomAppearance.RecoverOnLogin(bot);
    }

    void CustomBotPool::InitializeNewCharacter(Player* bot)
    {
        Record* record = FindRecord(bot->GetGUID().GetCounter());
        if (!record || record->Blocked || !IsDesired(*record))
            return;
        if (sCityLifeCustomAppearance.HasPendingRecovery(bot) &&
            !sCityLifeCustomAppearance.RecoverOnLogin(bot))
            return;
        bot->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
        if (!record->Initialized)
        {
            if (record->TargetLevel > 1)
            {
                PlayerbotFactory factory(bot, record->TargetLevel, ITEM_QUALITY_EPIC);
                factory.InitSkills();
                factory.InitClassSpells();
                factory.InitAvailableSpells();
                factory.InitTalentsTree();
                factory.InitBags();
                factory.InitEquipment(false);
                factory.InitAmmo();
                factory.InitFood();
            }
            if (record->Role == CustomRole::Fishing)
            {
                bot->learnSpell(7620, false);
                if (!bot->GetSkillValue(SKILL_FISHING))
                    bot->SetSkill(SKILL_FISHING, 1, 1, 75);
            }
            bot->SetHealth(bot->GetMaxHealth());
            bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            bot->SaveToDB(trans, false, false);
            trans->Append("UPDATE city_life_custom_bot SET initialized=1 "
                "WHERE id={} AND owner='CityLife-v1' AND character_guid={}", record->Id, record->Guid);
            bool const committed = CharacterDatabase.AsyncCommitTransaction(trans).m_future.get();
            QueryResult confirmed = CharacterDatabase.Query(
                "SELECT initialized FROM city_life_custom_bot "
                "WHERE id={} AND owner='CityLife-v1' AND character_guid={}", record->Id, record->Guid);
            if (committed && confirmed && confirmed->Fetch()[0].Get<uint8>() != 0)
                record->Initialized = true;
            else
            {
                // Item update flags were consumed by SaveToDB. Reload a durable baseline before retrying.
                // Saving this uncertain in-memory inventory on logout could publish an incomplete attempt.
                auto const session = _sessions.find(record->Guid);
                if (session != _sessions.end())
                {
                    session->second.DiscardChanges = true;
                    session->second.Failed = true;
                }
                record->RetryAt = PoolTime() + 30;
                LOG_WARN("module", "[CityLife] Custom initialization for registry row {} not confirmed; "
                    "session discarded for a clean retry.", record->Id);
            }
        }
        if (record->Initialized)
            ApplyLegendaryPersona(bot);
    }

    void CustomBotPool::ApplyLegendaryPersona(Player* player)
    {
        if (!_enabled || !player || !player->IsInWorld() || player->IsBeingTeleported() ||
            sCityLifeCustomAppearance.HasPendingRecovery(player))
            return;
        uint32 const guidLow = player->GetGUID().GetCounter();
        auto session = _sessions.find(guidLow);
        Record* record = FindRecord(guidLow);
        if (session == _sessions.end() || session->second.Pending || session->second.Failed ||
            session->second.PersonaApplied || !record || !record->Initialized ||
            record->Role != CustomRole::Legendary || !IsDesired(*record) || !IsManagedLegendaryBot(guidLow) ||
            player->GetSession()->GetAccountId() != record->AccountId)
            return;
        auto discard = [&]()
        {
            // Save APIs consume update flags; reload a durable baseline after a failed persona save.
            session->second.DiscardChanges = true;
            session->second.Failed = true;
            record->RetryAt = PoolTime() + 30;
            LOG_WARN("module", "[CityLife] Legendary persona for registry row {} could not be saved; "
                "session discarded for a clean retry.", record->Id);
        };
        if (!sCityLifeLegendaryPersona.Apply(player))
        {
            discard();
            return;
        }
        CharacterDatabaseTransaction save = CharacterDatabase.BeginTransaction();
        player->SaveToDB(save, false, false);
        if (!CharacterDatabase.AsyncCommitTransaction(save).m_future.get())
        {
            discard();
            return;
        }
        session->second.PersonaApplied = true;
    }

    void CustomBotPool::HoldIdle(Player* player) const
    {
        if (!player || !player->IsInWorld() || player->IsBeingTeleported() || player->IsInCombat() ||
            player->GetGroup() || PlayerbotsLife::IsReserved(player->GetGUID().GetCounter()))
            return;
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        if (!ai || ai->HasGameClientMaster())
            return;
        ai->ChangeStrategy("-stay,-follow,+passive,-grind,-rpg,-new rpg,-travel,-move random,-collision,"
            "-move from group,-quest,-loot,-gather,-duel,-pvp,-bg,-lfg,-start duel", BOT_STATE_NON_COMBAT);
        player->GetMotionMaster()->MoveIdle();
    }

    void CustomBotPool::FinishLogin(uint32 guidLow)
    {
        Record* record = FindRecord(guidLow);
        if (record)
            record->RetryAt = PoolTime() + 30;
    }

    void CustomBotPool::OnLogin(Player* player)
    {
        if (player && IsManagedBot(player->GetGUID().GetCounter()))
            player->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }

    void CustomBotPool::UpdateSessionsOwned()
    {
        for (auto it = _sessions.begin(); it != _sessions.end();)
        {
            Session& entry = it->second;
            entry.Value->ProcessQueryCallbacks();
            if (entry.Pending && PoolTime() > entry.StartedAt + 120)
                entry.Failed = true;
            Player* player = entry.Value->GetPlayer();
            Record const* record = FindRecord(it->first);
            if ((!record || !IsDesired(*record)) &&
                (!player || (!PlayerbotsLife::IsReserved(it->first) && !player->IsInCombat() &&
                    !player->IsBeingTeleported() && !player->GetGroup() && !player->InBattleground())))
                entry.Failed = true;
            if (entry.Failed || (!entry.Pending && !player))
            {
                RemoveFromPlayerbotsMap(ObjectGuid::Create<HighGuid::Player>(it->first));
                if (entry.DiscardChanges)
                    entry.Value->LogoutPlayer(false);
                if (Record* record = FindRecord(it->first))
                    record->RetryAt = PoolTime() + 30;
                it = _sessions.erase(it);
                continue;
            }
            if (player && !entry.Pending)
            {
                if (Record* pendingInitialization = FindRecord(it->first))
                {
                    if (!pendingInitialization->Initialized &&
                        !sCityLifeCustomAppearance.HasPendingRecovery(player))
                        InitializeNewCharacter(player);
                    else if (pendingInitialization->Initialized && !entry.PersonaApplied &&
                        pendingInitialization->Role == CustomRole::Legendary)
                        ApplyLegendaryPersona(player);
                }
                if (entry.Failed)
                    continue;
                if (player->IsBeingTeleported())
                {
                    if (PlayerbotAI* ai = GET_PLAYERBOT_AI(player))
                        ai->HandleTeleportAck();
                }
                else if (player->IsInWorld())
                    HandleBotPackets(entry.Value.get());
            }
            ++it;
        }
    }

    void CustomBotPool::Update()
    {
        UpdateSessionsOwned();
        uint32 const now = PoolTime();
        if (_lastWorkAt == now)
            return;
        _lastWorkAt = now;
        for (auto const& [guid, session] : _sessions)
            if (!session.Pending && !session.Failed)
                HoldIdle(session.Value->GetPlayer());
        if (!_enabled || !_databaseReady || _stopping)
            return;
        for (auto request = _requests.begin(); request != _requests.end();)
        {
            if (request->second.Until < now || !_desiredHubs.count(request->first))
            {
                request = _requests.erase(request);
                continue;
            }
            uint32 matching = 0;
            for (Record& record : _records)
            {
                if (!Matches(record, request->second))
                    continue;
                ++matching;
                if ((!record.Guid || !_sessions.count(record.Guid)) &&
                    (!record.Guid || !ObjectAccessor::FindConnectedPlayer(
                        ObjectGuid::Create<HighGuid::Player>(record.Guid))))
                    AdvanceRecord(record);
            }
            PoolSettings const& settings = GetPoolSettings(request->second.Role);
            if (settings.CreationEnabled && matching < request->second.Population &&
                CountRecords(request->second.Role) < settings.MaxBots && now >= settings.NextCreateAt)
                ReserveRecord(request->second);
            ++request;
        }
        for (CustomRole role : {CustomRole::Legendary, CustomRole::Fishing})
        {
            PoolSettings& settings = GetPoolSettings(role);
            if (IsRoleEnabled(role) && settings.CreationEnabled && CountRecords(role) >= settings.MaxBots &&
                now >= settings.NextWarningAt)
            {
                settings.NextWarningAt = now + 5 * MINUTE;
                LOG_DEBUG("module", "[CityLife] {} managed pool reached MaxBots={}; "
                    "no additional characters created.", role == CustomRole::Fishing ? "FishingLife" : "Custom",
                    settings.MaxBots);
            }
        }
    }

    void CustomBotPool::Shutdown()
    {
        _stopping = true;
        _requests.clear();
        playerBots.clear();
        _sessions.clear();
    }
}
