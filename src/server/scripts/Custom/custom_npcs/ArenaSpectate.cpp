/*
* Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
* Copyright (C) 2016-2017 RustEmu <https://gitlab.com/healthstone/>
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "CreatureTextMgr.h"
#include "Chat.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "RBAC.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define GTS session->GetTrinityString

enum ClassTalents
{
    SPELL_DK_BLOOD                  = 62905,    // Not Dancing Rune Weapon (i used Improved Death Strike)
    SPELL_DK_FROST                  = 49143,    // Not Howling Blash (i used Frost Strike)
    SPELL_DK_UNHOLY                 = 49206,

    SPELL_DRUID_BALANCE             = 48505,
    SPELL_DRUID_FERAL_COMBAT        = 50334,
    SPELL_DRUID_RESTORATION         = 65139,    // Not Wild Growth (i used Improved Barkskin)

    SPELL_HUNTER_BEAST_MASTERY      = 53270,
    SPELL_HUNTER_MARKSMANSHIP       = 53209,
    SPELL_HUNTER_SURVIVAL           = 53301,

    SPELL_MAGE_ARCANE               = 44425,
    SPELL_MAGE_FIRE                 = 31661,    // Not Living Bomb (i used Dragon's Breath)
    SPELL_MAGE_FROST                = 44572,

    SPELL_PALADIN_HOLY              = 53563,
    SPELL_PALADIN_PROTECTION        = 53595,
    SPELL_PALADIN_RETRIBUTION       = 53385,

    SPELL_PRIEST_DISCIPLINE         = 47540,
    SPELL_PRIEST_HOLY               = 47788,
    SPELL_PRIEST_SHADOW             = 47585,

    SPELL_ROGUE_ASSASSINATION       = 1329,     // Not Hunger For Blood (i used Mutilate)
    SPELL_ROGUE_COMBAT              = 51690,
    SPELL_ROGUE_SUBTLETY            = 51713,

    SPELL_SHAMAN_ELEMENTAL          = 51490,
    SPELL_SHAMAN_ENHACEMENT         = 30823,    // Not Feral Spirit (i used Shamanistic Rage)
    SPELL_SHAMAN_RESTORATION        = 974,      // Not Riptide (i used Earth Shield)

    SPELL_WARLOCK_AFFLICTION        = 48181,
    SPELL_WARLOCK_DEMONOLOGY        = 59672,
    SPELL_WARLOCK_DESTRUCTION       = 50796,

    SPELL_WARRIOR_ARMS              = 46924,
    SPELL_WARRIOR_FURY              = 23881,    // Not Titan's Grip (i used Bloodthirst)
    SPELL_WARRIOR_PROTECTION        = 46968,
};

std::vector<Battleground*> ratedArenas;

void LoadAllArenas()
{
    ratedArenas.clear();
    for (uint8 i = 0; i <= MAX_BATTLEGROUND_TYPE_ID; ++i)
    {
        if (!sBattlegroundMgr->IsArenaType(BattlegroundTypeId(i)))
            continue;

        BattlegroundData* arenas = sBattlegroundMgr->GetAllBattlegroundsWithTypeId(BattlegroundTypeId(i));

        if (!arenas || arenas->m_Battlegrounds.empty())
            continue;

        for (BattlegroundContainer::const_iterator itr = arenas->m_Battlegrounds.begin(); itr != arenas->m_Battlegrounds.end(); ++itr)
        {
            Battleground* arena = itr->second;

            if (!arena->GetPlayersSize())
                continue;

            if (!arena->isRated())
                continue;

            ratedArenas.push_back(arena);
        }
    }

    if (ratedArenas.size() < 2)
        return;

    std::vector<Battleground*>::iterator itr = ratedArenas.begin();
    uint32 count = 0;
    for (; itr != ratedArenas.end(); ++itr)
    {
        if (!(*itr))
        {
            count++;
            continue;
        }

        // I have no idea if this event could ever happen, but if it did, it would most likely
        // cause crash
        if (count >= ratedArenas.size())
            return;

        uint32 size = ratedArenas.size();
        // Bubble sort, oh yeah, that's the stuff..
        for (uint32 i = count; i < size; i++)
        {
            if (ratedArenas[i])
            {
                Battleground* tmp = (*itr);
                (*itr) = ratedArenas[i];
                ratedArenas[i] = tmp;
            }
        }
        count++;
    }
    return;
}

uint32 GetSpecificArenasCount(ArenaType type)
{
    if (ratedArenas.empty())
        return 0;

    uint32 count = 0;

    for (std::vector<Battleground*>::const_iterator citr = ratedArenas.begin(); citr != ratedArenas.end(); ++citr)
    {
        if (Battleground* arena = (*citr))
        {
            if (arena->GetArenaType() == type)
                count++;
        }
    }

    return count;
}

class arena_spectator_commands : public CommandScript
{
    public:
        arena_spectator_commands() : CommandScript("arena_spectator_commands") { }

        static bool HandleSpectateCommand(ChatHandler* handler, char const* args)
        {
            Player* target;
            ObjectGuid target_guid;
            std::string target_name;
            if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
                return false;

            if (Player* player = handler->GetSession()->GetPlayer())
            {
                if (!target) // Prevent Crash
                {
                    handler->PSendSysMessage(LANG_SPECTATE_WRONG_NAME);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (target->GetTypeId() != TYPEID_PLAYER)
                {
                    handler->PSendSysMessage(LANG_SPECTATE_NON_PLAYER);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (target == player || target_guid == player->GetGUID())
                {
                    handler->PSendSysMessage(LANG_SPECTATE_YOUSELF);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->IsInCombat())
                {
                    handler->PSendSysMessage(LANG_YOU_IN_COMBAT);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->GetPet())
                {
                    handler->PSendSysMessage(LANG_SPECTATE_DISMIS_PET);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->GetMap()->IsBattlegroundOrArena() && !player->IsSpectator())
                {
                    handler->PSendSysMessage(LANG_NOT_USED_BG);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                Map* cMap = target->GetMap();
                if (!cMap->IsBattleArena())
                {
                    handler->PSendSysMessage(LANG_SPECTATE_PLAYER_NOT_ARENA);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->GetMap()->IsBattleground())
                {
                    handler->PSendSysMessage(LANG_NOT_USED_BG);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                player->SetBattlegroundId(target->GetBattlegroundId(), target->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!player->GetMap()->IsBattlegroundOrArena())
                    player->SetBattlegroundEntryPoint();

                if (target->IsSpectator())
                {
                    handler->PSendSysMessage(LANG_SPECTATE_CANT_SPECTATORS);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (target->HasAura(32728) || target->HasAura(32727))
                {
                    handler->PSendSysMessage(LANG_SPECTATE_WAIT_START);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->HasAura(9454))
                {
                    handler->PSendSysMessage(LANG_YOU_ARE_FROZEN);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->IsMounted()) // Prevent Bug with Pets / Minions/ Guardian for Spectators in arenas
                {
                    handler->PSendSysMessage(LANG_SPECTATE_MOUNT);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (/*player->IsInvitedForBattlegroundQueueType() || */player->InBattlegroundQueue()) // Prevent Crash
                {
                    handler->PSendSysMessage(LANG_SPECTATE_QUEUE);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                // stop flight if need
                if (player->IsInFlight())
                {
                    player->GetMotionMaster()->Clear();
                    player->CleanupAfterTaxiFlight();
                }
                // save only in non-flight case
                else
                    player->SaveRecallPosition();

                // search for two teams
                Battleground *bGround = target->GetBattleground();
                if (bGround->isRated())
                {
                    uint32 slot = bGround->GetArenaType() - 2;
                    if (bGround->GetArenaType() == ARENA_TYPE_3v3)
                        slot = 2;
                    uint32 firstTeamID = target->GetArenaTeamId(slot);
                    uint32 secondTeamID = 0;
                    Player *firstTeamMember  = target;
                    Player *secondTeamMember = NULL;
                    for (Battleground::BattlegroundPlayerMap::const_iterator itr = bGround->GetPlayers().begin(); itr != bGround->GetPlayers().end(); ++itr)
                        if (Player* tmpPlayer = ObjectAccessor::FindPlayer(itr->first))
                        {
                            if (tmpPlayer->IsSpectator())
                                continue;

                            uint32 tmpID = tmpPlayer->GetArenaTeamId(slot);
                            if (tmpID != firstTeamID && tmpID > 0)
                            {
                                secondTeamID = tmpID;
                                secondTeamMember = tmpPlayer;
                                break;
                            }
                        }

                    if (firstTeamID > 0 && secondTeamID > 0 && secondTeamMember)
                    {
                        ArenaTeam *firstTeam  = sArenaTeamMgr->GetArenaTeamById(firstTeamID);
                        ArenaTeam *secondTeam = sArenaTeamMgr->GetArenaTeamById(secondTeamID);
                        if (firstTeam && secondTeam)
                        {
                            handler->PSendSysMessage("You're entered to rated arena.");
                            handler->PSendSysMessage("Teams:");
                            handler->PSendSysMessage("Team GOLD : {} vs Team GREEN : {}", firstTeam->GetName(), secondTeam->GetName());
                            handler->PSendSysMessage("Rating {} (MMR {}) - Rating {} (MMR {})", firstTeam->GetRating(), firstTeam->GetAverageMMR(firstTeamMember->GetGroup()),
                            secondTeam->GetRating(), secondTeam->GetAverageMMR(secondTeamMember->GetGroup()));
                        }
                    }
                }

                // to point to see at target with same orientation
                float x, y, z;
                target->GetContactPoint(player, x, y, z);

                player->SetPhaseMask(target->GetPhaseMask(), true);
                player->SetSpectate(true);
                player->TeleportTo(target->GetMapId(), x, y, z, player->GetOrientation());

                return true;
            }
            else
            {
                handler->PSendSysMessage("ERROR: Can't find Player");
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        static bool HandleSpectateCancelCommand(ChatHandler* handler, const char* /*args*/)
        {
            Player* player =  handler->GetSession()->GetPlayer();

            if (!player->IsSpectator())
            {
                handler->PSendSysMessage("ERROR: You are not spectator.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            player->SetSpectate(false);
            player->TeleportToBGEntryPoint();

            return true;
        }

        static bool HandleSpectateFromCommand(ChatHandler* handler, const char *args)
        {
            Player* target;
            ObjectGuid target_guid;
            std::string target_name;
            if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
                return false;

            if (Player* player = handler->GetSession()->GetPlayer())
            {
                if (!target) // Prevent Crash
                {
                    handler->PSendSysMessage(LANG_SPECTATE_WRONG_NAME);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (target->GetTypeId() != TYPEID_PLAYER)
                {
                    handler->PSendSysMessage(LANG_SPECTATE_NON_PLAYER);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (!player->IsSpectator())
                {
                    handler->PSendSysMessage(LANG_SPECTATE_NOT_SPECTATOR);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (target->IsSpectator() && target != player)
                {
                    handler->PSendSysMessage(LANG_SPECTATE_YOU_TARGET_SPECTATOR);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->GetMap() != target->GetMap())
                {
                    handler->PSendSysMessage(LANG_SPECTATE_DIFERENT_ARENAS);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (target && (target->HasAuraType(SPELL_AURA_MOD_STEALTH) || target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY)))
                {
                    handler->PSendSysMessage(LANG_SPECTATE_TARGET_INVISEBLE);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                // check for arena preperation
                // if exists than battle didn`t begin
                if (target && (target->HasAura(32728) || target->HasAura(32727)))
                {
                    handler->PSendSysMessage(LANG_SPECTATE_WAIT_START);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->HasAura(9454))
                {
                    handler->PSendSysMessage(LANG_YOU_ARE_FROZEN);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (player->GetViewpoint())
                    player->SetViewpoint(player->GetViewpoint(), false);

                player->SetViewpoint(target, true);
                return true;
            } // Prevent Crash if not Exist Player
            else
                return false;
        }

        static bool HandleSpectateResetCommand(ChatHandler* handler, const char*)
        {
            Player* player = handler->GetSession()->GetPlayer();

            if (!player)
            {
                handler->PSendSysMessage(LANG_SPECTATE_CANT_FIND_PLAYER);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!player->IsSpectator())
            {
                handler->PSendSysMessage(LANG_SPECTATE_NOT_SPECTATOR);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (player->GetViewpoint())
                player->SetViewpoint(player->GetViewpoint(), false);

            Battleground *bGround = player->GetBattleground();
            if (!bGround)
                return false;

            if (bGround->GetStatus() != STATUS_IN_PROGRESS)
                return true;

            for (Battleground::BattlegroundPlayerMap::const_iterator itr = bGround->GetPlayers().begin(); itr != bGround->GetPlayers().end(); ++itr)
                if (Player* tmpPlayer = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (tmpPlayer->IsSpectator())
                        continue;

                    if (tmpPlayer->IsGameMaster())
                        continue;

                    uint32 tmpID = bGround->GetPlayerTeam(tmpPlayer->GetGUID());

                    // generate addon massage
                    std::string pName = tmpPlayer->GetName();
                    std::string tName = "";

                    if (Player *target = tmpPlayer->GetSelectedPlayer())
                        tName = target->GetName();

                    SpectatorAddonMsg msg;
                    msg.SetPlayer(pName);
                    if (tName != "")
                        msg.SetTarget(tName);
                    msg.SetStatus(tmpPlayer->IsAlive());
                    msg.SetClass(tmpPlayer->GetClass());
                    msg.SetCurrentHP(tmpPlayer->GetHealth());
                    msg.SetMaxHP(tmpPlayer->GetMaxHealth());
                    Powers powerType = tmpPlayer->GetPowerType();
                    msg.SetMaxPower(tmpPlayer->GetMaxPower(powerType));
                    msg.SetCurrentPower(tmpPlayer->GetPower(powerType));
                    msg.SetPowerType(powerType);
                    msg.SetTeam(tmpID);
                    msg.SendPacket(player->GetGUID());
                }

            return true;
        }

        std::vector<ChatCommand> GetCommands() const override
        {
            static std::vector<ChatCommand> spectateCommandTable =
            {
                { "player",         rbac::RBAC_PERM_COMMAND_SPECTATE_PLAYER, true,  &HandleSpectateCommand,        "" },
                { "view",           rbac::RBAC_PERM_COMMAND_SPECTATE_VIEW,      true,  &HandleSpectateFromCommand,    "" },
                { "reset",          rbac::RBAC_PERM_COMMAND_SPECTATE_RESET,      true,  &HandleSpectateResetCommand,   "" },
                { "leave",          rbac::RBAC_PERM_COMMAND_SPECTATE_LEAVE,      true,  &HandleSpectateCancelCommand,  "" },
            };
            static std::vector<ChatCommand> commandTable =
            {
                { "spectate",       rbac::RBAC_PERM_COMMAND_SPECTATE, false,  NULL, "", spectateCommandTable },
            };
            return commandTable;
        }
};

enum NpcSpectatorActions
{
    // will be used for scrolling
    NPC_SPECTATOR_ACTION_MAIN_MENU          = 1,
    NPC_SPECTATOR_ACTION_SPECIFIC           = 10,
    NPC_SPECTATOR_ACTION_2V2_GAMES          = 100,
    NPC_SPECTATOR_ACTION_3V3_GAMES          = 300,
    NPC_SPECTATOR_ACTION_5V5_GAMES          = 500,

    // NPC_SPECTATOR_ACTION_SELECTED_PLAYER + player.Guid()
    NPC_SPECTATOR_ACTION_SELECTED_PLAYER    = 700
};

const uint8 GamesOnPage                     = 15;

class npc_arena_spectator : public CreatureScript
{
public:
    npc_arena_spectator() : CreatureScript("npc_arena_spectator") { }

    struct npc_arena_spectatorAI : public ScriptedAI
    {
        npc_arena_spectatorAI(Creature* creature) : ScriptedAI(creature) {}

        bool OnGossipHello(Player* player) override
        {
            WorldSession* session = player->GetSession();
            LoadAllArenas();
            uint32 arenasQueueTotal[3] = { 0, 0, 0 };
            arenasQueueTotal[0] = GetSpecificArenasCount(ARENA_TYPE_2v2);
            arenasQueueTotal[1] = GetSpecificArenasCount(ARENA_TYPE_3v3);
            arenasQueueTotal[2] = GetSpecificArenasCount(ARENA_TYPE_5v5);

            std::stringstream Gossip2s;
            Gossip2s << GTS(LANG_SPECTATE_MENU_2VS2) << arenasQueueTotal[0] << ")"/* << arenasQueuePlaying[0] << ")"*/;
            std::stringstream Gossip3s;
            Gossip3s << GTS(LANG_SPECTATE_MENU_3VS3) << arenasQueueTotal[1] << ")"/* << arenasQueuePlaying[1] << ")"*/;
            std::stringstream Gossip3ss;
            Gossip3ss << GTS(LANG_SPECTATE_MENU_5VS5) << arenasQueueTotal[2] << ")"/* << arenasQueuePlaying[2] << ")"*/;
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Gossip2s.str(), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_2V2_GAMES);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Gossip3s.str(), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_3V3_GAMES);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Gossip3ss.str(), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_5V5_GAMES);
            //player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_CHAT, "Spectate Specific Player", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SPECIFIC, "", 0, true);
            SendGossipMenuFor(player, 71, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 uiAction) override
        {
            WorldSession* session = player->GetSession();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(uiAction);
            ClearGossipMenuFor(player);

            if (action == NPC_SPECTATOR_ACTION_MAIN_MENU)
            {
                OnGossipHello(player);
                return true;
            }

            if (action >= NPC_SPECTATOR_ACTION_2V2_GAMES && action < NPC_SPECTATOR_ACTION_3V3_GAMES)
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GTS(LANG_SPECTATE_MENU_UPDATE), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_2V2_GAMES);
                ShowPage(player, action - NPC_SPECTATOR_ACTION_2V2_GAMES, false);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            }
            else if (action >= NPC_SPECTATOR_ACTION_3V3_GAMES && action < NPC_SPECTATOR_ACTION_5V5_GAMES)
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GTS(LANG_SPECTATE_MENU_UPDATE), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_3V3_GAMES);
                ShowPage(player, action - NPC_SPECTATOR_ACTION_3V3_GAMES, true);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            }
            else if (action >= NPC_SPECTATOR_ACTION_5V5_GAMES && action < NPC_SPECTATOR_ACTION_SELECTED_PLAYER)
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GTS(LANG_SPECTATE_MENU_UPDATE), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_5V5_GAMES);
                ShowPage(player, action - NPC_SPECTATOR_ACTION_5V5_GAMES, true);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            }
            else
            {
                ObjectGuid guid = ObjectGuid(uint64(action - NPC_SPECTATOR_ACTION_SELECTED_PLAYER));
                if (Player* target = ObjectAccessor::FindPlayer(guid))
                {
                    ChatHandler handler(player->GetSession());
                    char const* pTarget = target->GetName().c_str();
                    arena_spectator_commands::HandleSpectateCommand(&handler, pTarget);
                }
            }

            return true;
        }

        std::string GetClassNameById(Player* player)
        {
            std::string sClass = "";
            switch (player->GetClass())
            {
            case CLASS_WARRIOR:
                if (player->HasTalent(SPELL_WARRIOR_ARMS, player->GetActiveSpec()))
                    sClass = "A";
                else if (player->HasTalent(SPELL_WARRIOR_FURY, player->GetActiveSpec()))
                    sClass = "F";
                else if (player->HasTalent(SPELL_WARRIOR_PROTECTION, player->GetActiveSpec()))
                    sClass = "P";
                sClass += "Warrior ";
                break;
            case CLASS_PALADIN:
                if (player->HasTalent(SPELL_PALADIN_HOLY, player->GetActiveSpec()))
                    sClass = "H";
                else if (player->HasTalent(SPELL_PALADIN_PROTECTION, player->GetActiveSpec()))
                    sClass = "P";
                else if (player->HasTalent(SPELL_PALADIN_RETRIBUTION, player->GetActiveSpec()))
                    sClass = "R";
                sClass += "Paladin ";
                break;
            case CLASS_HUNTER:
                if (player->HasTalent(SPELL_HUNTER_BEAST_MASTERY, player->GetActiveSpec()))
                    sClass = "B";
                else if (player->HasTalent(SPELL_HUNTER_MARKSMANSHIP, player->GetActiveSpec()))
                    sClass = "M";
                else if (player->HasTalent(SPELL_HUNTER_SURVIVAL, player->GetActiveSpec()))
                    sClass = "S";
                sClass += "Hunter ";
                break;
            case CLASS_ROGUE:
                if (player->HasTalent(SPELL_ROGUE_ASSASSINATION, player->GetActiveSpec()))
                    sClass = "A";
                else if (player->HasTalent(SPELL_ROGUE_COMBAT, player->GetActiveSpec()))
                    sClass = "C";
                else if (player->HasTalent(SPELL_ROGUE_SUBTLETY, player->GetActiveSpec()))
                    sClass = "S";
                sClass += "Rogue ";
                break;
            case CLASS_PRIEST:
                if (player->HasTalent(SPELL_PRIEST_DISCIPLINE, player->GetActiveSpec()))
                    sClass = "D";
                else if (player->HasTalent(SPELL_PRIEST_HOLY, player->GetActiveSpec()))
                    sClass = "H";
                else if (player->HasTalent(SPELL_PRIEST_SHADOW, player->GetActiveSpec()))
                    sClass = "S";
                sClass += "Priest ";
                break;
            case CLASS_DEATH_KNIGHT:
                if (player->HasTalent(SPELL_DK_BLOOD, player->GetActiveSpec()))
                    sClass = "B";
                else if (player->HasTalent(SPELL_DK_FROST, player->GetActiveSpec()))
                    sClass = "F";
                else if (player->HasTalent(SPELL_DK_UNHOLY, player->GetActiveSpec()))
                    sClass = "U";
                sClass += "DK ";
                break;
            case CLASS_SHAMAN:
                if (player->HasTalent(SPELL_SHAMAN_ELEMENTAL, player->GetActiveSpec()))
                    sClass = "EL";
                else if (player->HasTalent(SPELL_SHAMAN_ENHACEMENT, player->GetActiveSpec()))
                    sClass = "EN";
                else if (player->HasTalent(SPELL_SHAMAN_RESTORATION, player->GetActiveSpec()))
                    sClass = "R";
                sClass += "Shaman ";
                break;
            case CLASS_MAGE:
                if (player->HasTalent(SPELL_MAGE_ARCANE, player->GetActiveSpec()))
                    sClass = "A";
                else if (player->HasTalent(SPELL_MAGE_FIRE, player->GetActiveSpec()))
                    sClass = "Fi";
                else if (player->HasTalent(SPELL_MAGE_FROST, player->GetActiveSpec()))
                    sClass = "Fr";
                sClass += "Mage ";
                break;
            case CLASS_WARLOCK:
                if (player->HasTalent(SPELL_WARLOCK_AFFLICTION, player->GetActiveSpec()))
                    sClass = "A";
                else if (player->HasTalent(SPELL_WARLOCK_DEMONOLOGY, player->GetActiveSpec()))
                    sClass = "Dem";
                else if (player->HasTalent(SPELL_WARLOCK_DESTRUCTION, player->GetActiveSpec()))
                    sClass = "Des";
                sClass += "Warlock ";
                break;
            case CLASS_DRUID:
                if (player->HasTalent(SPELL_DRUID_BALANCE, player->GetActiveSpec()))
                    sClass = "B";
                else if (player->HasTalent(SPELL_DRUID_FERAL_COMBAT, player->GetActiveSpec()))
                    sClass = "F";
                else if (player->HasTalent(SPELL_DRUID_RESTORATION, player->GetActiveSpec()))
                    sClass = "R";
                sClass += "Druid ";
                break;
            default:
                sClass = "<Unknown>";
                break;
            }

            return sClass;
        }

        std::string GetGamesStringData(Battleground* team, uint16 mmr, uint16 mmrTwo)
        {
            std::string teamsMember[PVP_TEAMS_COUNT];
            uint32 firstTeamId = 0;
            for (Battleground::BattlegroundPlayerMap::const_iterator itr = team->GetPlayers().begin(); itr != team->GetPlayers().end(); ++itr)
            {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (player->IsSpectator())
                        continue;

                    if (player->IsGameMaster())
                        continue;

                    uint32 team = itr->second.Team;
                    if (!firstTeamId)
                        firstTeamId = team;

                    teamsMember[firstTeamId == team] += GetClassNameById(player);
                }
            }

            std::string data = teamsMember[0] + "(";
            std::stringstream ss;
            ss << mmr;
            std::stringstream sstwo;
            sstwo << mmrTwo;
            data += ss.str();
            data += ") vs ";
            data += teamsMember[1] + "(" + sstwo.str();
            data += ")";
            return data;
        }

        ObjectGuid GetFirstPlayerGuid(Battleground* team)
        {
            for (Battleground::BattlegroundPlayerMap::const_iterator itr = team->GetPlayers().begin(); itr != team->GetPlayers().end(); ++itr)
                if (ObjectAccessor::FindPlayer(itr->first))
                    return itr->first;
            return ObjectGuid::Empty;
        }

        void ShowPage(Player* player, uint16 page, bool IsTop)
        {
            WorldSession* session = player->GetSession();

            uint32 firstTeamId = 0;
            uint16 TypeTwo = 0;
            uint16 TypeThree = 0;
            uint16 TypeSolo = 0;
            uint16 mmr = 0;
            uint16 mmrTwo = 0;
            bool haveNextPage[3];
            haveNextPage[2] = false;
            haveNextPage[1] = false;
            haveNextPage[0] = false;
            for (uint8 i = 0; i <= MAX_BATTLEGROUND_TYPE_ID; ++i)
            {
                if (!sBattlegroundMgr->IsArenaType(BattlegroundTypeId(i)))
                    continue;

                BattlegroundContainer* arenas = sBattlegroundMgr->GetBattlegroundsByType(BattlegroundTypeId(i));

                if (!arenas || arenas->empty())
                    continue;

                for (BattlegroundContainer::const_iterator itr = arenas->begin(); itr != arenas->end(); ++itr)
                {
                    Battleground* arena = itr->second;
                    Player* target = ObjectAccessor::FindPlayer(GetFirstPlayerGuid(arena));
                    if (!target)
                        continue;

                    if (target->GetTypeId() == TYPEID_PLAYER && (target->HasAura(32728) || target->HasAura(32727)))
                        continue;

                    if (!arena->GetPlayersSize())
                        continue;

                    if (!arena->isRated())
                        continue;

                    if (arena->GetArenaType() == ARENA_TYPE_2v2)
                    {
                        if (ArenaTeam *targetArenaMmr = sArenaTeamMgr->GetArenaTeamById(target->GetArenaTeamId(0)))
                            mmr = targetArenaMmr->GetMember(target->GetGUID())->MatchMakerRating;
                        firstTeamId = target->GetArenaTeamId(0);
                        Battleground::BattlegroundPlayerMap::const_iterator citr = arena->GetPlayers().begin();
                        for (; citr != arena->GetPlayers().end(); ++citr)
                        {
                            if (Player* plrs = ObjectAccessor::FindPlayer(citr->first))
                            {
                                if (plrs->GetArenaTeamId(0) != firstTeamId)
                                {
                                    if (ArenaTeam *arenaMmr = sArenaTeamMgr->GetArenaTeamById(plrs->GetArenaTeamId(0)))
                                        mmrTwo = arenaMmr->GetMember(plrs->GetGUID())->MatchMakerRating;
                                    break;
                                }
                            }
                        }
                    }
                    else if (arena->GetArenaType() == ARENA_TYPE_3v3)
                    {
                        if (ArenaTeam *targetArenaMmr = sArenaTeamMgr->GetArenaTeamById(target->GetArenaTeamId(1)))
                            mmr = targetArenaMmr->GetMember(target->GetGUID())->MatchMakerRating;
                        firstTeamId = target->GetArenaTeamId(1);
                        Battleground::BattlegroundPlayerMap::const_iterator citr = arena->GetPlayers().begin();
                        for (; citr != arena->GetPlayers().end(); ++citr)
                        {
                            if (Player* plrs = ObjectAccessor::FindPlayer(citr->first))
                            {
                                if (plrs->GetArenaTeamId(1) != firstTeamId)
                                {
                                    if (ArenaTeam *arenaMmr = sArenaTeamMgr->GetArenaTeamById(plrs->GetArenaTeamId(1)))
                                        mmrTwo = arenaMmr->GetMember(plrs->GetGUID())->MatchMakerRating;
                                    break;
                                }
                            }
                        }
                    }
                    else if (arena->GetArenaType() == ARENA_TYPE_5v5)
                    {
                        if (ArenaTeam *targetArenaMmr = sArenaTeamMgr->GetArenaTeamById(target->GetArenaTeamId(2)))
                            mmr = targetArenaMmr->GetMember(target->GetGUID())->MatchMakerRating;
                        firstTeamId = target->GetArenaTeamId(2);
                        Battleground::BattlegroundPlayerMap::const_iterator citr = arena->GetPlayers().begin();
                        for (; citr != arena->GetPlayers().end(); ++citr)
                        {
                            if (Player* plrs = ObjectAccessor::FindPlayer(citr->first))
                            {
                                if (plrs->GetArenaTeamId(2) != firstTeamId)
                                {
                                    if (ArenaTeam *arenaMmr = sArenaTeamMgr->GetArenaTeamById(plrs->GetArenaTeamId(2)))
                                        mmrTwo = arenaMmr->GetMember(plrs->GetGUID())->MatchMakerRating;
                                    break;
                                }
                            }
                        }
                    }

                    if (IsTop && arena->GetArenaType() == ARENA_TYPE_3v3)
                    {
                        TypeThree++;
                        if (TypeThree > (page + 1) * GamesOnPage)
                        {
                            haveNextPage[1] = true;
                            break;
                        }

                        if (TypeThree >= page * GamesOnPage)
                            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, GetGamesStringData(arena, mmr, mmrTwo), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SELECTED_PLAYER + GetFirstPlayerGuid(arena));
                    }
                    else if (!IsTop && arena->GetArenaType() == ARENA_TYPE_2v2)
                    {
                        TypeTwo++;
                        if (TypeTwo > (page + 1) * GamesOnPage)
                        {
                            haveNextPage[0] = true;
                            break;
                        }

                        if (TypeTwo >= page * GamesOnPage)
                            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, GetGamesStringData(arena, mmr, mmrTwo), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SELECTED_PLAYER + GetFirstPlayerGuid(arena));
                    }
                    else if (IsTop && arena->GetArenaType() == ARENA_TYPE_5v5)
                    {
                        TypeSolo++;
                        if (TypeSolo > (page + 1) * GamesOnPage)
                        {
                            haveNextPage[2] = true;
                            break;
                        }

                        if (TypeSolo >= page * GamesOnPage)
                            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, GetGamesStringData(arena, mmr, mmrTwo), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SELECTED_PLAYER + GetFirstPlayerGuid(arena));
                    }
                }
            }

            if (page > 0)
            {
                if (haveNextPage[0] == true)
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "<<", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_2V2_GAMES + page - 1);
                if (haveNextPage[1] == true)
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "<<", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_3V3_GAMES + page - 1);
                if (haveNextPage[2] == true)
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "<<", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_5V5_GAMES + page - 1);
            }
            else
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GTS(LANG_SPECTATE_MAIN_MENU), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_MAIN_MENU);

            if (haveNextPage[0] == true)
                AddGossipItemFor(player, GOSSIP_ICON_DOT, ">>", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_2V2_GAMES + page + 1);
            if (haveNextPage[1] == true)
                AddGossipItemFor(player, GOSSIP_ICON_DOT, ">>", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_3V3_GAMES + page + 1);
            if (haveNextPage[2] == true)
                AddGossipItemFor(player, GOSSIP_ICON_DOT, ">>", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_5V5_GAMES + page + 1);
        }

        /*bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
        {
            if (!player)
                return true;

            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();
            if (sender == GOSSIP_SENDER_MAIN)
            {
                switch (action)
                {
                case NPC_SPECTATOR_ACTION_SPECIFIC: // choosing a player
                    if (Player* target = ObjectAccessor::FindPlayerByName(code))
                    {
                        ChatHandler handler(player->GetSession());
                        char const* pTarget = target->GetName().c_str();
                        arena_spectator_commands::HandleSpectateCommand(&handler, pTarget);
                    }
                    else
                        ChatHandler(player->GetSession()).SendSysMessage("ERROR: Player not found.");
                    return true;
                }
            }
            return false;
        }*/
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_arena_spectatorAI(creature);
    }
};

void AddSC_arena_spectator_script()
{
    new arena_spectator_commands();
    new npc_arena_spectator();
}
