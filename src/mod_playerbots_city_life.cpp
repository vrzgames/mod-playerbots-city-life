/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeMgr.h"
#include "CityLifeBeggarTrade.h"
#include "CityLifeCustomActivities.h"
#include "CityLifeCustomAppearance.h"
#include "CityLifeCustomBotPool.h"
#include "CityLifeCustomSocial.h"
#include "LifeBotReservation.h"

#include "Chat.h"
#include "CommandScript.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "WorldScript.h"

using namespace Acore::ChatCommands;

namespace
{
    bool IsCustomReserved(Player const* player)
    {
        return player && PlayerbotsLife::IsReservedBy(player->GetGUID().GetCounter(),
            PlayerbotsLife::ReservationOwner::CustomLife);
    }
}

class CityLifeWorldScript : public WorldScript
{
public:
    CityLifeWorldScript()
        : WorldScript("CityLifeWorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_UPDATE,
            WORLDHOOK_ON_STARTUP, WORLDHOOK_ON_SHUTDOWN })
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        if (_started)
            sCityLifeMgr.LoadConfig();
    }

    void OnStartup() override
    {
        // Item templates, creature scripts and character creation data are available only now.
        _started = true;
        sCityLifeMgr.LoadConfig();
    }

    void OnUpdate(uint32 diff) override
    {
        if (_started)
            sCityLifeMgr.Update(diff);
    }

    void OnShutdown() override { sCityLifeMgr.Shutdown(); }

private:
    bool _started = false;
};

class CityLifeCustomPlayerScript : public PlayerScript
{
public:
    CityLifeCustomPlayerScript() : PlayerScript("CityLifeCustomPlayerScript",
        {PLAYERHOOK_ON_TEXT_EMOTE, PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_BEFORE_LOGOUT,
            PLAYERHOOK_CAN_INIT_TRADE, PLAYERHOOK_CAN_SET_TRADE_ITEM, PLAYERHOOK_CAN_SELL_ITEM,
            PLAYERHOOK_CAN_SEND_MAIL, PLAYERHOOK_CAN_GROUP_ACCEPT, PLAYERHOOK_CAN_JOIN_LFG,
            PLAYERHOOK_CAN_JOIN_IN_BATTLEGROUND_QUEUE, PLAYERHOOK_CAN_JOIN_IN_ARENA_QUEUE}) { }

    void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 /*emoteNum*/, ObjectGuid guid) override
    {
        sCityLifeCustomSocial.OnTextEmote(player, textEmote, guid);
    }

    void OnPlayerLogin(Player* player) override
    {
        // Recovery remains active even when City Life or Custom is switched off.
        if (!sCityLifeCustomAppearance.RecoverOnLogin(player))
            sCityLifeCustomActivities.RecoverPending();
        sCityLifeCustomBotPool.OnLogin(player);
    }

    void OnPlayerBeforeLogout(Player* player) override { sCityLifeMgr.OnPlayerBeforeLogout(player); }

    bool OnPlayerCanInitTrade(Player* player, Player* target) override
    {
        if (sCityLifeBeggarTrade.IsInitiating(player, target))
            return !IsCustomReserved(target);
        return !IsCustomReserved(player) && !IsCustomReserved(target);
    }

    bool OnPlayerCanSetTradeItem(Player* player, Item* /*item*/, uint8 /*slot*/) override
    {
        return !IsCustomReserved(player);
    }

    bool OnPlayerCanSellItem(Player* player, Item* /*item*/, Creature* /*vendor*/) override
    {
        return !IsCustomReserved(player);
    }

    bool OnPlayerCanSendMail(Player* player, ObjectGuid /*receiver*/, ObjectGuid /*mailbox*/,
        std::string& /*subject*/, std::string& /*body*/, uint32 /*money*/, uint32 /*cod*/, Item* /*item*/) override
    {
        return !IsCustomReserved(player);
    }

    bool OnPlayerCanGroupAccept(Player* player, Group* /*group*/) override { return !IsCustomReserved(player); }

    bool OnPlayerCanJoinLfg(Player* player, uint8 /*roles*/, std::set<uint32>& /*dungeons*/,
        std::string const& /*comment*/) override { return !IsCustomReserved(player); }

    bool OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid /*battlemaster*/,
        BattlegroundTypeId /*type*/, uint8 /*asGroup*/, GroupJoinBattlegroundResult& /*error*/) override
    {
        return !IsCustomReserved(player);
    }

    bool OnPlayerCanJoinInArenaQueue(Player* player, ObjectGuid /*battlemaster*/, uint8 /*slot*/,
        BattlegroundTypeId /*type*/, uint8 /*asGroup*/, uint8 /*rated*/,
        GroupJoinBattlegroundResult& /*error*/) override
    {
        return !IsCustomReserved(player);
    }
};

class CityLifeCommandScript : public CommandScript
{
public:
    CityLifeCommandScript() : CommandScript("CityLifeCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "citylife", HandleCityLifeCommand, SEC_GAMEMASTER, Console::Yes }
        };
        return commandTable;
    }

    static bool HandleCityLifeCommand(ChatHandler* handler, char const* args)
    {
        return sCityLifeMgr.HandleCommand(handler, args ? args : "");
    }
};

void AddCityLifeScripts()
{
    new CityLifeWorldScript();
    new CityLifeCustomPlayerScript();
    new CityLifeCommandScript();
    LOG_INFO("server.loading", ">> Loaded mod-playerbots-city-life");
}
