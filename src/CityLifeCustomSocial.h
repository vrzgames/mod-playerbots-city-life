/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CITY_LIFE_CUSTOM_SOCIAL_H
#define CITY_LIFE_CUSTOM_SOCIAL_H

#include "Define.h"
#include "ObjectGuid.h"

#include <array>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;

namespace CityLife
{
    class CustomSocial
    {
    public:
        static CustomSocial& Instance();

        void LoadConfig();
        bool LoadData();
        // Call on the world thread, about once per second, with idle city residents only.
        void Update(std::vector<uint32> const& eligibleBotGuids);
        // The player hook can run on a map thread. It only queues GUIDs for Update().
        void OnTextEmote(Player* sender, uint32 textEmote, ObjectGuid targetGuid);
        void Remove(uint32 botGuid);
        bool IsConversing(uint32 botGuid) const;

    private:
        struct Line
        {
            uint8 Channel = 0;
            std::string Text;
        };

        struct Dialogue
        {
            uint32 Id = 0;
            uint8 Team = 2;
            uint32 Weight = 100;
            std::array<Line, 3> Lines;
        };

        struct ActiveDialogue
        {
            uint32 First = 0;
            uint32 Second = 0;
            uint32 TemplateIndex = 0;
            uint32 NextAt = 0;
            uint8 NextStep = 1;
        };

        struct PendingEmote
        {
            uint32 Sender = 0;
            uint32 TextEmote = 0;
            uint32 ExpiresAt = 0;
            ObjectGuid Target;
        };

        CustomSocial() = default;

        bool IsEligible(Player* player) const;
        bool IsBusy(uint32 botGuid) const;
        bool CanInteract(Player* observer, Player* bot, float range) const;
        std::vector<uint32> RealObservers() const;
        bool HasObserver(Player* first, Player* second, std::vector<uint32> const& observers) const;
        bool CanConverse(Player* first, Player* second) const;
        bool Speak(Player* speaker, Line const& line) const;
        void ProcessReactions(uint32 now);
        void ProcessDialogues(uint32 now);
        void TryStartDialogue(uint32 now);

        bool _enabled = true;
        bool _reactionsEnabled = true;
        uint32 _reactionChance = 70;
        uint32 _reactionCooldownSeconds = 30;
        float _reactionRange = 20.0f;
        bool _conversationsEnabled = true;
        uint32 _conversationMinSeconds = 180;
        uint32 _conversationMaxSeconds = 420;
        float _observerRange = 25.0f;
        float _partnerRange = 12.0f;
        uint32 _replyMinSeconds = 2;
        uint32 _replyMaxSeconds = 5;
        uint32 _maxConcurrent = 2;
        uint32 _nextConversationAt = 0;
        uint32 _lastUpdateAt = 0;
        uint32 _nextPruneAt = 0;

        std::unordered_set<uint32> _eligibleBots;
        std::unordered_map<uint32, uint32> _botCooldownUntil;
        std::unordered_map<uint32, uint32> _senderCooldownUntil;
        std::vector<Dialogue> _dialogues;
        std::vector<ActiveDialogue> _activeDialogues;
        std::mutex _pendingMutex;
        std::vector<PendingEmote> _pendingEmotes;
    };
}

#define sCityLifeCustomSocial CityLife::CustomSocial::Instance()

#endif
