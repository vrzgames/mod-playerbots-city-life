/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeLegendaryPersona.h"
#include "CityLifeCustomBotPool.h"

#include "AchievementMgr.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "GameTime.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryHolder.h"
#include "ReputationMgr.h"
#include "UpdateFields.h"
#include "WorldSession.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <unordered_set>

namespace
{
    constexpr uint32 DungeonCategory = 168;
    constexpr uint32 GeneralCategory = 92;
    constexpr uint32 ExplorationCategory = 97;
    constexpr uint32 HonorTargetSalt = 0x484B4C46u;
    constexpr uint32 AchievementTargetSalt = 0x41434C46u;

    uint32 RootCategory(uint32 categoryId)
    {
        // Invalid or cyclic category data must not turn initialization into an endless loop.
        for (uint32 depth = 0; depth < 16; ++depth)
        {
            AchievementCategoryEntry const* entry = sAchievementCategoryStore.LookupEntry(categoryId);
            if (!entry)
                return 0;
            if (entry->parentCategory < 0)
                return categoryId;
            categoryId = static_cast<uint32>(entry->parentCategory);
        }
        return 0;
    }

    bool IsSafeHistoryAchievement(AchievementEntry const* entry)
    {
        if (!entry || !entry->points ||
            (entry->flags & (ACHIEVEMENT_FLAG_COUNTER | ACHIEVEMENT_FLAG_HIDDEN |
                ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL)))
            return false;
        // Provision history without creating reward mail or tradable reward items.
        AchievementReward const* reward = sAchievementMgr->GetAchievementReward(entry);
        return !reward || (!reward->sender && !reward->itemId && !reward->mailTemplate);
    }

    bool MatchesFaction(Player const* player, AchievementEntry const* entry)
    {
        int32 const faction = player->GetTeamId() == TEAM_ALLIANCE ?
            ACHIEVEMENT_FACTION_ALLIANCE : ACHIEVEMENT_FACTION_HORDE;
        return entry->requiredFaction == ACHIEVEMENT_FACTION_ANY || entry->requiredFaction == faction;
    }

    uint32 HistoryOrder(uint32 guid, uint32 achievementId)
    {
        uint32 value = guid ^ (achievementId * 0x9E3779B9u);
        value ^= value >> 16;
        value *= 0x7FEB352Du;
        value ^= value >> 15;
        value *= 0x846CA68Bu;
        return value ^ (value >> 16);
    }

    uint32 PersonaTarget(uint32 guid, uint32 minimum, uint32 maximum, uint32 salt)
    {
        // Configuration and persistent identity determine the target; current totals never move the range.
        return minimum + HistoryOrder(guid, salt) % (maximum - minimum + 1);
    }

    uint32 AchievementPoints(Player* player)
    {
        uint32 points = 0;
        for (auto const& completed : player->GetAchievementMgr()->GetCompletedAchievements())
            if (AchievementEntry const* entry = sAchievementStore.LookupEntry(completed.first))
                points += entry->points;
        return points;
    }
}

namespace CityLife
{
    LegendaryPersona& LegendaryPersona::Instance()
    {
        static LegendaryPersona instance;
        return instance;
    }

    void LegendaryPersona::LoadConfig()
    {
        _enabled = sConfigMgr->GetOption<bool>("CityLife.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Enable", true);
        _honorEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Honor.Enable", true);
        _honorableKills = std::min<uint32>(1000000,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.HonorableKills", 10000));
        _randomizeHonorableKills = sConfigMgr->GetOption<bool>(
            "CityLife.Custom.Legendary.Honor.Randomize", true);
        _honorableKillsMin = std::min<uint32>(1000000,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.HonorableKills.Min", 11000));
        _honorableKillsMax = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.HonorableKills.Max", 25000),
            _honorableKillsMin, 1000000);
        _achievementsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Achievements.Enable", true);
        _achievementPoints = std::min<uint32>(25000,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.AchievementPoints", 5000));
        _randomizeAchievementPoints = sConfigMgr->GetOption<bool>(
            "CityLife.Custom.Legendary.Achievements.Randomize", true);
        _achievementPointsMin = std::min<uint32>(25000,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.AchievementPoints.Min", 5100));
        _achievementPointsMax = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Legendary.AchievementPoints.Max", 6000),
            _achievementPointsMin, 25000);
        _guildEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.Guild.Enable", true);
        _gearReputation = sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.TemplateGear.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Legendary.TemplateGear.ExaltedReputation", true);
        _guildName = sConfigMgr->GetOption<std::string>("CityLife.Custom.Legendary.Guild.Name", "The GOATS");
        _guildWarningLogged = false;
        _achievementWarningLogged = false;

        _achievementIds.clear();
        std::string configured = sConfigMgr->GetOption<std::string>(
            "CityLife.Custom.Legendary.AchievementIds", "default");
        _defaultAchievements = configured == "default";
        if (_defaultAchievements)
        {
            // Read IDs and point values from the running core's DBCs instead of hard-coding another client revision.
            for (uint32 id = 0; id < sAchievementStore.GetNumRows(); ++id)
                if (AchievementEntry const* entry = sAchievementStore.LookupEntry(id))
                {
                    uint32 category = RootCategory(entry->categoryId);
                    if (IsSafeHistoryAchievement(entry) && (category == DungeonCategory ||
                        category == GeneralCategory || category == ExplorationCategory))
                        _achievementIds.push_back(id);
                }
        }
        else
        {
            std::replace(configured.begin(), configured.end(), ',', ' ');
            std::istringstream input(configured);
            uint32 id;
            while (input >> id)
            {
                if (IsSafeHistoryAchievement(sAchievementStore.LookupEntry(id)))
                    _achievementIds.push_back(id);
                else
                    LOG_WARN("module", "[CityLife.Legendary] Achievement {} is invalid or unsafe; skipped.", id);
            }
            if (!input.eof())
                LOG_WARN("module", "[CityLife.Legendary] Invalid AchievementIds list; check comma-separated IDs.");
        }
        std::sort(_achievementIds.begin(), _achievementIds.end());
        _achievementIds.erase(std::unique(_achievementIds.begin(), _achievementIds.end()), _achievementIds.end());
    }

    bool LegendaryPersona::RefreshAchievements(Player* player) const
    {
        auto holder = std::make_shared<CharacterDatabaseQueryHolder>();
        holder->SetSize(2);
        CharacterDatabasePreparedStatement* statement =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_ACHIEVEMENTS);
        statement->SetData(0, player->GetGUID().GetCounter());
        if (!holder->SetPreparedQuery(0, statement))
            return false;
        // A real character row distinguishes a healthy empty history from an unavailable database connection.
        statement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER);
        statement->SetData(0, player->GetGUID().GetCounter());
        if (!holder->SetPreparedQuery(1, statement))
            return false;
        SQLQueryHolderCallback result = CharacterDatabase.DelayQueryHolder(holder);
        result.m_future.get();
        PreparedQueryResult character = holder->GetPreparedResult(1);
        if (!character || character->Fetch()[0].Get<uint32>() != player->GetGUID().GetCounter() ||
            character->Fetch()[1].Get<uint32>() != player->GetSession()->GetAccountId())
            return false;
        // This merges earned records without clearing criteria or broadcasting hundreds of simulated earns.
        player->GetAchievementMgr()->LoadFromDB(holder->GetPreparedResult(0), nullptr, nullptr);
        return true;
    }

    bool LegendaryPersona::ApplyAchievements(Player* player, uint32 targetPoints)
    {
        if (!_achievementsEnabled || !targetPoints || _achievementIds.empty())
            return true;

        // Preserve any real login progress before a DB refresh marks durable records unchanged.
        CharacterDatabaseTransaction pending = CharacterDatabase.BeginTransaction();
        player->GetAchievementMgr()->SaveToDB(pending);
        if (pending->GetSize() && !CharacterDatabase.AsyncCommitTransaction(pending).m_future.get())
            return false;
        // Reconcile a previous successful seed even when its acknowledgement/readback failed.
        if (!RefreshAchievements(player))
            return false;
        uint32 points = AchievementPoints(player);
        if (points >= targetPoints)
            return true;

        uint32 const guid = player->GetGUID().GetCounter();
        std::vector<uint32> candidates = _achievementIds;
        std::sort(candidates.begin(), candidates.end(), [guid](uint32 left, uint32 right)
        {
            uint32 leftOrder = HistoryOrder(guid, left);
            uint32 rightOrder = HistoryOrder(guid, right);
            return leftOrder == rightOrder ? left < right : leftOrder < rightOrder;
        });
        std::vector<uint32> selected;
        std::unordered_set<uint32> planned;
        auto select = [&](uint32 target, bool onlyDungeons)
        {
            for (uint32 id : candidates)
            {
                if (points >= target)
                    break;
                AchievementEntry const* entry = sAchievementStore.LookupEntry(id);
                if (!IsSafeHistoryAchievement(entry) || !MatchesFaction(player, entry) ||
                    player->HasAchieved(id) || planned.count(id) ||
                    (onlyDungeons && RootCategory(entry->categoryId) != DungeonCategory))
                    continue;
                selected.push_back(id);
                planned.insert(id);
                points += entry->points;
            }
        };
        // Most of the veteran's history comes from dungeons and raids; the remainder varies by fixed identity.
        if (_defaultAchievements)
            select(targetPoints * 7 / 10, true);
        select(targetPoints, false);
        if (selected.empty())
            return true;

        CharacterDatabaseTransaction history = CharacterDatabase.BeginTransaction();
        uint32 const earnedAt = static_cast<uint32>(GameTime::GetGameTime().count());
        for (uint32 id : selected)
        {
            CharacterDatabasePreparedStatement* statement =
                CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_ACHIEVEMENT);
            statement->SetData(0, guid);
            statement->SetData(1, id);
            statement->SetData(2, earnedAt);
            history->Append(statement);
        }
        if (!CharacterDatabase.AsyncCommitTransaction(history).m_future.get() || !RefreshAchievements(player))
            return false;
        for (uint32 id : selected)
            if (!player->HasAchieved(id))
                return false;
        if (AchievementPoints(player) < targetPoints && !_achievementWarningLogged)
        {
            LOG_WARN("module", "[CityLife.Legendary] Eligible achievements cannot reach the configured {} points; "
                "the available history was kept.", targetPoints);
            _achievementWarningLogged = true;
        }
        return true;
    }

    bool LegendaryPersona::IsOwnedGuild(Guild const* guild) const
    {
        if (!guild || !sCityLifeCustomBotPool.IsManagedLegendaryBot(guild->GetLeaderGUID().GetCounter()))
            return false;
        for (auto const& member : guild->GetUnorderedMembers())
            if (!sCityLifeCustomBotPool.IsManagedLegendaryBot(member.first))
                return false;
        return true;
    }

    void LegendaryPersona::ApplyGuild(Player* player)
    {
        if (!_guildEnabled)
            return;
        auto warn = [this](char const* reason)
        {
            if (!_guildWarningLogged)
            {
                LOG_WARN("module", "[CityLife.Legendary] Guild '{}' was skipped: {}.", _guildName, reason);
                _guildWarningLogged = true;
            }
        };
        if (!ObjectMgr::IsValidCharterName(_guildName))
        {
            warn("invalid or reserved guild name");
            return;
        }
        Guild* guild = sGuildMgr->GetGuildByName(_guildName);
        if (guild && !IsOwnedGuild(guild))
        {
            warn("the existing guild is not owned exclusively by registered Legendary characters");
            return;
        }
        if (player->GetGuildId())
        {
            if (!guild || player->GetGuildId() != guild->GetId())
                warn("the character already belongs to another guild");
            return;
        }
        if (!guild)
        {
            auto created = std::make_unique<Guild>();
            if (!created->Create(player, _guildName))
            {
                warn("the core could not create the guild");
                return;
            }
            guild = created.release();
            sGuildMgr->AddGuild(guild);
            return;
        }
        // Guild::AddMember handles persistence and permits this dedicated mixed-faction display guild.
        if (!guild->AddMember(player->GetGUID()))
            warn("the core could not add the registered character");
    }

    bool LegendaryPersona::Apply(Player* player)
    {
        if (!_enabled)
            return true;
        if (!player || !player->GetSession() ||
            !sCityLifeCustomBotPool.IsManagedLegendaryBot(player->GetGUID().GetCounter()))
            return false;
        // The bundled BiS Ashen Verdict rings require Exalted. Touch only this registered display character.
        if (_gearReputation)
            if (FactionEntry const* ashenVerdict = sFactionStore.LookupEntry(1156))
                if (player->GetReputationMgr().GetReputation(ashenVerdict) < 42000)
                    player->GetReputationMgr().SetOneFactionReputation(ashenVerdict, 42000.0f, false);
        uint32 const guid = player->GetGUID().GetCounter();
        uint32 const targetPoints = _randomizeAchievementPoints ?
            PersonaTarget(guid, _achievementPointsMin, _achievementPointsMax, AchievementTargetSalt) :
            _achievementPoints;
        if (!ApplyAchievements(player, targetPoints))
            return false;
        uint32 const honorableKills = _randomizeHonorableKills ?
            PersonaTarget(guid, _honorableKillsMin, _honorableKillsMax, HonorTargetSalt) : _honorableKills;
        if (_honorEnabled && player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS) < honorableKills)
            player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, honorableKills);
        ApplyGuild(player);
        return true;
    }
}
