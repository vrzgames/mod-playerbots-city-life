/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_CUSTOM_BOT_POOL_H
#define CITY_LIFE_CUSTOM_BOT_POOL_H

#include "CityLifeTypes.h"
#include "PlayerbotMgr.h"
#include "WorldSession.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace CityLife
{
    // A separate holder keeps these characters out of RandomPlayerbotMgr's population/cycling.
    // Call every method on the world thread. Shutdown before the world/maps are destroyed.
    class CustomBotPool : public PlayerbotHolder
    {
    public:
        static CustomBotPool& Instance();
        ~CustomBotPool() override;

        void LoadConfig();
        bool LoadData();
        void SyncHubs(std::vector<Hub> const& hubs);
        void Update();
        bool RequestBot(Hub const& hub);
        bool IsManagedBot(uint32 guidLow) const;
        bool IsManagedLegendaryBot(uint32 guidLow);
        bool IsAssignedTo(uint32 guidLow, Hub const& hub) const;
        uint32 GetSlot(uint32 guidLow) const;
        void OnLogin(Player* player);
        void Shutdown();

    protected:
        void OnBotLoginInternal(Player* const bot) override;

    private:
        struct PoolSettings
        {
            bool Enabled = true;
            bool CreationEnabled = true;
            uint32 MaxBots = 30;
            uint32 CreateIntervalSeconds = 30;
            std::string AccountPrefix = "CITYLIFE";
            uint32 NextCreateAt = 0;
            uint32 NextWarningAt = 0;
        };

        struct Record
        {
            uint32 Id = 0;
            uint32 AccountId = 0;
            uint32 Guid = 0;
            std::string AccountName;
            std::string OwnerToken;
            std::string HubName;
            uint32 Slot = 0;
            uint8 TargetLevel = 1;
            uint8 Race = 1;
            uint8 Class = 1;
            CustomRole Role = CustomRole::None;
            bool Initialized = false;
            bool AccountVerified = false;
            bool Blocked = false;
            uint32 AccountQueuedAt = 0;
            uint32 RetryAt = 0;
        };

        struct Request
        {
            std::string HubName;
            uint8 MinLevel = 1;
            uint8 MaxLevel = 80;
            TeamSide Team = TeamSide::Any;
            CustomRole Role = CustomRole::None;
            uint32 Population = 0;
            uint32 Until = 0;
        };

        struct Session
        {
            std::unique_ptr<WorldSession> Value;
            bool Pending = true;
            bool Failed = false;
            bool DiscardChanges = false;
            bool PersonaApplied = false;
            uint32 StartedAt = 0;
        };

        CustomBotPool() = default;
        PoolSettings& GetPoolSettings(CustomRole role);
        PoolSettings const& GetPoolSettings(CustomRole role) const;
        bool IsRoleEnabled(CustomRole role) const;
        uint32 CountRecords(CustomRole role) const;
        Record* FindRecord(uint32 guidLow);
        Record const* FindRecord(uint32 guidLow) const;
        bool Matches(Record const& record, Request const& request) const;
        bool IsDesired(Record const& record) const;
        bool ReserveRecord(Request const& request);
        bool AdvanceRecord(Record& record);
        bool VerifyAccount(Record& record);
        bool CreateCharacter(Record& record);
        bool ConfirmCharacter(Record& record, uint32 guidLow);
        bool StartLogin(Record& record);
        void InitializeNewCharacter(Player* player);
        void ApplyLegendaryPersona(Player* player);
        void HoldIdle(Player* player) const;
        void FinishLogin(uint32 guidLow);
        void UpdateSessionsOwned();

        bool _enabled = true;
        bool _databaseReady = false;
        bool _stopping = false;
        PoolSettings _customSettings;
        PoolSettings _fishingSettings;
        std::vector<uint8> _legendaryClasses = {1, 4, 2};
        uint32 _lastWorkAt = 0;
        std::vector<Record> _records;
        std::map<std::string, Request> _requests;
        std::map<std::string, Request> _desiredHubs;
        std::unordered_map<uint32, Session> _sessions;
    };
}

#define sCityLifeCustomBotPool CityLife::CustomBotPool::Instance()

#endif
