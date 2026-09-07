/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_MGR_H
#define CITY_LIFE_MGR_H

#include "CityLifeTypes.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ChatHandler;
class Field;
class Player;

namespace CityLife
{
    class Manager
    {
    public:
        static Manager& Instance();

        void LoadConfig();
        void Update(uint32 diff);
        bool HandleCommand(ChatHandler* handler, std::string const& text);

    private:
        Manager() = default;

        bool VerifyDatabase();
        bool LoadData();
        void ImportPlayerbotDefaults();

        TeamSide TeamForRace(uint8 race) const;
        Player* FindPlayer(uint32 guidLow) const;
        bool IsConfiguredBotAccount(uint32 accountId);
        bool IsCandidate(Player* player);
        bool IsResident(uint32 guidLow) const;
        bool IsResidentUsable(Player* player) const;
        std::vector<BotCandidate> LoadCandidates(Hub const& hub);

        Hub* FindHub(uint32 hubId);
        Hub* FindHub(std::string const& name);
        bool IsStaticServiceSpot(Hub const& hub, Spot const& spot) const;
        Spot const* ChooseSpot(Hub const& hub, uint32 ignoredSpotId = 0) const;
        uint32 CountResidents(uint32 hubId) const;
        uint32 TargetPopulation(Hub const& hub) const;
        uint32 CurrentTimeMultiplier() const;

        bool AddResident(Hub const& hub);
        void ReleaseResident(Resident const& resident, bool returnHome);
        void ReleaseAll(bool returnHome);
        void RemoveUnavailableResidents();
        void MaintainPopulation();
        void MaintainResidents();
        void RelocateResident(Resident& resident, Hub const& hub);
        void AssignSpot(Resident& resident, Hub const& hub, Spot const& spot, bool allowStationary);

        void ApplyCityStrategies(Player* player) const;
        void SetStayPosition(Player* player, Resident const& resident) const;
        void ClearCityStrategies(Player* player) const;
        void MoveBot(Player* player, uint32 mapId, float x, float y, float z, float o, bool teleport) const;
        void TryEmote(Player* player) const;

        void PrintStatus(ChatHandler* handler) const;
        void PrintHelp(ChatHandler* handler) const;
        bool HandleHubCommand(ChatHandler* handler, std::vector<std::string> const& args);
        bool HandleSpotCommand(ChatHandler* handler, std::vector<std::string> const& args);

        bool _enable = true;
        bool _databaseReady = false;
        bool _debug = false;
        uint32 _startupDelaySeconds = 90;
        uint32 _tickSeconds = 10;
        uint32 _startupElapsedMs = 0;
        uint32 _timerMs = 0;

        bool _usePlayerbotConfig = true;
        std::string _botAccountPrefix = "auto";
        uint32 _botAccountMin = 0;
        uint32 _botAccountMax = 0;
        uint32 _botQueryLimit = 500;
        bool _skipGroupedBots = true;
        bool _respectPlayerbotActivity = false;
        bool _returnBots = true;
        uint32 _maxTotalPopulation = 300;
        uint32 _maxChangesPerTick = 3;

        bool _timeScaling = true;
        uint32 _morningMultiplier = 60;
        uint32 _dayMultiplier = 80;
        uint32 _eveningMultiplier = 100;
        uint32 _nightMultiplier = 25;

        bool _roaming = true;
        uint32 _relocateMinSeconds = 180;
        uint32 _relocateMaxSeconds = 600;
        uint32 _leashRadius = 35;
        bool _emotes = true;
        uint32 _emoteChance = 35;
        uint32 _emoteMinSeconds = 120;
        uint32 _emoteMaxSeconds = 360;
        uint32 _serviceSpotWeightMultiplier = 250;
        uint32 _staticServiceChance = 80;
        std::unordered_set<std::string> _staticServiceHubs;
        std::unordered_set<std::string> _staticServiceCategories;
        std::string _nonCombatStrategies = "+stay,-follow,-passive,-grind,-rpg,-travel,-duel,-pvp";

        std::vector<Hub> _hubs;
        std::vector<Resident> _residents;
        std::unordered_map<uint32, bool> _botAccountCache;
    };
}

#define sCityLifeMgr CityLife::Manager::Instance()

#endif
