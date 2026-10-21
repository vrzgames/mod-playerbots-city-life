/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeCustomSocial.h"

#include "Config.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Random.h"
#include "SharedDefines.h"

#include <algorithm>
#include <map>
#include <shared_mutex>

namespace
{
    uint32 SocialTime()
    {
        return static_cast<uint32>(GameTime::GetGameTime().count());
    }

    Player* FindSocialPlayer(uint32 guid)
    {
        return ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(guid));
    }

    uint32 ReactionEmote(uint32 textEmote)
    {
        switch (textEmote)
        {
            case TEXT_EMOTE_WAVE:
                return EMOTE_ONESHOT_WAVE;
            case TEXT_EMOTE_BOW:
                return EMOTE_ONESHOT_BOW;
            case TEXT_EMOTE_DANCE:
                return EMOTE_ONESHOT_DANCE;
            case TEXT_EMOTE_CHEER:
                return EMOTE_ONESHOT_CHEER;
            default:
                return 0;
        }
    }
}

namespace CityLife
{
    CustomSocial& CustomSocial::Instance()
    {
        static CustomSocial instance;
        return instance;
    }

    void CustomSocial::LoadConfig()
    {
        _enabled = sConfigMgr->GetOption<bool>("CityLife.Enable", true) &&
            sConfigMgr->GetOption<bool>("CityLife.Custom.Enable", true);
        _reactionsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.PlayerReactions.Enable", true);
        _reactionChance = std::min<uint32>(100,
            sConfigMgr->GetOption<uint32>("CityLife.Custom.PlayerReactions.ChancePct", 70));
        _reactionCooldownSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.PlayerReactions.CooldownSeconds", 30), 5, 3600);
        _reactionRange = std::clamp(
            sConfigMgr->GetOption<float>("CityLife.Custom.PlayerReactions.Range", 20.0f), 1.0f, 40.0f);
        _conversationsEnabled = sConfigMgr->GetOption<bool>("CityLife.Custom.Conversations.Enable", true);
        _conversationMinSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Conversations.MinSeconds", 180), 30, 86400);
        _conversationMaxSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Conversations.MaxSeconds", 420),
            _conversationMinSeconds, 86400);
        _observerRange = std::clamp(
            sConfigMgr->GetOption<float>("CityLife.Custom.Conversations.Range", 25.0f), 1.0f, 40.0f);
        _partnerRange = std::clamp(
            sConfigMgr->GetOption<float>("CityLife.Custom.Conversations.PartnerRange", 12.0f), 1.0f, 25.0f);
        _replyMinSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Conversations.ReplyMinSeconds", 2), 1, 30);
        _replyMaxSeconds = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Conversations.ReplyMaxSeconds", 5),
            _replyMinSeconds, 30);
        _maxConcurrent = std::clamp<uint32>(
            sConfigMgr->GetOption<uint32>("CityLife.Custom.Conversations.MaxConcurrent", 2), 1, 10);

        _eligibleBots.clear();
        _activeDialogues.clear();
        _botCooldownUntil.clear();
        _senderCooldownUntil.clear();
        _nextConversationAt = SocialTime() + urand(_conversationMinSeconds, _conversationMaxSeconds);
        _lastUpdateAt = 0;
        _nextPruneAt = 0;
        std::lock_guard<std::mutex> lock(_pendingMutex);
        _pendingEmotes.clear();
    }

    bool CustomSocial::LoadData()
    {
        _dialogues.clear();
        _activeDialogues.clear();
        if (!_enabled || !_conversationsEnabled)
            return true;
        if (!WorldDatabase.Query("SHOW TABLES LIKE 'city_life_custom_chat'"))
        {
            LOG_WARN("module", "[CityLife] Custom conversations need WORLD table city_life_custom_chat. "
                "Apply data/sql/manual/world_city_life_custom.sql");
            return false;
        }

        QueryResult result = WorldDatabase.Query(
            "SELECT conversation_id,step,speaker_team,channel,text,weight FROM city_life_custom_chat "
            "WHERE group_name='conversation' AND enabled=1 ORDER BY conversation_id,step,id");
        if (!result)
            return true;

        struct PendingDialogue
        {
            Dialogue Value;
            std::array<bool, 3> Seen = {};
            bool Valid = true;
        };
        std::map<uint32, PendingDialogue> pending;
        do
        {
            Field* fields = result->Fetch();
            uint32 const id = fields[0].Get<uint32>();
            if (!pending.count(id) && pending.size() >= 1024)
                continue;
            PendingDialogue& entry = pending[id];
            entry.Value.Id = id;
            uint8 const step = fields[1].Get<uint8>();
            uint8 const team = fields[2].Get<uint8>();
            uint8 const channel = fields[3].Get<uint8>();
            std::string const text = fields[4].Get<std::string>();
            if (step > 2 || team > 2 || channel > 1 || text.empty() || text.size() > 255)
            {
                entry.Valid = false;
                continue;
            }
            if (entry.Seen[step] || (team != 2 && entry.Value.Team != 2 && entry.Value.Team != team))
                entry.Valid = false;
            entry.Seen[step] = true;
            if (team != 2)
                entry.Value.Team = team;
            entry.Value.Lines[step] = {channel, text};
            if (step == 0)
                entry.Value.Weight = std::clamp<uint32>(fields[5].Get<uint32>(), 1, 1000000);
        } while (result->NextRow());
        for (auto const& [id, entry] : pending)
        {
            if (entry.Valid && std::all_of(entry.Seen.begin(), entry.Seen.end(), [](bool seen) { return seen; }))
                _dialogues.push_back(entry.Value);
            else
                LOG_WARN("module", "[CityLife] Ignoring invalid Custom conversation {} (need unique steps 0,1,2).", id);
        }
        return true;
    }

    bool CustomSocial::IsEligible(Player* player) const
    {
        return player && _eligibleBots.count(player->GetGUID().GetCounter()) && player->GetSession() &&
            player->IsInWorld() && player->IsAlive() && !player->IsBeingTeleported() &&
            !player->IsInCombat() && !player->isMoving() && !player->IsInFlight() && !player->duel &&
            !player->GetGroup() && !player->InBattleground() && !player->InBattlegroundQueue() &&
            !player->IsNonMeleeSpellCast(false) && GET_PLAYERBOT_AI(player) && !::IsRealPlayer(player) &&
            !GET_PLAYERBOT_AI(player)->HasGameClientMaster();
    }

    bool CustomSocial::IsBusy(uint32 botGuid) const
    {
        return std::any_of(_activeDialogues.begin(), _activeDialogues.end(),
            [botGuid](ActiveDialogue const& dialogue)
            {
                return dialogue.First == botGuid || dialogue.Second == botGuid;
            });
    }

    bool CustomSocial::CanInteract(Player* observer, Player* bot, float range) const
    {
        return observer && bot && observer->GetSession() && observer->IsInWorld() && observer->IsAlive() &&
            !observer->IsBeingTeleported() && ::IsRealPlayer(observer) &&
            observer->GetTeamId() == bot->GetTeamId() && observer->InSamePhase(bot) &&
            observer->IsWithinDistInMap(bot, range) && observer->CanSeeOrDetect(bot) &&
            bot->CanSeeOrDetect(observer);
    }

    std::vector<uint32> CustomSocial::RealObservers() const
    {
        std::vector<uint32> observers;
        std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
            if (player && player->GetSession() && player->IsInWorld() && player->IsAlive() &&
                !player->IsBeingTeleported() && ::IsRealPlayer(player))
                observers.push_back(guid.GetCounter());
        return observers;
    }

    bool CustomSocial::HasObserver(Player* first, Player* second, std::vector<uint32> const& observers) const
    {
        for (uint32 guid : observers)
        {
            Player* observer = FindSocialPlayer(guid);
            if (CanInteract(observer, first, _observerRange) &&
                (!second || CanInteract(observer, second, _observerRange)))
                return true;
        }
        return false;
    }

    bool CustomSocial::CanConverse(Player* first, Player* second) const
    {
        return IsEligible(first) && IsEligible(second) && first != second &&
            first->GetTeamId() == second->GetTeamId() && first->InSamePhase(second) &&
            first->IsWithinDistInMap(second, _partnerRange) && first->CanSeeOrDetect(second) &&
            second->CanSeeOrDetect(first);
    }

    bool CustomSocial::Speak(Player* speaker, Line const& line) const
    {
        PlayerbotAI* ai = GET_PLAYERBOT_AI(speaker);
        if (!ai)
            return false;
        speaker->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
        return line.Channel == 1 ? ai->Yell(line.Text) : ai->Say(line.Text);
    }

    void CustomSocial::OnTextEmote(Player* sender, uint32 textEmote, ObjectGuid targetGuid)
    {
        if (!sender || !::IsRealPlayer(sender) || !ReactionEmote(textEmote) ||
            (!targetGuid.IsEmpty() && !targetGuid.IsPlayer()))
            return;
        std::lock_guard<std::mutex> lock(_pendingMutex);
        if (_pendingEmotes.size() < 128)
            _pendingEmotes.push_back({sender->GetGUID().GetCounter(), textEmote, SocialTime() + 10, targetGuid});
    }

    void CustomSocial::ProcessReactions(uint32 now)
    {
        std::vector<PendingEmote> pending;
        {
            std::lock_guard<std::mutex> lock(_pendingMutex);
            pending.swap(_pendingEmotes);
        }
        if (!_enabled || !_reactionsEnabled)
            return;
        for (PendingEmote const& emote : pending)
        {
            if (emote.ExpiresAt < now || _senderCooldownUntil[emote.Sender] > now)
                continue;
            Player* sender = FindSocialPlayer(emote.Sender);
            Player* bot = nullptr;
            if (!emote.Target.IsEmpty())
                bot = FindSocialPlayer(emote.Target.GetCounter());
            else
            {
                float nearest = _reactionRange;
                for (uint32 guid : _eligibleBots)
                {
                    Player* candidate = FindSocialPlayer(guid);
                    if (IsEligible(candidate) && !IsBusy(guid) && CanInteract(sender, candidate, nearest))
                    {
                        bot = candidate;
                        nearest = sender->GetExactDist(candidate);
                    }
                }
            }
            if (!IsEligible(bot) || !CanInteract(sender, bot, _reactionRange) ||
                IsBusy(bot->GetGUID().GetCounter()) || _botCooldownUntil[bot->GetGUID().GetCounter()] > now)
                continue;
            _senderCooldownUntil[emote.Sender] = now + _reactionCooldownSeconds;
            if (urand(1, 100) > _reactionChance)
                continue;
            bot->SetFacingToObject(sender);
            bot->HandleEmoteCommand(ReactionEmote(emote.TextEmote));
            _botCooldownUntil[bot->GetGUID().GetCounter()] = now + _reactionCooldownSeconds;
        }
    }

    void CustomSocial::ProcessDialogues(uint32 now)
    {
        std::vector<uint32> observers;
        if (!_activeDialogues.empty())
            observers = RealObservers();
        for (auto dialogue = _activeDialogues.begin(); dialogue != _activeDialogues.end();)
        {
            Player* first = FindSocialPlayer(dialogue->First);
            Player* second = FindSocialPlayer(dialogue->Second);
            if (!CanConverse(first, second) || !HasObserver(first, second, observers) ||
                dialogue->TemplateIndex >= _dialogues.size())
            {
                dialogue = _activeDialogues.erase(dialogue);
                continue;
            }
            if (now < dialogue->NextAt)
            {
                ++dialogue;
                continue;
            }
            Player* speaker = dialogue->NextStep == 1 ? second : first;
            Player* listener = dialogue->NextStep == 1 ? first : second;
            speaker->SetFacingToObject(listener);
            Speak(speaker, _dialogues[dialogue->TemplateIndex].Lines[dialogue->NextStep]);
            if (++dialogue->NextStep > 2)
                dialogue = _activeDialogues.erase(dialogue);
            else
            {
                dialogue->NextAt = now + urand(_replyMinSeconds, _replyMaxSeconds);
                ++dialogue;
            }
        }
    }

    void CustomSocial::TryStartDialogue(uint32 now)
    {
        if (_activeDialogues.size() >= _maxConcurrent || _dialogues.empty() || _eligibleBots.size() < 2)
            return;
        std::vector<uint32> observers = RealObservers();
        if (observers.empty())
            return;
        std::vector<uint32> candidates;
        for (uint32 guid : _eligibleBots)
        {
            Player* bot = FindSocialPlayer(guid);
            if (IsEligible(bot) && !IsBusy(guid) && _botCooldownUntil[guid] <= now &&
                HasObserver(bot, nullptr, observers))
                candidates.push_back(guid);
        }
        for (std::size_t i = candidates.size(); i > 1; --i)
            std::swap(candidates[i - 1], candidates[urand(0, static_cast<uint32>(i - 1))]);
        for (uint32 firstGuid : candidates)
        {
            Player* first = FindSocialPlayer(firstGuid);
            for (uint32 secondGuid : candidates)
            {
                Player* second = FindSocialPlayer(secondGuid);
                if (!CanConverse(first, second) || !HasObserver(first, second, observers))
                    continue;
                uint32 totalWeight = 0;
                for (Dialogue const& entry : _dialogues)
                    if (entry.Team == 2 || entry.Team == static_cast<uint8>(first->GetTeamId()))
                        totalWeight += entry.Weight;
                if (!totalWeight)
                    continue;
                uint32 roll = urand(1, totalWeight);
                for (std::size_t i = 0; i < _dialogues.size(); ++i)
                {
                    Dialogue const& entry = _dialogues[i];
                    if (entry.Team != 2 && entry.Team != static_cast<uint8>(first->GetTeamId()))
                        continue;
                    if (roll > entry.Weight)
                    {
                        roll -= entry.Weight;
                        continue;
                    }
                    first->SetFacingToObject(second);
                    Speak(first, entry.Lines[0]);
                    _activeDialogues.push_back({firstGuid, secondGuid, static_cast<uint32>(i),
                        now + urand(_replyMinSeconds, _replyMaxSeconds), 1});
                    _botCooldownUntil[firstGuid] = now + _conversationMinSeconds;
                    _botCooldownUntil[secondGuid] = now + _conversationMinSeconds;
                    return;
                }
            }
        }
    }

    void CustomSocial::Update(std::vector<uint32> const& eligibleBotGuids)
    {
        _eligibleBots.clear();
        _eligibleBots.insert(eligibleBotGuids.begin(), eligibleBotGuids.end());
        uint32 const now = SocialTime();
        if (_lastUpdateAt == now)
            return;
        _lastUpdateAt = now;
        ProcessReactions(now);
        if (!_enabled || !_conversationsEnabled)
            _activeDialogues.clear();
        else
        {
            ProcessDialogues(now);
            if (now >= _nextConversationAt)
            {
                _nextConversationAt = now + urand(_conversationMinSeconds, _conversationMaxSeconds);
                TryStartDialogue(now);
            }
        }
        if (now >= _nextPruneAt)
        {
            _nextPruneAt = now + MINUTE;
            for (auto it = _botCooldownUntil.begin(); it != _botCooldownUntil.end();)
                if (it->second <= now)
                    it = _botCooldownUntil.erase(it);
                else
                    ++it;
            for (auto it = _senderCooldownUntil.begin(); it != _senderCooldownUntil.end();)
                if (it->second <= now)
                    it = _senderCooldownUntil.erase(it);
                else
                    ++it;
        }
    }

    void CustomSocial::Remove(uint32 botGuid)
    {
        _eligibleBots.erase(botGuid);
        _botCooldownUntil.erase(botGuid);
        _activeDialogues.erase(std::remove_if(_activeDialogues.begin(), _activeDialogues.end(),
            [botGuid](ActiveDialogue const& entry)
            {
                return entry.First == botGuid || entry.Second == botGuid;
            }), _activeDialogues.end());
    }

    bool CustomSocial::IsConversing(uint32 botGuid) const
    {
        return IsBusy(botGuid);
    }
}
