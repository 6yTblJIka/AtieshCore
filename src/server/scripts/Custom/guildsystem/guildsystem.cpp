/*
 * Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
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

#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "RBAC.h"
#include "Player.h"
#include "Map.h"
#include "WorldSession.h"

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

class GuildSystem_player : public PlayerScript
{
public:
    GuildSystem_player() : PlayerScript("GuildSystem_player") { }

    void OnGiveXP(Player* player, uint32& amount, Unit* /*victim*/) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        if (Guild* guildtarget = player->GetGuild())
        {
            if (guildtarget->GetGuildLevel() >= 1)
                amount *= sWorld->customGetRate(RATE_GSYSTEM_BONUS_EXP);
        }
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (Guild* guildtarget = player->GetGuild())
        {
            if (sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
                player->AddGuildAurasForPlr(guildtarget->GetGuildLevel());

            if (sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
            {
                if (sGuildMgr->GuildHasWarState(guildtarget->GetId()))
                {
                    if (!player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
                        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
                    player->InitPvP();
                }
            }
        }
    }

    void OnLogout(Player* player) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        if (player->GetGuild())
            player->RemoveGuildAurasForPlr();
    }
};

class GuildSystem : public GuildScript
{
public:
    GuildSystem() : GuildScript("GuildSystem") { }

    void EnteredInGuildWar(Guild* guild, std::string const& guildName) override
    {
        guild->BroadcastToGuildEnteredInGWWith(guildName);
    }

    void LeftInGuildWar(Guild* guild, int32 ratingChange, std::string const& guildName, std::string const& winnerguildName) override
    {
        guild->BroadcastToGuildEndedGWWith(guildName, winnerguildName, ratingChange);
    }

    void OnAddMember(Guild* guild, Player* player, uint8& /*plRank*/) override
    {
        if (sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            player->AddGuildAurasForPlr(guild->GetGuildLevel());

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
            return;

        if (sGuildMgr->GuildHasWarState(guild->GetId()))
        {
            if (!player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
                player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
            player->InitPvP();
            sGuildMgr->UpdateWarFlagForAllEnemiesAndThis(guild->GetId());
        }
    }

    void OnRemoveMember(Guild* guild, Player* player, bool /*isDisbanding*/, bool /*isKicked*/) override
    {
        if (sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            player->RemoveGuildAurasForPlr();

        if (sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
        {
            if (sGuildMgr->GuildHasWarState(guild->GetId()))
                sGuildMgr->UpdateWarFlagForAllEnemiesAndThis(guild->GetId());
        }
    }

    void OnLevelUp(Guild* guild, Player* player, uint32 receivedLevel) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        std::string name = "";
        if (player)
            name = player->GetName();
        guild->BroadcastToGuildLevelUp(receivedLevel, name.c_str());
        guild->CastGuildLevelAuras(receivedLevel);
        guild->UpdateQueryStateForPlayers();
    }

    void OnLevelDown(Guild* guild, Player* player, uint32 newLevel, uint32 lost) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        std::string name = "";
        if (player)
            name = player->GetName();
        guild->BroadcastToGuildLevelDown(newLevel, lost, name.c_str());
        guild->RemoveGuildLevelAuras();
        guild->UpdateQueryStateForPlayers();
    }

    void OnExpReceived(Guild* guild, Player* player, uint32 receivedExp) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        std::string name = "";
        if (player)
            name = player->GetName();
        guild->BroadcastToGuildExp(receivedExp, name);
    }

    void OnRatingReceived(Guild* guild, Player* player, uint32 receivedRating) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        std::string name = "";
        if (player)
            name = player->GetName();
        guild->BroadcastToGuildExp(receivedRating, name);
    }

    void OnArenaWon(Guild* guild, Player* player) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        uint32 receivedExp = sWorld->customGetIntConfig(CONFIG_GSYSTEM_ARENA_EXP);
        guild->AddGuildExp(receivedExp, player);
    }

    void OnBattlegroundWon(Guild* guild, Player* player) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        uint32 receivedExp = sWorld->customGetIntConfig(CONFIG_GSYSTEM_BG_EXP);
        guild->AddGuildExp(receivedExp, player);
    }

    void OnLFGComplete(Guild* guild, Player* player) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
            return;

        uint32 receivedExp = sWorld->customGetIntConfig(CONFIG_GSYSTEM_LFG_EXP);
        guild->AddGuildExp(receivedExp, player);
    }

    void OnKillGuildEnemyEvent(Guild* guild, Player* killer) override
    {
        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED) || !sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
            return;

        uint32 receivedExp = sWorld->customGetIntConfig(CONFIG_GSYSTEM_GW_KILL_EXP);
        guild->AddGuildExp(receivedExp, killer);
    }
};

class guildprogress_commandscript : public CommandScript
{
public:
    guildprogress_commandscript() : CommandScript("guildprogress_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> warCommandTable =
        {
            { "start",          rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleStartWarGuildProgressCommand, "" },
            { "stop",           rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleStopWarGuildProgressCommand,  "" },
            { "",               rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleWarGuildProgressCommand,      "" },
        };
        static std::vector<ChatCommand> guildProgressCommandTable =
        {
            { "addexperience",  rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleGuildAddExperienceCommand, "" },
            { "addlevel",       rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleGuildAddLevelCommand,      "" },
            { "removelevel",    rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleGuildRemoveLevelCommand,   "" },
            { "repair",         rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleGuildRepairCommand,        "" },
            { "mybank",         rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleGuildMyBankCommand,        "" },
            { "war",            rbac::RBAC_PERM_GSYSTEM_COMMANDS,     false, nullptr,           "", warCommandTable },
            { "",               rbac::RBAC_PERM_GSYSTEM_COMMANDS,     true, &HandleGuildProgressCommand,      "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "gprogress",  rbac::RBAC_PERM_GSYSTEM_COMMANDS,  true, nullptr, "", guildProgressCommandTable },
        };
        return commandTable;
    }

    static bool HandleGuildProgressCommand(ChatHandler* handler, char const* args)
    {
        Guild* guild = nullptr;

        if (args && args[0] != '\0')
        {
            if (isNumeric(args))
            {
                uint32 guildId = uint32(atoi(args));
                guild = sGuildMgr->GetGuildById(guildId);
            }
            else
            {
                std::string guildName = args;
                guild = sGuildMgr->GetGuildByName(guildName);
            }
        }
        else if (Player * target = handler->getSelectedPlayerOrSelf())
            guild = target->GetGuild();

        if (!guild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_DISABLED);
            return true;
        }

        // Display Guild Information
        handler->PSendSysMessage(LANG_GUILD_INFO_NAME, guild->GetName(), guild->GetId()); // Guild Id + Name
        handler->PSendSysMessage(LANG_GSYSTEM_ANNOUNCE_INFO, guild->GetGuildLevel(), guild->GetGuildExperience(), guild->GetGuildRating());
        return true;
    }

    static bool HandleGuildAddLevelCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        Guild* targetGuild = target->GetGuild();
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_DISABLED);
            return true;
        }

        char* value = strtok((char*)args, " ");
        int32 addedlvl = 1;
        if (value)
            addedlvl = atoi(value);

        if (addedlvl < 0)
            return false;

        targetGuild->AddGuildLevel(uint32(addedlvl), nullptr);
        handler->PSendSysMessage(LANG_GSYSTEM_GM_RECEIVED_LEVEL, targetGuild->GetName(), uint32(addedlvl), targetGuild->GetGuildLevel());
        return true;
    }

    static bool HandleGuildRemoveLevelCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        Guild* targetGuild = target->GetGuild();
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_DISABLED);
            return true;
        }

        char* value = strtok((char*)args, " ");
        int32 removedlvl = 1;
        if (value)
            removedlvl = atoi(value);

        targetGuild->RemoveGuildLevel(removedlvl, target);
        handler->PSendSysMessage(LANG_GSYSTEM_GM_LOOSED_LEVEL, targetGuild->GetName(), removedlvl, targetGuild->GetGuildLevel());
        return true;
    }

    static bool HandleGuildAddExperienceCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        Guild* targetGuild = target->GetGuild();
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_DISABLED);
            return true;
        }

        char* value = strtok((char*)args, " ");
        int32 addedExp = 1;
        if (value)
            addedExp = atoi(value);

        if (addedExp < 0)
            return false;

        targetGuild->AddGuildExp(uint32(addedExp), nullptr);
        handler->PSendSysMessage(LANG_GSYSTEM_GM_RECEIVED_EXP, targetGuild->GetName(), uint32(addedExp), targetGuild->GetGuildExperience());
        return true;
    }

    static bool HandleGuildRepairCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* targetGuild = player->GetGuild();
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_DISABLED);
            return true;
        }

        uint32 needlvl = sWorld->customGetIntConfig(CONFIG_GSYSTEM_LVL_FOR_REPAIR);

        if (targetGuild->GetGuildLevel() < needlvl)
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GW_REQ_LEVEL, needlvl);
            return true;
        }

        player->DurabilityRepairAll(false, 0, false);
        handler->PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetNameLink(player));
        return true;
    }

    static bool HandleGuildMyBankCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* targetGuild = player->GetGuild();
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_LEVEL_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_DISABLED);
            return true;
        }

        uint32 needlvl = sWorld->customGetIntConfig(CONFIG_GSYSTEM_LVL_FOR_BANK);

        if (targetGuild->GetGuildLevel() < needlvl)
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GW_REQ_LEVEL, needlvl);
            return true;
        }

        if (player->IsInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->IsInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->GetMap()->IsBattlegroundOrArena())
        {
            handler->SendSysMessage(LANG_VIP_BG);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->HasStealthAura())
        {
            handler->SendSysMessage(LANG_VIP_STEALTH);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->isDead() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        {
            handler->SendSysMessage(LANG_VIP_DEAD);
            handler->SetSentErrorMessage(true);
            return true;
        }

        handler->GetSession()->SendShowBank(player->GetGUID());
        return true;
    }

    static bool HandleWarGuildProgressCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* targetGuild = player->GetGuild();
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_WARS_DISABLED);
            return true;
        }

        // Display Guild Information
        if (!sGuildMgr->GuildHasWarState(targetGuild->GetId()))
            handler->PSendSysMessage(LANG_GSYSTEM_GW_NO_ENEMY);
        else
            handler->PSendSysMessage(LANG_GSYSTEM_GW_HAS_ENEMY, sGuildMgr->GetGuildEnemy(targetGuild->GetId())); // Extra Information
        return true;
    }

    static bool HandleStartWarGuildProgressCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* ownGuild = player->GetGuild();
        if (!ownGuild)
            return false;

        if (!*args)
            return false;

        char* guildStr = handler->extractQuotedArg((char*)args);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;

        Guild* targetGuild = sGuildMgr->GetGuildByName(guildName);
        if (!targetGuild)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_WARS_DISABLED);
            return true;
        }

        if (!ownGuild->CanStartGuildWarByGuildRights(player->GetSession()))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GW_REQ_GUILDRIGHTS);
            return true;
        }

        if (ownGuild->GetGuildFaction() != targetGuild->GetGuildFaction())
        {
            if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_BETWEEN_TWO_FACTION_ENABLED))
            {
                handler->PSendSysMessage(LANG_GSYSTEM_GW_REQ_ONEFACTION);
                return true;
            }
        }

        if (sGuildMgr->IsGuildsInWar(ownGuild->GetId(), targetGuild->GetId()))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GW_ALREADY_ENEMY, guildName);
            return true;
        }

        std::string msg;
        if (!ownGuild->CanStartGuildWarByCount(player->GetSession(), msg, false))
        {
            handler->PSendSysMessage("{}", msg);
            return true;
        }

        if (!ownGuild->CanStartGuildWarByTimer(player->GetSession(), msg))
        {
            handler->PSendSysMessage("{}", msg);
            return true;
        }

        if (!targetGuild->CanStartGuildWarByCount(player->GetSession(), msg, true))
        {
            handler->PSendSysMessage("{}", msg);
            return true;
        }

        GuildWars data;
        data.attackerGuildId = ownGuild->GetId();
        data.defenderGuildId = targetGuild->GetId();
        if (sGuildMgr->StartNewWar(data))
            handler->PSendSysMessage(LANG_GSYSTEM_GW_START, guildName);

        return true;
    }

    static bool HandleStopWarGuildProgressCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* ownGuild = player->GetGuild();
        if (!ownGuild)
            return false;

        if (!*args)
            return false;

        char* guildStr = handler->extractQuotedArg((char*)args);
        if (!guildStr)
            return false;

        if (!sWorld->customGetBoolConfig(CONFIG_GSYSTEM_GUILDWARS_ENABLED))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GUILD_WARS_DISABLED);
            return true;
        }

        std::string guildName = guildStr;

        Guild* targetGuild = sGuildMgr->GetGuildByName(guildName);
        if (!targetGuild)
            return false;

        if (!sGuildMgr->IsGuildsInWar(ownGuild->GetId(), targetGuild->GetId()))
        {
            handler->PSendSysMessage(LANG_GSYSTEM_GW_NOT_ENEMY, guildName);
            return true;
        }

        sGuildMgr->StopWarBetween(ownGuild->GetId(), targetGuild->GetId(), targetGuild->GetId());
        handler->PSendSysMessage(LANG_GSYSTEM_GW_STOP, guildName);

        return true;
    }
};

void AddSC_GuildSystem()
{
    new GuildSystem_player();
    new GuildSystem();
    new guildprogress_commandscript();
}
