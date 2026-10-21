/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeQuesting.h"

#include <array>
#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Engine.h"
#include "GameTime.h"
#include "Map.h"
#include "NewRpgBaseAction.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "Random.h"
#include "Timer.h"

namespace
{
    constexpr uint32 QuestSearchInterval = 5 * IN_MILLISECONDS;
    constexpr float WaypointArrivalDistance = 8.0f;

    uint32 GameTimeSeconds() { return static_cast<uint32>(GameTime::GetGameTime().count()); }

    bool IsInsideArea(CityLife::QuestingArea const& area, uint32 mapId, float x, float y)
    {
        float deltaX = x - area.X;
        float deltaY = y - area.Y;
        return mapId == area.MapId && deltaX * deltaX + deltaY * deltaY <= area.Radius * area.Radius;
    }

    struct LocalQuestInfo
    {
        bool Valid = false;
        bool HasReward = false;
        uint32 ObjectiveMask = 0;
    };

    class QuestingListener final : public ActionExecutionListener, private NewRpgBaseAction
    {
    public:
        QuestingListener(PlayerbotAI* ai, std::shared_ptr<CityLife::QuestingControl> control)
            : NewRpgBaseAction(ai, "city life quest control"), _control(std::move(control))
        {
            _control->Attached.store(true);
        }

        ~QuestingListener() override { _control->Attached.store(false); }

        bool Before(Action* action, Event /*event*/) override
        {
            _intercepted = false;
            _didWork = false;
            ResetDuration();
            if (!action || action->getName() != "new rpg status update")
                return true;

            std::shared_ptr<CityLife::QuestingArea const> area = _control->Area.load();
            if (!area)
                return true;

            _intercepted = true;
            if (!bot->IsInWorld() || !bot->IsAlive() || bot->IsBeingTeleported() || bot->IsInCombat() ||
                bot->IsInFlight() || bot->GetTransport() || bot->GetVehicle() || bot->IsNonMeleeSpellCast(false) ||
                ::IsRealPlayer(bot) || botAI->HasGameClientMaster() || bot->duel || bot->InBattleground() ||
                bot->InBattlegroundQueue())
                return false;

            if (area != _area)
            {
                _area = std::move(area);
                _questInfo.clear();
                _lastSearchAt = 0;
                _hasActivity = false;
                _waypointSelected = false;
            }

            bool hasActiveQuest = false;
            if (auto* quest = std::get_if<NewRpgInfo::DoQuest>(&botAI->rpgInfo.data))
            {
                Quest const* questTemplate = sObjectMgr->GetQuestTemplate(quest->questId);
                if (IsUsableQuest(questTemplate, bot->GetQuestStatus(quest->questId)) &&
                    (quest->pos == WorldPosition() ||
                     IsInsideArea(*_area, quest->pos.GetMapId(), quest->pos.GetPositionX(), quest->pos.GetPositionY())))
                {
                    quest->quest = questTemplate;
                    ObserveQuestActivity(questTemplate);
                    hasActiveQuest = true;
                }
            }

            if (!hasActiveQuest && botAI->rpgInfo.GetStatus() != RPG_IDLE)
                botAI->rpgInfo.ChangeToIdle();

            if (!IsInsideArea(*_area, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY()) ||
                bot->GetZoneId() != _area->ZoneId)
            {
                if (bot->GetMapId() == _area->MapId)
                {
                    _didWork = MoveFarTo(WorldPosition(_area->MapId, _area->X, _area->Y, _area->Z));
                }
                return false;
            }

            if (hasActiveQuest)
                return false;

            if (_lastSearchAt && GetMSTimeDiffToNow(_lastSearchAt) < QuestSearchInterval)
                return false;
            _lastSearchAt = getMSTime();

            if (Quest const* quest = SelectQuest())
            {
                botAI->rpgInfo.ChangeToDoQuest(quest->GetQuestId(), quest);
                ObserveQuestActivity(quest);
                return false;
            }

            // New RPG may accept hand-off quests; only validated local quests get objective movement here.
            _didWork = SearchQuestGiverAndAcceptOrReward();
            if (_didWork)
                ObserveQuestActivity(nullptr);
            else
                _didWork = VisitQuestingWaypoint();
            return false;
        }

        bool AllowExecution(Action* /*action*/, Event /*event*/) override { return true; }

        void After(Action* /*action*/, bool /*executed*/, Event /*event*/) override {}

        bool OverrideResult(Action* /*action*/, bool executed, Event /*event*/) override
        {
            if (_intercepted && _didWork && GetDuration() > 0)
                botAI->SetActionDuration(GetDuration());
            // A status-only change must not consume the tick needed by the normal quest action.
            return _intercepted ? _didWork : executed;
        }

    private:
        void ObserveQuestActivity(Quest const* quest)
        {
            std::array<uint32, QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT + 2> progress{};
            if (quest)
            {
                uint32 const questId = quest->GetQuestId();
                progress[0] = questId;
                progress[1] = static_cast<uint32>(bot->GetQuestStatus(questId));
                auto status = bot->getQuestStatusMap().find(questId);
                if (status != bot->getQuestStatusMap().end())
                {
                    for (uint32 index = 0; index < QUEST_OBJECTIVES_COUNT; ++index)
                        progress[index + 2] = status->second.CreatureOrGOCount[index];
                    for (uint32 index = 0; index < QUEST_ITEM_OBJECTIVES_COUNT; ++index)
                        progress[QUEST_OBJECTIVES_COUNT + index + 2] = status->second.ItemCount[index];
                }
            }
            float const deltaX = bot->GetPositionX() - _lastActivityX;
            float const deltaY = bot->GetPositionY() - _lastActivityY;
            bool const moved = bot->GetMapId() != _lastActivityMap || deltaX * deltaX + deltaY * deltaY >= 25.0f;
            if (!_hasActivity || moved || progress != _lastProgress ||
                botAI->rpgStatistic.questAccepted != _lastAccepted ||
                botAI->rpgStatistic.questRewarded != _lastRewarded)
            {
                _control->LastActivityAt.store(GameTimeSeconds());
                _lastProgress = progress;
                _lastAccepted = botAI->rpgStatistic.questAccepted;
                _lastRewarded = botAI->rpgStatistic.questRewarded;
                _lastActivityMap = bot->GetMapId();
                _lastActivityX = bot->GetPositionX();
                _lastActivityY = bot->GetPositionY();
                _hasActivity = true;
            }
        }

        bool VisitQuestingWaypoint()
        {
            Map* map = bot->GetMap();
            if (!map || _area->Waypoints.empty())
                return false;
            if (!_waypointSelected)
            {
                _waypointIndex = urand(0, static_cast<uint32>(_area->Waypoints.size() - 1));
                _waypointSelected = true;
            }

            for (size_t checked = 0; checked < _area->Waypoints.size(); ++checked)
            {
                CityLife::QuestingWaypoint const& point = _area->Waypoints[_waypointIndex];
                bool valid = std::isfinite(point.X) && std::isfinite(point.Y) && std::isfinite(point.Z) &&
                             IsInsideArea(*_area, _area->MapId, point.X, point.Y) &&
                             map->GetZoneId(bot->GetPhaseMask(), point.X, point.Y, point.Z) == _area->ZoneId;
                if (valid && bot->GetDistance2d(point.X, point.Y) > WaypointArrivalDistance)
                    return MoveFarTo(WorldPosition(_area->MapId, point.X, point.Y, point.Z));

                _waypointIndex = (_waypointIndex + 1) % _area->Waypoints.size();
            }
            return false;
        }

        LocalQuestInfo const& GetLocalQuestInfo(Quest const* quest)
        {
            uint32 questId = quest->GetQuestId();
            auto found = _questInfo.find(questId);
            if (found != _questInfo.end())
                return found->second;

            LocalQuestInfo info;
            if (quest->GetZoneOrSort() != static_cast<int32>(_area->ZoneId))
                return _questInfo.emplace(questId, info).first->second;

            QuestPOIVector const* points = sObjectMgr->GetQuestPOIVector(questId);
            if (!points)
                return _questInfo.emplace(questId, info).first->second;

            bool hasPoints = false;
            for (QuestPOI const& poi : *points)
            {
                if (poi.points.empty())
                    continue;
                if (poi.MapId != _area->MapId)
                    return _questInfo.emplace(questId, LocalQuestInfo{}).first->second;

                for (QuestPOIPoint const& point : poi.points)
                    if (!IsInsideArea(*_area, poi.MapId, static_cast<float>(point.x), static_cast<float>(point.y)))
                        return _questInfo.emplace(questId, LocalQuestInfo{}).first->second;

                hasPoints = true;
                info.HasReward = info.HasReward || poi.ObjectiveIndex == -1;
                if (poi.ObjectiveIndex >= 0 &&
                    poi.ObjectiveIndex < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
                    info.ObjectiveMask |= uint32{1} << poi.ObjectiveIndex;
            }
            info.Valid = hasPoints && info.HasReward;
            return _questInfo.emplace(questId, info).first->second;
        }

        bool IsUsableQuest(Quest const* quest, QuestStatus status)
        {
            if (!quest || (status != QUEST_STATUS_INCOMPLETE && status != QUEST_STATUS_COMPLETE) ||
                botAI->lowPriorityQuest.contains(quest->GetQuestId()) || !IsQuestWorthDoing(quest) ||
                !IsQuestCapableDoing(quest))
                return false;

            LocalQuestInfo const& info = GetLocalQuestInfo(quest);
            if (!info.Valid)
                return false;
            if (status == QUEST_STATUS_COMPLETE)
                return true;

            auto progress = bot->getQuestStatusMap().find(quest->GetQuestId());
            if (progress == bot->getQuestStatusMap().end())
                return false;
            for (uint32 index = 0; index < QUEST_OBJECTIVES_COUNT; ++index)
                if ((info.ObjectiveMask & (uint32{1} << index)) && quest->RequiredNpcOrGo[index] &&
                    progress->second.CreatureOrGOCount[index] < quest->RequiredNpcOrGoCount[index])
                    return true;
            for (uint32 index = 0; index < QUEST_ITEM_OBJECTIVES_COUNT; ++index)
                if ((info.ObjectiveMask & (uint32{1} << (QUEST_OBJECTIVES_COUNT + index))) &&
                    quest->RequiredItemId[index] && progress->second.ItemCount[index] < quest->RequiredItemCount[index])
                    return true;
            return false;
        }

        Quest const* SelectQuest()
        {
            std::vector<Quest const*> incomplete;
            std::vector<Quest const*> complete;
            for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                uint32 questId = bot->GetQuestSlotQuestId(slot);
                if (!questId)
                    continue;
                Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
                QuestStatus status = bot->GetQuestStatus(questId);
                if (!IsUsableQuest(quest, status))
                    continue;

                if (status == QUEST_STATUS_COMPLETE)
                    complete.push_back(quest);
                else
                    incomplete.push_back(quest);
            }
            std::vector<Quest const*> const& candidates = complete.empty() ? incomplete : complete;
            if (candidates.empty())
                return nullptr;
            return candidates[urand(0, static_cast<uint32>(candidates.size() - 1))];
        }

        std::shared_ptr<CityLife::QuestingControl> _control;
        std::shared_ptr<CityLife::QuestingArea const> _area;
        std::unordered_map<uint32, LocalQuestInfo> _questInfo;
        uint32 _lastSearchAt = 0;
        size_t _waypointIndex = 0;
        bool _waypointSelected = false;
        bool _intercepted = false;
        bool _didWork = false;
        std::array<uint32, QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT + 2> _lastProgress{};
        uint32 _lastAccepted = 0;
        uint32 _lastRewarded = 0;
        uint32 _lastActivityMap = 0;
        float _lastActivityX = 0.0f;
        float _lastActivityY = 0.0f;
        bool _hasActivity = false;
    };
}  // namespace

namespace CityLife
{
    std::shared_ptr<QuestingControl> StartQuesting(Player* player, QuestingArea const& area,
                                                   std::weak_ptr<QuestingControl> const& previous)
    {
        PlayerbotAI* ai = player ? GET_PLAYERBOT_AI(player) : nullptr;
        if (!ai || ai->GetState() != BOT_STATE_NON_COMBAT || !ai->GetCurrentEngine() || !area.ZoneId ||
            !std::isfinite(area.X) || !std::isfinite(area.Y) || !std::isfinite(area.Z) || !std::isfinite(area.Radius) ||
            area.Radius <= 0.0f)
            return nullptr;

        std::shared_ptr<QuestingControl> control = previous.lock();
        if (!control || !control->Attached.load())
        {
            control = std::make_shared<QuestingControl>();
            ai->GetCurrentEngine()->AddActionExecutionListener(new QuestingListener(ai, control));
        }
        control->LastActivityAt.store(GameTimeSeconds());
        control->Area.store(std::make_shared<QuestingArea const>(area));
        return control;
    }

    void StopQuesting(std::shared_ptr<QuestingControl> const& control)
    {
        if (control)
            control->Area.store(nullptr);
    }
}  // namespace CityLife
