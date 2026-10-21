/*
 * PvP Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PVP_LIFE_TYPES_H
#define PVP_LIFE_TYPES_H

#include "Define.h"
#include <string>
#include <vector>

namespace PvPLife
{
    enum class TeamSide : uint8
    {
        Alliance = 0,
        Horde = 1,
        Any = 2
    };

    enum class ActivityType : uint8
    {
        Skirmish = 0,
        Duel = 1,
        ForTheHorde = 2,
        ForTheAlliance = 3
    };

    enum class ChatChannel : uint8
    {
        Yell = 0,
        World = 1
    };

    struct Zone
    {
        uint32 Id = 0;
        std::string Name;
        bool Enabled = true;
        ActivityType Type = ActivityType::Skirmish;
        TeamSide AttackerTeam = TeamSide::Horde;
        TeamSide DefenderTeam = TeamSide::Alliance;
        uint8 MinLevel = 1;
        uint8 MaxLevel = 80;
        uint32 MapId = 0;
        float RallyX = 0.0f;
        float RallyY = 0.0f;
        float RallyZ = 0.0f;
        float RallyO = 0.0f;
        float TargetX = 0.0f;
        float TargetY = 0.0f;
        float TargetZ = 0.0f;
        float TargetO = 0.0f;
        uint32 AttackersMin = 3;
        uint32 AttackersMax = 8;
        uint32 DefendersMin = 3;
        uint32 DefendersMax = 8;
        uint32 PopulationMin = 6;
        uint32 PopulationMax = 16;
        uint32 DurationMin = 10;
        uint32 DurationMax = 25;
        uint32 Weight = 100;
        uint32 CooldownSeconds = 900;
        uint32 LastStart = 0;
        bool ChallengePlayers = false;
        bool BotChat = false;
    };

    struct BotCandidate
    {
        uint32 GuidLow = 0;
        uint32 AccountId = 0;
        std::string Name;
        uint8 Level = 1;
        uint8 Race = 0;
        uint8 Class = 0;
        TeamSide Team = TeamSide::Any;
    };

    struct Participant
    {
        BotCandidate Bot;
        bool Attacker = false;
        uint32 OriginalMap = 0;
        float OriginalX = 0.0f;
        float OriginalY = 0.0f;
        float OriginalZ = 0.0f;
        float OriginalO = 0.0f;
        float ActivityX = 0.0f;
        float ActivityY = 0.0f;
        float ActivityZ = 0.0f;
        float ActivityO = 0.0f;
    };

    struct ActiveEvent
    {
        uint64 EventId = 0;
        Zone EventZone;
        uint32 StartedAt = 0;
        uint32 EndsAt = 0;
        std::vector<Participant> Members;
    };

    struct MoveStep
    {
        uint64 EventId = 0;
        uint32 GuidLow = 0;
        uint32 ExecuteAt = 0;
        uint32 MapId = 0;
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
        float O = 0.0f;
        bool Teleport = false;
    };

    struct DuelStep
    {
        uint64 EventId = 0;
        uint32 ChallengerGuidLow = 0;
        uint32 TargetGuidLow = 0;
        uint32 ExecuteAt = 0;
        uint8 RetryCount = 0;
        bool TargetIsRealPlayer = false;
    };

    struct ChatStep
    {
        uint64 EventId = 0;
        uint32 SpeakerGuidLow = 0;
        uint32 ExecuteAt = 0;
        ChatChannel Channel = ChatChannel::Yell;
        std::string Text;
    };
}

#endif
