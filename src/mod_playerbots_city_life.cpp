/*
 * City Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CityLifeMgr.h"

#include "Chat.h"
#include "CommandScript.h"
#include "ScriptMgr.h"
#include "WorldScript.h"

using namespace Acore::ChatCommands;

class CityLifeWorldScript : public WorldScript
{
public:
    CityLifeWorldScript()
        : WorldScript("CityLifeWorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_UPDATE })
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sCityLifeMgr.LoadConfig();
    }

    void OnUpdate(uint32 diff) override
    {
        sCityLifeMgr.Update(diff);
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
    new CityLifeCommandScript();
    LOG_INFO("server.loading", ">> Loaded mod-playerbots-city-life");
}
