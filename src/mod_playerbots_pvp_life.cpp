/*
 * PvP Life
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "PvPLifeMgr.h"

#include "Chat.h"
#include "CommandScript.h"
#include "ScriptMgr.h"
#include "WorldScript.h"

using namespace Acore::ChatCommands;

class PvPLifeWorldScript : public WorldScript
{
public:
    PvPLifeWorldScript()
        : WorldScript("PvPLifeWorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_UPDATE })
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sPvPLifeMgr.LoadConfig();
    }

    void OnUpdate(uint32 diff) override
    {
        sPvPLifeMgr.Update(diff);
    }
};

class PvPLifeCommandScript : public CommandScript
{
public:
    PvPLifeCommandScript() : CommandScript("PvPLifeCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "pvplife", HandlePvPLifeCommand, SEC_GAMEMASTER, Console::Yes }
        };
        return commandTable;
    }

    static bool HandlePvPLifeCommand(ChatHandler* handler, char const* args)
    {
        return sPvPLifeMgr.HandleCommand(handler, args ? args : "");
    }
};

void AddPvPLifeScripts()
{
    new PvPLifeWorldScript();
    new PvPLifeCommandScript();
    LOG_INFO("server.loading", ">> Loaded City Life PvP subsystem");
}
