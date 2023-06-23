/*
 * Copyright (C) 2021+ ATieshCore <https://at-wow.org/>
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

#include "WintergraspMgr.h"
#include "AchievementMgr.h"
#include "CreatureTextMgr.h"
#include "SharedDefines.h"
#include "SpellAuras.h"
#include "ObjectAccessor.h"
#include "WGGOBuilding.h"
#include "WGWorkshop.h"
#include "WGGraveyard.h"

 // Player sections
void WintergraspMgr::AddPlayer(Player* plr, bool InZone, bool IsWaitingQueue, bool IsWaitingWar, bool IsWaitingKick, time_t time)
{
    if (PlayerInBFPlayerMap(plr))
        return;

    PlayerHolder pl;
    pl.inZone = InZone;
    pl.isWaitingQueue = IsWaitingQueue;
    pl.isWaitingWar = IsWaitingWar;
    pl.isWaitingKick = IsWaitingKick;
    pl.inQueue = false;
    pl.inWar = false;
    pl.team = plr->GetCFSTeamId();
    pl.time = time;
    m_PlayerMap[plr->GetGUID()] = pl;
}

void WintergraspMgr::UpdateZoneStatusInPlayerMap(Player* plr, bool inZone)
{
    PlayerHolderContainer::iterator itr = m_PlayerMap.find(plr->GetGUID());
    if (itr != m_PlayerMap.end())
        itr->second.inZone = inZone;
}

bool WintergraspMgr::PlayerInBFPlayerMap(Player* plr)
{
    PlayerHolderContainer::const_iterator itr = m_PlayerMap.find(plr->GetGUID());
    if (itr != m_PlayerMap.end())
        return true;
    return false;
}

void WintergraspMgr::RemovePlayer(Player* plr)
{
    bool founded = false;
    PlayerHolderContainer::const_iterator itr = m_PlayerMap.find(plr->GetGUID());
    if (itr != m_PlayerMap.end())
    {
        if (itr->second.inQueue)
            m_freeslots[itr->second.team]++;

        if (itr->second.inWar)
            plr->SetBattlefieldWarMember(false);

        founded = true;
    }

    if (founded)
        m_PlayerMap.erase(plr->GetGUID());
}

void WintergraspMgr::InviteNewPlayerToQueue(Player* player, bool isInZone)
{
    if (!player)
        return;

    // we should to add each player in queue
    AddPlayer(player, isInZone, true, false, false);
    player->GetSession()->SendBattlefieldInvitePlayerToQueue(BATTLEFIELD_WG);
}

void WintergraspMgr::InviteNewPlayerToWar(Player* player, bool inZone)
{
    if (!player)
        return;

    // we should to add each player in queue, doesn't matter war status
    AddPlayer(player, inZone, true, true, false, GameTime::GetGameTime() + m_TimeForAcceptInvite);
    player->GetSession()->SendBattlefieldInvitePlayerToWar(BATTLEFIELD_WG, AREA_WINTERGRASP, m_TimeForAcceptInvite);
}

void WintergraspMgr::InvitePlayersInZoneToQueue()
{
    time_t futuretime = GameTime::GetGameTime() + m_TimeForAcceptInvite;
    for (PlayerHolderContainer::iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        if (!itr->second.inQueue && !itr->second.isWaitingQueue)
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            {
                itr->second.isWaitingQueue = true;
                itr->second.time = futuretime;
                player->GetSession()->SendBattlefieldInvitePlayerToQueue(BATTLEFIELD_WG);
            }
}

void WintergraspMgr::InvitePlayersInQueueToWar()
{
    time_t futuretime = GameTime::GetGameTime() + m_TimeForAcceptInvite;
    for (PlayerHolderContainer::iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        if (itr->second.inQueue)
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            {
                itr->second.isWaitingWar = true;
                itr->second.time = futuretime;
                player->GetSession()->SendBattlefieldInvitePlayerToWar(BATTLEFIELD_WG, AREA_WINTERGRASP, m_TimeForAcceptInvite);
            }
}

void WintergraspMgr::TryInvitePlayersNotInQueueToWarOrKickThem()
{
    std::set<ObjectGuid>_playerkick;
    time_t futuretime = GameTime::GetGameTime() + m_TimeForAcceptInvite;
    for (PlayerHolderContainer::iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        if (!itr->second.inQueue)
        {
            if (GetFreeslot(itr->second.team))
            {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (player->GetLevel() >= m_MinLevel)
                    {
                        itr->second.isWaitingQueue = true;
                        itr->second.isWaitingWar = true;
                        itr->second.time = futuretime;
                        player->GetSession()->SendBattlefieldInvitePlayerToWar(BATTLEFIELD_WG, AREA_WINTERGRASP, m_TimeForAcceptInvite);
                    }
                    else
                        _playerkick.insert(itr->first);
                }
            }
            else
                _playerkick.insert(itr->first);
        }

    for (auto itr = _playerkick.begin(); itr != _playerkick.end(); ++itr)
        KickPlayerFromBattlefield(*itr);
    _playerkick.clear();
}

void WintergraspMgr::KickPlayerFromBattlefield(ObjectGuid guid)
{
    if (Player* player = ObjectAccessor::FindPlayer(guid))
        if (player->GetZoneId() == AREA_WINTERGRASP)
        {
            player->TeleportTo(KickPosition);
            RemovePlayer(player);
            player->GetSession()->SendBattlefieldLeaveMessage(BATTLEFIELD_WG);
        }
}

// Called in WorldSession::HandleBfQueueInviteResponse
void WintergraspMgr::PlayerAcceptInviteToQueue(Player* player)
{
    PlayerHolderContainer::iterator itr = m_PlayerMap.find(player->GetGUID());
    if (itr != m_PlayerMap.end())
    {
        if (!itr->second.inQueue && itr->second.isWaitingQueue)
        {
            if (GetFreeslot(player->GetCFSTeamId()))
            {
                m_freeslots[player->GetCFSTeamId()]--;
                itr->second.isWaitingQueue = false;
                itr->second.inQueue = true;
                // Send notification
                player->GetSession()->SendBattlefieldQueueInviteResponse(BATTLEFIELD_WG, AREA_WINTERGRASP);
            }
            else
            {
                itr->second.isWaitingQueue = false;
                itr->second.inQueue = false;
                // Send notification
                player->GetSession()->SendBattlefieldQueueInviteResponse(BATTLEFIELD_WG, AREA_WINTERGRASP, false, false);
            }
        }
    }
}

// Called in WorldSession::HandleBfEntryInviteResponse
void WintergraspMgr::PlayerAcceptInviteToWar(Player* player)
{
    if (!IsWarTime())
        return;

    PlayerHolderContainer::iterator itr = m_PlayerMap.find(player->GetGUID());
    if (itr != m_PlayerMap.end())
    {
        // if newplayer trying to enter in war we should add him in queue
        if (itr->second.isWaitingQueue)
        {
            if (GetFreeslot(player->GetCFSTeamId()))
            {
                m_freeslots[player->GetCFSTeamId()]--;
                itr->second.isWaitingQueue = false;
                itr->second.isWaitingWar = false;
                itr->second.inQueue = true;
                player->SetBattlefieldWarMember(true);
            }
            else // if player has declined invite in queue in zone, and bf has started, but no more free slots
            {
                itr->second.isWaitingQueue = false;
                itr->second.isWaitingWar = false;
                // just kick him by next update tick
                itr->second.isWaitingKick = true;
                itr->second.time = GameTime::GetGameTime();
                player->GetSession()->SendBattlefieldEjectPending(BATTLEFIELD_WG, true);
                player->SetBattlefieldWarMember(false);
            }
        }

        // Check that this player already in queue
        if (itr->second.inQueue)
        {
            itr->second.isWaitingWar = false;
            itr->second.inWar = true;
            player->SetBattlefieldWarMember(true);
            if (AddOrSetPlayerToCorrectBfGroup(player))
            {
                player->GetSession()->SendBattlefieldEntered(BATTLEFIELD_WG);
                if (player->isAFK())
                    player->ToggleAFK();

                OnPlayerJoinWar(player);                               //for scripting
            }
        }
    }
}

void WintergraspMgr::PlayerDeclineInviteToQueue(Player* player)
{
    PlayerHolderContainer::iterator itr = m_PlayerMap.find(player->GetGUID());
    if (itr != m_PlayerMap.end())
    {
        itr->second.isWaitingQueue = false;
        itr->second.inQueue = false;
    }
}

void WintergraspMgr::PlayerDeclineInviteToWar(Player* player)
{
    if (PlayerInBFPlayerMap(player))
    {
        PlayerHolderContainer::const_iterator itr = m_PlayerMap.find(player->GetGUID());
        if (itr != m_PlayerMap.end())
        {
            if (itr->second.inQueue || itr->second.inWar)
                m_freeslots[player->GetCFSTeamId()]++;
        }        
    }
    AskToLeaveQueue(player);
}

void WintergraspMgr::SendRemoveWorldStates(Player* /*player*/) // TODO it
{
}

void WintergraspMgr::SendInitWorldStatesTo(Player* player)
{
    WorldPackets::WorldState::InitWorldStates packet;
    packet.MapID = BATTLEFIELD_WG_MAPID;
    packet.ZoneID = AREA_WINTERGRASP;
    packet.AreaID = player->GetAreaId();
    FillInitialWorldStates(packet);

    player->SendDirectMessage(packet.Write());
}

void WintergraspMgr::SendInitWorldStatesToAll()
{
    for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayerByLowGUID(itr->first))
            SendInitWorldStatesTo(player);
}

void WintergraspMgr::SendAreaSpiritHealerQueryOpcode(Player* player, ObjectGuid guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    uint32 time = m_LastResurrectTimer;  // resurrect every 30 seconds

    data << guid << time;
    player->SendDirectMessage(&data);
}

void WintergraspMgr::HandleResurrectedPlayer(ObjectGuid guid)
{
    for (auto& graveID : m_graveyardMap)
    {
        if (WGGraveyard* grave = graveID.second)
        {
            if (grave->HasPlayer(guid))
            {
                grave->RemovePlayer(guid);
                break;
            }
        }
    }
}

void WintergraspMgr::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_DEFENDED_A, GetData(BATTLEFIELD_WG_DATA_DEF_A));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_DEFENDED_H, GetData(BATTLEFIELD_WG_DATA_DEF_H));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_ATTACKED_A, GetData(BATTLEFIELD_WG_DATA_WON_A));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_ATTACKED_H, GetData(BATTLEFIELD_WG_DATA_WON_H));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_ATTACKER, GetAttackerTeam());
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_DEFENDER, GetDefenderTeam());

    // Note: cleanup these two, their names look awkward
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_ACTIVE, IsWarTime() ? 0 : 1);
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_SHOW_WORLDSTATE, IsWarTime() ? 1 : 0);

    for (uint32 itr = 0; itr < 2; ++itr)
        packet.Worldstates.emplace_back(ClockWorldState[itr], int32(GameTime::GetGameTime()) + int32(m_Timer) / int32(1000));

    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_VEHICLE_H));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_MAX_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_VEHICLE_A));
    packet.Worldstates.emplace_back(WS_BATTLEFIELD_WG_MAX_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A));

    for (auto pointer : m_buildingsInZone)
        pointer.second->FillInitialWorldStates(packet);

    for (auto pointer : m_workshopAndCaptures)
        pointer.second._workshopPoint->FillInitialWorldStates(packet);
}

void WintergraspMgr::SendWarning(uint8 id, WorldObject const* target /*= nullptr*/)
{
    if (Creature* stalker = GetCreature(m_StalkerGuid))
        sCreatureTextMgr->SendChat(stalker, id, target);
}

void WintergraspMgr::SendUpdateWorldState(uint32 field, uint32 value)
{
    for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            player->SendUpdateWorldState(field, value);
}

void WintergraspMgr::RemoveAurasFromPlayer(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_RECRUIT);
    player->RemoveAurasDueToSpell(SPELL_CORPORAL);
    player->RemoveAurasDueToSpell(SPELL_LIEUTENANT);
    player->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);
    player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY);
    player->RemoveAurasDueToSpell(SPELL_TENACITY);
    player->RemoveAurasDueToSpell(SPELL_ESSENCE_OF_WINTERGRASP);
    player->RemoveAurasDueToSpell(SPELL_WINTERGRASP_RESTRICTED_FLIGHT_AREA);
}

void WintergraspMgr::UpdateTenacity()
{
    uint32 alliancePlayers = 0;
    uint32 hordePlayers = 0;
    for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
    {
        if (itr->second.inWar)
        {
            if (itr->second.team == TEAM_ALLIANCE)
                alliancePlayers++;
            else if (itr->second.team == TEAM_HORDE)
                hordePlayers++;
        }
    }
    int32 newStack = 0;

    if (alliancePlayers && hordePlayers)
    {
        if (alliancePlayers < hordePlayers)
            newStack = int32((float(hordePlayers / alliancePlayers) - 1) * 4);  // positive, should cast on alliance
        else if (alliancePlayers > hordePlayers)
            newStack = int32((1 - float(alliancePlayers / hordePlayers)) * 4);  // negative, should cast on horde
    }

    if (newStack == int32(m_tenacityStack))
        return;

    m_tenacityStack = newStack;

    // Apply new buff
    if (newStack)
    {
        m_tenacityTeam = newStack > 0 ? TEAM_ALLIANCE : TEAM_HORDE;

        if (newStack < 0)
            newStack = -newStack;
        if (newStack > 20)
            newStack = 20;

        uint32 buff_honor = SPELL_GREATEST_HONOR;
        if (newStack < 15)
            buff_honor = SPELL_GREATER_HONOR;
        if (newStack < 10)
            buff_honor = SPELL_GREAT_HONOR;
        if (newStack < 5)
            buff_honor = 0;

        for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        {
            if (itr->second.team == m_tenacityTeam)
            {
                // Remove old buff
                if (m_tenacityTeam != TEAM_NEUTRAL)
                {
                    if (Player* player = ObjectAccessor::FindPlayerByLowGUID(itr->first))
                        if (player->GetLevel() >= m_MinLevel)
                            player->RemoveAurasDueToSpell(SPELL_TENACITY);

                }

                if (itr->second.inWar)
                    if (Player* player = ObjectAccessor::FindPlayerByLowGUID(itr->first))
                    {
                        player->SetAuraStack(SPELL_TENACITY, player, newStack);

                        if (buff_honor != 0)
                            player->CastSpell(player, buff_honor, true);
                    }
            }
        }

        // Remove old buff
        if (m_tenacityTeam != TEAM_NEUTRAL)
        {
            for (auto itr = m_vehicles[m_tenacityTeam].begin(); itr != m_vehicles[m_tenacityTeam].end(); ++itr)
                if (Creature* creature = GetCreature(*itr))
                    creature->RemoveAurasDueToSpell(SPELL_TENACITY_VEHICLE);
        }

        for (auto itr = m_vehicles[m_tenacityTeam].begin(); itr != m_vehicles[m_tenacityTeam].end(); ++itr)
            if (Creature* creature = GetCreature(*itr))
                creature->SetAuraStack(SPELL_TENACITY_VEHICLE, creature, newStack);

        if (buff_honor != 0)
        {
            for (auto itr = m_vehicles[m_tenacityTeam].begin(); itr != m_vehicles[m_tenacityTeam].end(); ++itr)
                if (Creature* creature = GetCreature(*itr))
                    creature->CastSpell(creature, buff_honor, true);
        }
    }
    else
        m_tenacityTeam = TEAM_NEUTRAL;
}

void WintergraspMgr::BrokenWallOrTower(TeamId team, WGGameObjectBuilding* building)
{
    if (team == GetDefenderTeam())
    {
        for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
            if (itr->second.inWar && itr->second.team == GetAttackerTeam())
                if (Player* player = ObjectAccessor::FindPlayerByLowGUID(itr->first))
                    if (player->GetDistance2d(ASSERT_NOTNULL(GetGameObject(building->GetGUID()))) < 50.0f)
                        player->KilledMonsterCredit(QUEST_CREDIT_DEFEND_SIEGE);
    }
}

void WintergraspMgr::DoCompleteOrIncrementAchievement(uint32 achievement, Player* player, uint8 /*incrementNumber*/)
{
    if (!player)
        return;

    AchievementDBC const* AchievementDBC = sAchievementMgr->GetAchievement(achievement);
    if (!AchievementDBC)
        return;

    switch (achievement)
    {
        case ACHIEVEMENTS_WIN_WG_100:
        {
            // player->UpdateAchievementCriteria();
            break;
        }
        default:
        {
            player->CompletedAchievement(AchievementDBC);
            break;
        }
    }
}

// Called in WorldSession::HandleBfQueueExitRequest
void WintergraspMgr::AskToLeaveQueue(Player* player)
{
    if (IsWarTime())
        if (player->GetZoneId() == AREA_WINTERGRASP)
            PlayerAskToLeave(player);        

    RemovePlayer(player);
    player->GetSession()->SendBattlefieldLeaveMessage(BATTLEFIELD_WG);
}

// Called in WorldSession::HandleHearthAndResurrect
void WintergraspMgr::PlayerAskToLeave(Player* player)
{
    // Player leaving Wintergrasp, teleport to Dalaran.
    // ToDo: confirm teleport destination.
    player->TeleportTo(571, 5804.1499f, 624.7710f, 647.7670f, 1.6400f);
}

// Helpers
void WintergraspMgr::OnPlayerJoinWar(Player* player)
{
    RemoveAurasFromPlayer(player);

    player->CastSpell(player, SPELL_RECRUIT, true);

    if (player->GetZoneId() != AREA_WINTERGRASP)
    {
        if (player->GetTeamId() == GetDefenderTeam())
            player->TeleportTo(571, 5345, 2842, 410, 3.14f);
        else
        {
            if (player->GetTeamId() == TEAM_HORDE)
                player->TeleportTo(571, 5025.857422f, 3674.628906f, 362.737122f, 4.135169f);
            else
                player->TeleportTo(571, 5101.284f, 2186.564f, 373.549f, 3.812f);
        }
    }

    UpdateTenacity();

    if (player->GetTeamId() == GetAttackerTeam())
    {
        if (GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT) < 3)
            player->SetAuraStack(SPELL_TOWER_CONTROL, player, 3 - GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT));
    }
    else
    {
        if (GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT) > 0)
            player->SetAuraStack(SPELL_TOWER_CONTROL, player, GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT));
    }
    SendInitWorldStatesTo(player);
}

void WintergraspMgr::OnPlayerLeaveWar(Player* player)
{
    // Remove all aura from WG /// @todo false we can go out of this zone on retail and keep Rank buff, remove on end of WG
    if (!player->GetSession()->PlayerLogout())
    {
        if (Creature* vehicle = player->GetVehicleCreatureBase())   // Remove vehicle of player if he go out.
            vehicle->DespawnOrUnsummon();

        RemoveAurasFromPlayer(player);
    }

    /*player->RemoveAurasDueToSpell(SPELL_HORDE_CONTROLS_FACTORY_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_ALLIANCE_CONTROLS_FACTORY_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_HORDE_CONTROL_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_ALLIANCE_CONTROL_PHASE_SHIFT);*/
    UpdateTenacity();
}

void WintergraspMgr::HandlePromotion(Player* playerKiller, Unit* unitKilled)
{
    uint32 teamId = playerKiller->GetTeamId();

    for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
    {
        if (itr->second.inWar)
        {
            if (itr->second.team == teamId)
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                    if (player->GetDistance2d(unitKilled) < 40.0f)
                        PromotePlayer(player);
        }
    }
}

// Update rank for player
void WintergraspMgr::PromotePlayer(Player* killer)
{
    if (!m_isActive)
        return;
    // Updating rank of player
    if (Aura* auraRecruit = killer->GetAura(SPELL_RECRUIT))
    {
        if (auraRecruit->GetStackAmount() >= 5)
        {
            killer->RemoveAura(SPELL_RECRUIT);
            killer->CastSpell(killer, SPELL_CORPORAL, true);
            if (Creature* stalker = GetCreature(m_StalkerGuid))
                sCreatureTextMgr->SendChat(stalker, BATTLEFIELD_WG_TEXT_RANK_CORPORAL, killer, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, killer);
        }
        else
            killer->CastSpell(killer, SPELL_RECRUIT, true);
    }
    else if (Aura* auraCorporal = killer->GetAura(SPELL_CORPORAL))
    {
        if (auraCorporal->GetStackAmount() >= 5)
        {
            killer->RemoveAura(SPELL_CORPORAL);
            killer->CastSpell(killer, SPELL_LIEUTENANT, true);
            if (Creature* stalker = GetCreature(m_StalkerGuid))
                sCreatureTextMgr->SendChat(stalker, BATTLEFIELD_WG_TEXT_RANK_FIRST_LIEUTENANT, killer, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, killer);
        }
        else
            killer->CastSpell(killer, SPELL_CORPORAL, true);
    }
}

void WintergraspMgr::_OnBattleStartPlayers()
{
    for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
    {
        if (itr->second.inZone)
        {
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            {
                // Dismount players in zone when start WG
                if (player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                player->AddAura(12438, player);

                SendInitWorldStatesTo(player);

                // Teleport outzoned guys?
                float x, y, z;
                player->GetPosition(x, y, z);
                if (5500 > x && x > 5392 && y < 2880 && y > 2800 && z < 480)
                    player->TeleportTo(571, 5349.8686f, 2838.481f, 409.240f, 0.046328f);
            }
        }
    }
}
