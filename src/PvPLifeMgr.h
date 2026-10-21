/*
 * PvP Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PVP_LIFE_MGR_H
#define PVP_LIFE_MGR_H

#include "PvPLifeTypes.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ChatHandler;
class Player;
class Field;

namespace PvPLife
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
        void ImportPlayerbotDefaults();

        std::vector<Zone> LoadZones(bool readyOnly = false, int typeFilter = -1);
        bool LoadZoneByName(std::string name, Zone& out);
        void LoadZoneConfig();
        void ApplyZoneConfig(Zone& zone) const;
        Zone ReadZone(Field* fields);
        std::string ZoneSelectSql(std::string const& suffix = "") const;

        std::vector<BotCandidate> LoadCandidates(TeamSide team, uint8 minLevel, uint8 maxLevel,
            std::unordered_set<uint32> const& excluded);
        Player* FindPlayer(uint32 guidLow) const;
        TeamSide TeamForRace(uint8 race) const;
        bool IsConfiguredBotAccount(uint32 accountId);
        bool IsSafeBot(Player* player) const;
        bool IsRealPlayer(Player* player) const;
        bool IsParticipant(uint32 guidLow) const;

        bool StartZone(Zone const& zone, ChatHandler* handler = nullptr, bool forced = false);
        void EndEvent(ActiveEvent const& event, std::string const& reason, bool disabling = false);
        void ExpireEvents();
        void MaintainPersistentWorldPvp();
        void MaintainPersistentDuelZones();
        void TryStartRandomSkirmish();
        void TryStartFactionCampaign();

        void ScheduleMove(uint64 eventId, uint32 guidLow, uint32 mapId, float x, float y, float z, float o,
            uint32 delaySeconds, bool teleport);
        void ScheduleDuel(uint64 eventId, uint32 challengerGuid, uint32 targetGuid, uint32 delaySeconds,
            bool realPlayerTarget = false);
        void ScheduleEventChat(ActiveEvent const& event);
        void ProcessMoveQueue();
        void ProcessDuelQueue();
        void ProcessChatQueue();
        void ProcessRealPlayerChallenges();
        void MaintainDuelParticipants();
        void MaintainBotDuels();

        void ApplyPvpStrategies(Player* player, bool duelMode) const;
        void MoveBot(Player* player, uint32 mapId, float x, float y, float z, float o, bool teleport) const;
        float Jitter() const;
        void CalculatePopulation(Zone const& zone, uint32& attackerCount, uint32& defenderCount) const;

        std::string RandomChatLine(ActivityType type, ChatChannel channel, TeamSide speakerTeam);
        void Announce(std::string const& text) const;
        void PrintStatus(ChatHandler* handler) const;
        void PrintHelp(ChatHandler* handler) const;
        bool HandleZoneCommand(ChatHandler* handler, std::vector<std::string> const& args);

        bool _enable = true;
        bool _databaseReady = false;
        bool _debug = false;
        bool _announce = false;
        uint32 _startupDelaySeconds = 90;
        uint32 _tickSeconds = 20;
        uint32 _timerMs = 0;
        uint32 _startupElapsedMs = 0;

        bool _usePlayerbotConfig = true;
        std::string _botAccountPrefix = "auto";
        uint32 _botAccountMin = 0;
        uint32 _botAccountMax = 0;
        uint32 _botQueryLimit = 500;
        bool _skipGroupedBots = true;
        bool _respectPlayerbotActivity = false;
        bool _allowPartialTeams = true;
        uint32 _minimumBotsPerSide = 1;
        bool _returnBots = true;
        bool _useMovePoint = true;
        uint32 _positionJitter = 10;
        uint32 _arrivalStaggerMin = 2;
        uint32 _arrivalStaggerMax = 12;
        uint32 _moveAfterArrivalMin = 6;
        uint32 _moveAfterArrivalMax = 20;
        uint32 _maxBotsPerSide = 100;

        bool _alwaysActiveWorldPvp = true;
        uint32 _minActiveSkirmishes = 1;
        uint32 _maxActiveSkirmishes = 3;
        uint32 _randomSkirmishChance = 35;

        bool _duelEnable = true;
        bool _duelAlwaysActive = true;
        uint32 _duelSpellId = 7266;
        uint32 _duelPairLimit = 12;
        uint32 _duelPairDelayMin = 5;
        uint32 _duelPairDelayMax = 18;
        uint32 _duelLeashRadius = 40;
        uint32 _duelGuardIntervalMs = 1000;
        uint32 _duelGuardTimerMs = 0;
        bool _challengeRealPlayers = true;
        uint32 _realPlayerChallengeChance = 25;
        uint32 _realPlayerScanRadius = 55;
        uint32 _realPlayerMaxLevelDifference = 5;
        uint32 _realPlayerCooldownSeconds = 900;
        uint32 _botChallengeCooldownSeconds = 180;
        uint32 _botDuelCooldownSeconds = 45;
        uint32 _botDuelMaxLevelDifference = 5;

        bool _forTheHordeEnable = true;
        bool _forTheAllianceEnable = true;
        uint32 _factionCampaignChance = 12;
        uint32 _factionCampaignGlobalCooldown = 3600;
        uint32 _lastFactionCampaign = 0;

        bool _botChatEnable = true;
        uint32 _botChatYellChance = 70;
        uint32 _botChatWorldChance = 35;

        std::string _pvpCombatStrategies = "+pvp,+boost,+dps debuff,-passive,-stay";
        std::string _pvpNonCombatStrategies = "+pvp,+boost,-passive,-stay";
        std::string _duelNonCombatStrategies = "+duel,+pvp,+boost,+stay,-follow,-passive,-grind";

        uint64 _nextEventId = 1;
        std::vector<ActiveEvent> _activeEvents;
        std::vector<MoveStep> _moveQueue;
        std::vector<DuelStep> _duelQueue;
        std::vector<ChatStep> _chatQueue;
        std::unordered_map<uint32, uint32> _playerChallengeCooldown;
        std::unordered_map<uint32, uint32> _botChallengeCooldown;
        std::unordered_map<uint32, uint32> _botDuelCooldown;
        std::unordered_map<uint32, bool> _botAccountCache;

        struct ZoneConfig
        {
            std::optional<bool> Enabled;
            uint32 MinPopulation = 2;
            uint32 MaxPopulation = 2;
        };

        std::unordered_map<std::string, ZoneConfig> _zoneConfig;
    };
}

#define sPvPLifeMgr PvPLife::Manager::Instance()

#endif
