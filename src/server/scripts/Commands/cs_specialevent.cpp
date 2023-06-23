/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
Name: event_commandscript
%Complete: 100
Comment: All event related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "SpecialEvent.h"
#include "SpecialEventMgr.h"
#include "GameTime.h"
#include "Language.h"
#include "Player.h"
#include "RBAC.h"

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

class specialevent_commandscript : public CommandScript
{
public:
    specialevent_commandscript() : CommandScript("specialevent_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> specialeventCommandTable =
        {
            { "start",      rbac::RBAC_PERM_COMMAND_EVENT_START,      true, &HandleEventStartCommand,      "" },
            { "stop",       rbac::RBAC_PERM_COMMAND_EVENT_STOP,       true, &HandleEventStopCommand,       "" },
            { "info",       rbac::RBAC_PERM_COMMAND_EVENT_INFO,       true, &HandleEventInfoCommand,       "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "specialevent", rbac::RBAC_PERM_COMMAND_EVENT_INFO, false, nullptr, "", specialeventCommandTable },
        };
        return commandTable;
    }

    static std::string NextTimeEvent(time_t possible)
    {
        return fmt::format("({})", TimeToHumanReadable(possible));
    }

    static std::string PrepareText(bool enabled, bool active, bool repeat, uint32 cooldown, uint32 duration, std::string nexttime)
    {
        std::string enableStatus = enabled ? "Enabled" : "Disabled";
        std::string activeStatus = active ? "is Active" : "Not Activated";
        std::string repeatStatus = repeat ? "is Repeatable" : "is not Repeatable";
        std::string nexttexttime = repeat ? nexttime : "None";
        return fmt::format("{}, {}, {}, cooldown time = {}, duration time = {}, The next available time: {}", enableStatus, activeStatus, repeatStatus, cooldown, duration, nexttexttime);
    }

    static bool HandleEventInfoCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charid = strtok((char*)args, " ");
        if (!charid)
            return false;
        uint32 id = atoul(charid);
        if (!id)
            return false;

        if (SpecialEvent* Event = sSpecialEventMgr->GetSpecialEventByEventId(id))
        {
            std::string eventcomment = Event->GetComment();
            std::string text = PrepareText(Event->IsEnabled(), Event->IsActiveTime(), Event->IsRepeatable(), (Event->GetCooldownTimer() / MINUTE) / IN_MILLISECONDS, (Event->GetDurationTimer() / MINUTE) / IN_MILLISECONDS, NextTimeEvent(Event->GetTimeOfNextEvent()));

            handler->PSendSysMessage(LANG_SPECIALEVENT_INFO, eventcomment, text);
        }

        return true;
    }

    static bool HandleEventStartCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charid = strtok((char*)args, " ");
        if (!charid)
            return false;
        uint32 id = atoul(charid);
        if (!id)
            return false;

        if (SpecialEvent* Event = sSpecialEventMgr->GetEnabledSpecialEventByEventId(id))
        {
            Event->StartSpecialEvent();
            handler->PSendSysMessage(LANG_SPECIALEVENT_START, Event->GetComment());
        }
        return true;
    }

    static bool HandleEventStopCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charid = strtok((char*)args, " ");
        if (!charid)
            return false;
        uint32 id = atoul(charid);
        if (!id)
            return false;

        if (SpecialEvent* Event = sSpecialEventMgr->GetEnabledSpecialEventByEventId(id))
        {
            Event->EndSpecialEvent(false);
            handler->PSendSysMessage(LANG_SPECIALEVENT_STOP, Event->GetComment());
        }
        return true;
    }
};

void AddSC_specialevent_commandscript()
{
    new specialevent_commandscript();
}
