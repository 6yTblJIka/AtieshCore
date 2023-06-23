/*
 * Copyright (C) 2019+ ATieshCore <https://at-wow.org/>
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
#include "DalaranCrater.h"
#include "Group.h"
#include "Guild.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "Language.h"
#include "Log.h"
#include "WorldSession.h"
#include "World.h"

DalaranGEvent::DalaranGEvent()
{
    _eventId = SPECIALEVENT_EVENTID_DALARANCRATER;
    activeFight = false;
    registration = true;
    possibleDistance = 0;
    m_DurationTimer = 0;
    m_TeleporterTimer = 0;
    alivePlayerCount = 0;
    _winnername = "";
    SetPhase(BATTLE_ENDED);

    prepareTimer.Reset(IN_MILLISECONDS);
    combatTimer.Reset(IN_MILLISECONDS);
    playersTimer.Reset(IN_MILLISECONDS);
}

bool DalaranGEvent::SetupSpecialEvent(bool enabled, bool active, bool repeatable, uint32 id, uint32 cooldownTimer, uint32 durationTimer, std::string comment)
{
    // override Setup for Set time of first run in cooldownTimer
    if (SpecialEvent::SetupSpecialEvent(enabled, active, repeatable, id, cooldownTimer, durationTimer, comment))
        return true;

    return false;
}

DalaranGEvent::~DalaranGEvent()
{
    m_playersDataStore.clear();
}

void DalaranGEvent::Update(uint32 diff)
{
    SpecialEvent::Update(diff);

    if (IsActiveDalaranEvent())
    {
        // Timers for Players (distance / mapid ) and their Damage if out of range
        bool combatphases = GetPhase() > PREPARE_PHASE_0 && GetPhase() != BATTLE_ENDED;
        if (combatphases)
        {
            playersTimer.Update(diff);
            if (playersTimer.Passed())
            {
                std::vector<Player*> playerList;
                // First check on Map valid
                for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
                {
                    if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
                    {
                        if (player->GetMapId() != 0)
                        {
                            playerList.push_back(player);
                            continue;
                        }

                        // Second check on distance valid (damage players if not near)
                        if (!player->IsWithinDist2d(DalaranCraterPoint.GetPositionX(), DalaranCraterPoint.GetPositionY(), possibleDistance))
                            player->DealDamage(player, player, CalculatePct(player->GetMaxHealth(), 1.0f));
                    }
                }

                if (!playerList.empty())
                {
                    for (const auto& plrUD : playerList)
                        RemovePlayerFromQueue(plrUD);
                }
                playerList.clear();

                if (alivePlayerCount == 1)
                {
                    ReceiveWinnerName();
                    sWorld->SendWorldText(LANG_DALARAN_CRATER_SUCCESS_STOP_ANNOUNCE, _winnername.c_str());
                    StopFightEvent();
                }
            }
        }

        // Timers for state Combat ( 1-2-3 phases for others handlers)
        combatTimer.Update(diff);
        if (combatTimer.Passed())
        {
            switch (GetPhase())
            {
                case BATTLE_PHASE_1:
                {
                    possibleDistance = 300.0f;
                    combatTimer.Reset(2 * IN_MILLISECONDS * MINUTE);
                    SetPhase(BATTLE_PHASE_2);
                    break;
                }
                case BATTLE_PHASE_2:
                {
                    possibleDistance = 150.0f;
                    combatTimer.Reset(1 * IN_MILLISECONDS * MINUTE);
                    SetPhase(BATTLE_PHASE_3);
                    break;
                }
                case BATTLE_PHASE_3:
                {
                    possibleDistance = 75.0f;
                    combatTimer.Reset(1 * IN_MILLISECONDS * MINUTE);
                    break;
                }
                default:
                    break;
            }
        }

        if (m_DurationTimer > 0)
        {
            if (diff >= m_DurationTimer)
            {
                m_DurationTimer = 0;
                sWorld->SendWorldText(LANG_DALARAN_CRATER_PREPARE_TO_STOP_ANNOUNCE, GetCountPlayerInEvent());
            }
            else
                m_DurationTimer -= diff;
        }
    }
    else
    {
        prepareTimer.Update(diff);
        if (prepareTimer.Passed())
        {
            switch (GetPhase())
            {
                case PREPARE_PHASE_60:
                {
                    sWorld->SendWorldText(LANG_DALARAN_CRATER_ANNOUNCE_BEFORE_START_60);
                    SetPhase(PREPARE_PHASE_30);
                    prepareTimer.Reset(30 * IN_MILLISECONDS);
                    break;
                }
                case PREPARE_PHASE_30:
                {
                    sWorld->SendWorldText(LANG_DALARAN_CRATER_ANNOUNCE_BEFORE_START_30);
                    SetPhase(PREPARE_PHASE_0);
                    prepareTimer.Reset(30 * IN_MILLISECONDS);
                    break;
                }
                case PREPARE_PHASE_0:
                {
                    if (GetCountPlayerInEvent() < sWorld->customGetIntConfig(CONFIG_DALARAN_GAME_EVENTS_MIN_PLAYERS))
                    {
                        sWorld->SendWorldText(LANG_DALARAN_CRATER_NOT_ENOGH_MIN_PLAYERS);
                        m_playersDataStore.clear();
                        registration = true;

                        // Update timers
                        prepareTimer.Reset(_noEventTime);
                        combatTimer.Reset(_noEventTime);
                        SetPhase(BATTLE_ENDED);

                        // For main class SpecialEvent
                        _timer.Reset(_noEventTime);
                        _active = false;
                        _gameTimeNextEvent = uint32(GameTime::GetGameTime() + _noEventTime / IN_MILLISECONDS);
                    }
                    else
                    {
                        TeleportAllPlayersInZone();
                        BroadcastToMemberPrepare();
                        // players droped in ground and rdy to start fight (30 sec announce and set FFAPVP flag)
                        SetPhase(BATTLE_PHASE_0);
                        prepareTimer.Reset(30 * IN_MILLISECONDS);
                    }
                    break;
                }
                case BATTLE_PHASE_0:
                {
                    StartFightEvent();
                    break;
                }
                default:
                    break;
            }
        }

        if (m_TeleporterTimer > 0)
        {
            if (diff >= m_TeleporterTimer)
            {
                m_TeleporterTimer = 0;
                TeleportAllPlayersBack();
            }
            else
                m_TeleporterTimer -= diff;
        }
    }
}

void DalaranGEvent::OnSpecialEventStart()
{
    SetPhase(PREPARE_PHASE_60);
}

void DalaranGEvent::OnSpecialEventEnd(bool /*endByTimer*/)
{
    sWorld->SendWorldText(LANG_DALARAN_CRATER_DURATION_STOP_ANNOUNCE);
    StopFightEvent();
}

void DalaranGEvent::AddPlayer(ObjectGuid playerGUID)
{
    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGUID))
        InvitePlayerToQueue(player);
}

void DalaranGEvent::RemovePlayer(ObjectGuid playerGUID)
{
    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGUID))
    {
        if (IsActiveDalaranEvent())
        {
            if (sWorld->customGetBoolConfig(CONFIG_DALARAN_GAME_EVENTS_INSTANT_RETURN))
                RemovePlayerFromFight(player);
            else
                RemovePlayerFromQueue(player);
        }
        else
            RemovePlayerFromQueue(player);
    }
}

bool DalaranGEvent::IsMemberOfEvent(Player* player)
{
    PlayersDataContainer::const_iterator itr = m_playersDataStore.find(player->GetGUID());
    if (itr != m_playersDataStore.end())
        return true;

    return false;
}

void DalaranGEvent::StartFightEvent()
{
    // Update timers
    combatTimer.Reset(IN_MILLISECONDS);
    prepareTimer.Reset(_EventTime);

    sWorld->SendWorldText(LANG_DALARAN_CRATER_START_ANNOUNCE);
    SetPhase(BATTLE_PHASE_1);
    activeFight = true;
    if (GetDurationTimer() > (IN_MILLISECONDS * MINUTE))
        m_DurationTimer = GetDurationTimer() - (IN_MILLISECONDS * MINUTE);
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
            player->SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
    }
}

void DalaranGEvent::StopFightEvent()
{
    // Update timers
    prepareTimer.Reset(IN_MILLISECONDS);
    combatTimer.Reset(_noEventTime);

    SetPhase(BATTLE_ENDED);
    activeFight = false;
    m_DurationTimer = 0;
    m_TeleporterTimer = 45 * IN_MILLISECONDS;
    alivePlayerCount = 0;
    sWorld->SendWorldText(LANG_DALARAN_CRATER_PREPARE_TO_RETURN_PLAYERS);

    if (!sWorld->IsFFAPvPRealm())
    {
        for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
        {
            if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
                player->RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
        }
    }
}

void DalaranGEvent::SpawnGOLight()
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player * player = ObjectAccessor::FindConnectedPlayer(itr->first))
        {
            QuaternionData rotation = QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.f, 0.f);
            player->SummonGameObject(182483, DalaranCraterPoint, rotation, Minutes(GetDurationTimer()));
        }
    }
}

void DalaranGEvent::TeleportAllPlayersInZone()
{
    registration = false;

    std::vector<Player*> playerList;
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
        {
            if (!player->IsAlive())
            {
                playerList.push_back(player);
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_DALARAN_CRATER_LEAVE_PLAYER_NOT_ALIVE);
                continue;
            }

            if (player->GetGroup())
            {
                if (player->GetGroup()->isRaidGroup())
                    player->GetGroup()->RemoveMember(itr->first);
                else if (!sWorld->customGetBoolConfig(CONFIG_DALARAN_GAME_EVENTS_SQUAD_ENABLED))
                    player->GetGroup()->RemoveMember(itr->first);
            }

            player->TeleportTo(DalaranCrater);
            player->CastSpell(player, 59200, true);

            ++alivePlayerCount;
        }
    }

    if (!playerList.empty())
        for (const auto& plrUD : playerList)
            RemovePlayerFromQueue(plrUD);        
    playerList.clear();

    SpawnGOLight();
}

void DalaranGEvent::TeleportAllPlayersBack()
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
        {
            uint32 mapid = itr->second.mapid;
            float x = itr->second.x;
            float y = itr->second.y;
            float z = itr->second.z;
            player->ResurrectPlayer(player->GetSession()->HasPermission(rbac::RBAC_PERM_RESURRECT_WITH_FULL_HPS) ? 1.0f : 0.5f);
            player->TeleportTo(mapid, x, y, z, player->GetOrientation());
        }
    }

    m_playersDataStore.clear();
    registration = true;
}

void DalaranGEvent::BroadcastToMemberAboutLeavePlayer(std::string const& Name)
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_DALARAN_CRATER_LEAVE_PLAYER_AND_COUNT, Name, alivePlayerCount);
    }
}

void DalaranGEvent::BroadcastToMemberPrepare()
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player * player = ObjectAccessor::FindConnectedPlayer(itr->first))
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_DALARAN_CRATER_START_ANNOUNCE_30_SEC, GetCountPlayerInEvent());
    }
}

void DalaranGEvent::RemovePlayerFromFight(Player* player, bool withteleport)
{
    if (!IsMemberOfEvent(player))
        return;

    if (withteleport)
    {
        PlayersDataContainer::const_iterator itr = m_playersDataStore.find(player->GetGUID());
        if (itr != m_playersDataStore.end())
        {
            uint32 mapid = itr->second.mapid;
            float x = itr->second.x;
            float y = itr->second.y;
            float z = itr->second.z;
            player->TeleportTo(mapid, x, y, z, player->GetOrientation());
            if (!sWorld->IsFFAPvPRealm())
                player->RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
        }
    }    

    m_playersDataStore.erase(player->GetGUID());
    BroadcastToMemberAboutLeavePlayer(player->GetName());
    --alivePlayerCount;
}

void DalaranGEvent::ReceiveWinnerName()
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
        {
            if (player->IsAlive())
            {
                _winnername = player->GetName();
                player->RewardPVPCap();
                if (player->GetGuild())
                {
                    player->GetGuild()->AddGuildExp(15, player, true);
                    player->GetGuild()->UpdateGuildRating(2, true, player);
                }
            }
        }
    }
}

void DalaranGEvent::InvitePlayerToQueue(Player* player)
{
    PlayersData pd;
    pd.mapid = player->GetMapId();
    pd.x = player->GetPositionX();
    pd.y = player->GetPositionY();
    pd.z = player->GetPositionZ();

    // add in active player list of event
    m_playersDataStore[player->GetGUID()] = pd;
}

void DalaranGEvent::RemovePlayerFromQueue(Player* player)
{
    if (!IsMemberOfEvent(player))
        return;

    m_playersDataStore.erase(player->GetGUID());
}

class SpecialEvent_DalaranCrater : public SpecialEventScript
{
public:
    SpecialEvent_DalaranCrater() : SpecialEventScript("dalaran_crater") { }

    SpecialEvent* GetSpecialEvent() const override
    {
        return new DalaranGEvent();
    }
};

void AddSC_DalaranCraterScripts()
{
    new SpecialEvent_DalaranCrater();
}
