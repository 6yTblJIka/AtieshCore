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
#include "Battleground.h"
#include "DBCStoresMgr.h"
#include "GameObject.h"
#include "MapManager.h"
#include "Log.h"
#include "WGCapturePoint.h"
#include "WGGraveyard.h"
#include "WGGOBuilding.h"
#include "WGWorkshop.h"
#include "World.h"

WintergraspGameObjectData const WGPortalDefenderData[WG_MAX_TELEPORTER] =
{
    // Player teleporter
    { { 5153.408f, 2901.349f, 409.1913f, -0.06981169f }, { 0.f, 0.f, -0.03489876f, 0.9993908f }, 190763, 191575 },
    { { 5268.698f, 2666.421f, 409.0985f, -0.71558490f }, { 0.f, 0.f, -0.35020730f, 0.9366722f }, 190763, 191575 },
    { { 5197.050f, 2944.814f, 409.1913f,  2.33874000f }, { 0.f, 0.f,  0.92050460f, 0.3907318f }, 190763, 191575 },
    { { 5196.671f, 2737.345f, 409.1892f, -2.93213900f }, { 0.f, 0.f, -0.99452110f, 0.1045355f }, 190763, 191575 },
    { { 5314.580f, 3055.852f, 408.8620f,  0.54105060f }, { 0.f, 0.f,  0.26723770f, 0.9636307f }, 190763, 191575 },
    { { 5391.277f, 2828.094f, 418.6752f, -2.16420600f }, { 0.f, 0.f, -0.88294700f, 0.4694727f }, 190763, 191575 },
    { { 5153.931f, 2781.671f, 409.2455f,  1.65806200f }, { 0.f, 0.f,  0.73727700f, 0.6755905f }, 190763, 191575 },
    { { 5311.445f, 2618.931f, 409.0916f, -2.37364400f }, { 0.f, 0.f, -0.92718320f, 0.3746083f }, 190763, 191575 },
    { { 5269.208f, 3013.838f, 408.8276f, -1.76278200f }, { 0.f, 0.f, -0.77162460f, 0.6360782f }, 190763, 191575 },

    { { 5401.634f, 2853.667f, 418.6748f,  2.63544400f }, { 0.f, 0.f,  0.96814730f, 0.2503814f }, 192819, 192819 }, // return portal inside fortress, neutral
    // Vehicle teleporter
    { { 5314.515f, 2703.687f, 408.5502f, -0.89011660f }, { 0.f, 0.f, -0.43051050f, 0.9025856f }, 192951, 192951 },
    { { 5316.252f, 2977.042f, 408.5385f, -0.82030330f }, { 0.f, 0.f, -0.39874840f, 0.9170604f }, 192951, 192951 }
};

WintergraspMgr::WintergraspMgr()
{
    m_buildingsInZone.clear();
    m_workshopAndCaptures.clear();
    m_graveyardMap.clear();
    m_PlayerMap.clear();
    m_CreatureMap.clear();

    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        DefenderPortalList[i].clear();
    m_KeepHordeGameObjectList.clear();
    m_KeepAllianceGameObjectList.clear();
    m_KeepHordeNPCList.clear();
    m_KeepAllianceNPCList.clear();
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_vehicles[i].clear();
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_Groups[i].clear();

    m_Map = nullptr;

    m_isEnabled           = false;
    m_isActive            = false;
    m_isRelicInteractible = false;
    m_StartGrouping       = false;

    m_Timer               = 0;
    m_MinPlayer           = 0;
    m_MaxPlayer           = 0;
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_freeslots[i] = 0;
    m_TimeForAcceptInvite = 0;
    m_tenacityStack       = 0;
    m_MinLevel            = 0;
    m_BattleTime          = 0;
    m_NoWarBattleTime     = 0;
    m_StartGroupingTimer  = 0;
    m_saveTimer           = 0;
    m_uiAcceptTimer       = 0;
    m_LastResurrectTimer  = 0;

    KickPosition.Relocate(0,0,0,0);
    KickPosition.m_mapId = 0;

    m_DefenderTeam = TEAM_ALLIANCE;
    m_tenacityTeam = TEAM_ALLIANCE;

    m_StalkerGuid.Clear();
    m_TitansRelicGUID.Clear();
    m_KeepCollisionWall.Clear();

    // Init Graveyards
    Grave_NW       = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1330));
    Grave_NE       = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1329));
    Grave_SW       = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1334));
    Grave_SE       = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1333));
    Grave_Keep     = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1285));
    Grave_Horde    = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1331));
    Grave_Alliance = ASSERT_NOTNULL(sDBCStoresMgr->GetWorldSafeLocsDBC(1332));

    m_Data32.clear();
}

WintergraspMgr::~WintergraspMgr()
{
    for (auto pointer : m_buildingsInZone)
        delete pointer.second;
    m_buildingsInZone.clear();

    for (auto pointer : m_workshopAndCaptures)
    {
        delete pointer.second._capturePoint;
        delete pointer.second._workshopPoint;
    }        
    m_workshopAndCaptures.clear();

    for (auto pointer : m_graveyardMap)
        delete pointer.second;
    m_graveyardMap.clear();
    m_PlayerMap.clear();
    m_CreatureMap.clear();

    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        DefenderPortalList[i].clear();
    m_KeepHordeGameObjectList.clear();
    m_KeepAllianceGameObjectList.clear();
    m_KeepHordeNPCList.clear();
    m_KeepAllianceNPCList.clear();
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_vehicles[i].clear();
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_Groups[i].clear();

    m_Map = nullptr;

    m_isEnabled           = false;
    m_isActive            = false;
    m_isRelicInteractible = false;
    m_StartGrouping       = false;

    m_Timer               = 0;
    m_MinPlayer           = 0;
    m_MaxPlayer           = 0;
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_freeslots[i] = 0;
    m_TimeForAcceptInvite = 0;
    m_tenacityStack       = 0;
    m_MinLevel            = 0;
    m_BattleTime          = 0;
    m_NoWarBattleTime     = 0;
    m_StartGroupingTimer  = 0;
    m_saveTimer           = 0;
    m_uiAcceptTimer       = 0;
    m_LastResurrectTimer  = 0;

    KickPosition.Relocate(0, 0, 0, 0);
    KickPosition.m_mapId = 0;

    m_DefenderTeam = TEAM_ALLIANCE;
    m_tenacityTeam = TEAM_ALLIANCE;

    if (Creature* StalkerCreature = GetCreature(m_StalkerGuid))
        StalkerCreature->DespawnOrUnsummon();
    m_StalkerGuid.Clear();
    m_TitansRelicGUID.Clear();
    m_KeepCollisionWall.Clear();

    Grave_NW       = nullptr;
    Grave_NE       = nullptr;
    Grave_SW       = nullptr;
    Grave_SE       = nullptr;
    Grave_Keep     = nullptr;
    Grave_Horde    = nullptr;
    Grave_Alliance = nullptr;

    m_Data32.clear();
}

WintergraspMgr* WintergraspMgr::instance()
{
    static WintergraspMgr instance;
    return &instance;
}

void WintergraspMgr::SetupWG(TeamId defender, bool StartWar)
{
    m_DefenderTeam = defender;
    // Setup (for command bf switch)
    if (StartWar)
    {
        // Spawn titan relic
        if (GameObject* relic = SpawnGameObject(GO_WINTERGRASP_TITAN_S_RELIC, WintergraspRelicPos, WintergraspRelicRot))
        {
            // Update faction of relic, only attacker can click on
            relic->SetFaction(WintergraspFaction[GetAttackerTeam()]);
            // Set in use (not allow to click on before last door is broken)
            relic->SetFlag(GO_FLAG_IN_USE | GO_FLAG_NOT_SELECTABLE);
            m_TitansRelicGUID = relic->GetGUID();
        }
        else
            FMT_LOG_ERROR("bg.battlefield", "WG: Failed to spawn titan relic.");
    }
    // Rebuild all wall
    for (auto pointer : m_buildingsInZone)
        pointer.second->Rebuild();

    UpdateAllGuardsAndTurretsBeforeBattle();
    UpdateAllGOforKeep();

    SetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT, 0);
    SetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF, 0);
    SetData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT, 0);
    SetData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF, 0);

    // Update graveyard (in no war time all graveyard is to deffender, in war time, depend of base)
    for (auto pointer : m_workshopAndCaptures)
        pointer.second._workshopPoint->UpdateGraveyardAndWorkshop();

    uint32 gyID = _GetGraveyardIDByType(WGGraveyardId(BATTLEFIELD_WG_GY_KEEP));
    if (WGGraveyard* gy = GetGraveyardById(gyID))
        gy->GiveControlTo(GetDefenderTeam());

    // Initialize vehicle counter
    UpdateCounterVehicle(true);
}

void WintergraspMgr::InitializeWG()
{
    uint32 oldMSTime = getMSTime();
    m_Map = sMapMgr->FindMap(BATTLEFIELD_WG_MAPID, 0);
    if (!m_Map)
        m_Map = sMapMgr->CreateBaseMap(BATTLEFIELD_WG_MAPID);
    //m_Map->LoadAllCells();

    m_isEnabled = sWorld->getBoolConfig(CONFIG_WINTERGRASP_ENABLE);
    m_isActive  = sWorld->getWorldState(WS_BATTLEFIELD_WG_ACTIVE) != 0;
    /*m_isRelicInteractible*/
    m_StartGrouping = false;

    m_Timer               = sWorld->getWorldState(ClockWorldState[m_isEnabled && m_isActive ? 1 : 0]);
    m_MinPlayer           = sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MIN);
    m_MaxPlayer           = sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MAX);
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_freeslots[i] = m_MaxPlayer;
    m_TimeForAcceptInvite = 20;
    m_tenacityStack       = 0;
    m_MinLevel            = sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MIN_LVL);
    m_BattleTime          = sWorld->getIntConfig(CONFIG_WINTERGRASP_BATTLETIME) * MINUTE * IN_MILLISECONDS;
    m_NoWarBattleTime     = sWorld->getIntConfig(CONFIG_WINTERGRASP_NOBATTLETIME) * MINUTE * IN_MILLISECONDS;
    m_StartGroupingTimer  = 15 * MINUTE * IN_MILLISECONDS;
    m_saveTimer           = 60000;
    m_uiAcceptTimer       = 1000;
    m_LastResurrectTimer  = 30 * IN_MILLISECONDS;

    m_DefenderTeam = TeamId(sWorld->getWorldState(WS_BATTLEFIELD_WG_DEFENDER));
    m_tenacityTeam = TEAM_NEUTRAL;

    KickPosition.Relocate(5728.117f, 2714.346f, 697.733f, 0);
    KickPosition.m_mapId = BATTLEFIELD_WG_MAPID;

    InitStalker(BATTLEFIELD_WG_NPC_STALKER, WintergraspStalkerPos);

    m_Data32.resize(BATTLEFIELD_WG_DATA_MAX);
    // setup worldstate
    sWorld->setWorldState(WS_BATTLEFIELD_WG_ACTIVE, uint64(m_isActive));
    sWorld->setWorldState(WS_BATTLEFIELD_WG_DEFENDER, sWorld->getWorldState(WS_BATTLEFIELD_WG_DEFENDER));
    sWorld->setWorldState(ClockWorldState[0], uint64(m_NoWarBattleTime));

    SetData(BATTLEFIELD_WG_DATA_WON_A, uint32(sWorld->getWorldState(WS_BATTLEFIELD_WG_ATTACKED_A)));
    SetData(BATTLEFIELD_WG_DATA_DEF_A, uint32(sWorld->getWorldState(WS_BATTLEFIELD_WG_DEFENDED_A)));
    SetData(BATTLEFIELD_WG_DATA_WON_H, uint32(sWorld->getWorldState(WS_BATTLEFIELD_WG_ATTACKED_H)));
    SetData(BATTLEFIELD_WG_DATA_DEF_H, uint32(sWorld->getWorldState(WS_BATTLEFIELD_WG_DEFENDED_H)));

    //part 1 - create pointer at graveyard
    for (uint8 i = 0; i < BATTLEFIELD_WG_GRAVEYARD_MAX; i++)
    {
        WGGraveyard* graveyard = new WGGraveyard(this);
        graveyard->Initialize(WGGraveyardId(i));
        m_graveyardMap[graveyard->GetGraveyardId()] = graveyard;
    }

    //part 2 - create workshop with capturepoint
    for (uint8 i = 0; i < BATTLEFIELD_WG_WORKSHOP_MAX; i++)
    {
        WorkshopAndCapturePointPAIR wc;
        WGWorkshop* workshop         = nullptr;
        WGCapturePoint* capturePoint = nullptr;

        workshop = ASSERT_NOTNULL(new WGWorkshop(this, WintergraspWorkshopIds(i)));

        // capture point should be only in first 4 workshops : SE/SW/NE/NW
        if (i < BATTLEFIELD_WG_WORKSHOP_KEEP_WEST)
        {
            capturePoint = ASSERT_NOTNULL(new WGCapturePoint(this, WintergraspWorkshopIds(i), TEAM_NEUTRAL));

            if (i < BATTLEFIELD_WG_WORKSHOP_NE)
            {
                workshop->GiveControlTo(GetAttackerTeam(), true);
                capturePoint->SetInitialData(GetAttackerTeam());
            }
            else
            {
                workshop->GiveControlTo(GetDefenderTeam(), true);
                capturePoint->SetInitialData(GetDefenderTeam());
            }
        }
        else
            workshop->GiveControlTo(GetDefenderTeam(), true);

        wc._capturePoint  = capturePoint;
        wc._workshopPoint = workshop;
        m_workshopAndCaptures[WintergraspWorkshopIds(i)] = wc;
    }

    //part 3 - create towers, walls and etc
    for (uint8 i = 0; i < WG_MAX_OBJ; i++)
    {
        if (WGGameObjectBuilding* b = new WGGameObjectBuilding(this, WGGameObjectBuildings[i].type, WGGameObjectBuildings[i].WorldState))
            m_buildingsInZone[WGGameObjectBuildings[i].entry] = b;
    }

    // Spawning portal defender
    for (uint8 i = 0; i < WG_MAX_TELEPORTER; ++i)
    {
        WintergraspGameObjectData const& teleporter = WGPortalDefenderData[i];
        if (GameObject* go = ASSERT_NOTNULL(SpawnGameObject(teleporter.AllianceEntry, teleporter.Pos, teleporter.Rot)))
        {
            DefenderPortalList[TEAM_ALLIANCE].push_back(go->GetGUID());
            go->SetRespawnTime(GetDefenderTeam() == TEAM_ALLIANCE ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
        }

        if (GameObject* go = ASSERT_NOTNULL(SpawnGameObject(teleporter.HordeEntry, teleporter.Pos, teleporter.Rot)))
        {
            DefenderPortalList[TEAM_HORDE].push_back(go->GetGUID());
            go->SetRespawnTime(GetDefenderTeam() == TEAM_HORDE ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
        }
    }

    InitAllGOforKeep();
    UpdateCounterVehicle(true);
    FMT_LOG_INFO("server.loading", "Wintergrasp PVP system loaded in {} ms", GetMSTimeDiffToNow(oldMSTime));
}

void WintergraspMgr::Update(uint32 diff)
{
    if (m_Timer <= diff)
    {
        // Battlefield ends on time
        if (IsWarTime())
            EndBattle(true);
        else // Time to start a new battle!
            StartBattle();
    }
    else
        m_Timer -= diff;

    // Invite players a few minutes before the battle's beginning
    if (!IsWarTime() && !m_StartGrouping && m_Timer <= m_StartGroupingTimer)
    {
        m_StartGrouping = true;
        InvitePlayersInZoneToQueue();
        OnStartGrouping();
    }

    // Expired time for Awaiting Responce List
    if (m_uiAcceptTimer <= diff)
    {
        time_t now = GameTime::GetGameTime();
        std::set<ObjectGuid>_playerkick;
        for (PlayerHolderContainer::iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        {
            // check estimation time for awaiting war and waiting to kick
            if (itr->second.isWaitingWar || itr->second.isWaitingKick)
            {
                if (itr->second.time <= now)
                    _playerkick.insert(itr->first);
            }
        }

        for (auto itr = _playerkick.begin(); itr != _playerkick.end(); ++itr)
            KickPlayerFromBattlefield(*itr);
        _playerkick.clear();

        m_uiAcceptTimer = 1000;
    }
    else
        m_uiAcceptTimer -= diff;

    for (auto pointer : m_workshopAndCaptures)
        if (pointer.second._capturePoint)
            pointer.second._capturePoint->Update(diff);

    if (m_LastResurrectTimer <= diff)
    {
        for (auto& graveID : m_graveyardMap)
        {
            if (WGGraveyard* grave = graveID.second)
                grave->Resurrect();
        }
        m_LastResurrectTimer = RESURRECTION_INTERVAL;
    }
    else
        m_LastResurrectTimer -= diff;

    if (m_saveTimer <= diff)
    {
        sWorld->setWorldState(WS_BATTLEFIELD_WG_ACTIVE, uint64(m_isActive));
        sWorld->setWorldState(WS_BATTLEFIELD_WG_DEFENDER, uint64(m_DefenderTeam));
        sWorld->setWorldState(ClockWorldState[0], uint64(m_Timer));
        sWorld->setWorldState(WS_BATTLEFIELD_WG_ATTACKED_A, uint64(GetData(BATTLEFIELD_WG_DATA_WON_A)));
        sWorld->setWorldState(WS_BATTLEFIELD_WG_DEFENDED_A, uint64(GetData(BATTLEFIELD_WG_DATA_DEF_A)));
        sWorld->setWorldState(WS_BATTLEFIELD_WG_ATTACKED_H, uint64(GetData(BATTLEFIELD_WG_DATA_WON_H)));
        sWorld->setWorldState(WS_BATTLEFIELD_WG_DEFENDED_H, uint64(GetData(BATTLEFIELD_WG_DATA_DEF_H)));
        m_saveTimer = 60 * IN_MILLISECONDS;
    }
    else
        m_saveTimer -= diff;
}

void WintergraspMgr::StartBattle()
{
    if (m_isActive)
        return;

    for (uint8 team = 0; team < PVP_TEAMS_COUNT; ++team)
        m_Groups[team].clear();

    m_Timer = m_BattleTime;
    m_isActive = true;

    InvitePlayersInQueueToWar();
    TryInvitePlayersNotInQueueToWarOrKickThem();
    OnBattleStart();
}

void WintergraspMgr::EndBattle(bool endByTimer)
{
    if (!m_isActive)
        return;

    m_isActive      = false;
    m_StartGrouping = false;

    if (!endByTimer)
        SetDefenderTeam(GetAttackerTeam());

    OnBattleEnd(endByTimer);

    for (PlayerHolderContainer::iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
    {
        itr->second.inWar = false;
        itr->second.inQueue = false;

        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            player->SetBattlefieldWarMember(false);
    }

    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_freeslots[i] = m_MaxPlayer;

    // Reset bat0tlefield timer
    m_Timer = m_NoWarBattleTime;
    SendInitWorldStatesToAll();
}

void WintergraspMgr::OnBattleStart()
{
    SetupWG(m_DefenderTeam, true);
    // Send start warning to all players
    SendWarning(BATTLEFIELD_WG_TEXT_START_BATTLE);
    _OnBattleStartPlayers();
}

void WintergraspMgr::OnBattleEnd(bool endByTimer)
{
    // Remove relic
    if (m_TitansRelicGUID)
        if (GameObject* relic = GetGameObject(m_TitansRelicGUID))
            relic->RemoveFromWorld();
    m_TitansRelicGUID.Clear();

    // change collision wall state closed
    if (GameObject* kGO = GetGameObject(m_KeepCollisionWall))
        kGO->SetGoState(GO_STATE_READY); //not GO_STATE_ACTIVE

    // successful defense
    if (endByTimer)
        UpdateData(GetDefenderTeam() == TEAM_HORDE ? BATTLEFIELD_WG_DATA_DEF_H : BATTLEFIELD_WG_DATA_DEF_A, 1);
    // successful attack (note that teams have already been swapped, so defender team is the one who won)
    else
        UpdateData(GetDefenderTeam() == TEAM_HORDE ? BATTLEFIELD_WG_DATA_WON_H : BATTLEFIELD_WG_DATA_WON_A, 1);

    // Update all graveyard, control is to defender when no wartime
    for (uint8 i = 0; i < BATTLEFIELD_WG_GY_HORDE; i++)
        if (WGGraveyard* graveyard = GetGraveyardById(i))
            graveyard->GiveControlTo(GetDefenderTeam());

    // Update portals
    for (auto itr = DefenderPortalList[GetDefenderTeam()].begin(); itr != DefenderPortalList[GetDefenderTeam()].end(); ++itr)
        if (GameObject* portal = GetGameObject(*itr))
            portal->SetRespawnTime(RESPAWN_IMMEDIATELY);

    for (auto itr = DefenderPortalList[GetAttackerTeam()].begin(); itr != DefenderPortalList[GetAttackerTeam()].end(); ++itr)
        if (GameObject* portal = GetGameObject(*itr))
            portal->SetRespawnTime(RESPAWN_ONE_DAY);

    for (PlayerHolderContainer::iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
    {
        if (itr->second.inWar)
        {
            if (itr->second.team == GetDefenderTeam())
            {
                // section for defenders
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    player->CastSpell(player, SPELL_ESSENCE_OF_WINTERGRASP, true);
                    player->CastSpell(player, SPELL_VICTORY_REWARD, true);
                    // Complete victory quests
                    player->AreaExploredOrEventHappens(QUEST_VICTORY_WINTERGRASP_A);
                    player->AreaExploredOrEventHappens(QUEST_VICTORY_WINTERGRASP_H);
                    // Send Wintergrasp victory achievement
                    DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG, player);
                    // Award achievement for succeeding in Wintergrasp in 10 minutes or less
                    if (!endByTimer && m_Timer <= 10000)
                        DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG_TIMER_10, player);

                    RemoveAurasFromPlayer(player);
                }
            }
            else
            {
                // section for attackers
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    player->CastSpell(player, SPELL_DEFEAT_REWARD, true);
                    RemoveAurasFromPlayer(player);
                }
            }

            itr->second.inWar = false;
        }

        /*if (!endByTimer)
            if (Player* player = ObjectAccessor::FindPlayer(itr->second.GUID))
            {
                player->RemoveAurasDueToSpell(m_DefenderTeam == TEAM_ALLIANCE ? SPELL_HORDE_CONTROL_PHASE_SHIFT : SPELL_ALLIANCE_CONTROL_PHASE_SHIFT, player->GetGUID());
                player->AddAura(m_DefenderTeam == TEAM_HORDE ? SPELL_HORDE_CONTROL_PHASE_SHIFT : SPELL_ALLIANCE_CONTROL_PHASE_SHIFT, player);
            }*/
    }

    for (uint8 team = 0; team < PVP_TEAMS_COUNT; ++team)
    {
        for (auto itr = m_vehicles[team].begin(); itr != m_vehicles[team].end(); ++itr)
            if (Creature* creature = GetCreature(*itr))
                if (creature->IsVehicle())
                    creature->DespawnOrUnsummon();

        m_vehicles[team].clear();
    }

    if (!endByTimer) // win alli/horde
        SendWarning(GetDefenderTeam() == TEAM_ALLIANCE ? BATTLEFIELD_WG_TEXT_FORTRESS_CAPTURE_ALLIANCE : BATTLEFIELD_WG_TEXT_FORTRESS_CAPTURE_HORDE);
    else // defend alli/horde
        SendWarning(GetDefenderTeam() == TEAM_ALLIANCE ? BATTLEFIELD_WG_TEXT_FORTRESS_DEFEND_ALLIANCE : BATTLEFIELD_WG_TEXT_FORTRESS_DEFEND_HORDE);
}

uint32 WintergraspMgr::_GetGraveyardIDByType(WGGraveyardId type)
{
    uint32 result = 0;
    switch (type)
    {
        case BATTLEFIELD_WG_GY_WORKSHOP_SE: result = 1333; break;
        case BATTLEFIELD_WG_GY_WORKSHOP_SW: result = 1334; break;
        case BATTLEFIELD_WG_GY_WORKSHOP_NE: result = 1329; break;
        case BATTLEFIELD_WG_GY_WORKSHOP_NW: result = 1330; break;
        case BATTLEFIELD_WG_GY_KEEP: result = 1285; break;
        case BATTLEFIELD_WG_GY_HORDE: result = 1331; break;
        case BATTLEFIELD_WG_GY_ALLIANCE: result = 1332; break;
        default:
            break;
    }
    return result;
}

WGGraveyard* WintergraspMgr::GetGraveyardById(uint32 id) const
{
    GraveyardMap::const_iterator itr = m_graveyardMap.find(id);
    if (itr != m_graveyardMap.end())
        return itr->second;
    return nullptr;
}

WorldSafeLocsDBC const* WintergraspMgr::GetClosestGraveyard(Player* player)
{
    WGGraveyard* closestGY = nullptr;
    for (auto& graveID : m_graveyardMap)
    {
        if (WGGraveyard* grave = graveID.second)
        {
            if (grave->GetControlTeamId() != player->GetTeamId())
                continue;

            if (!closestGY)
                closestGY = grave;
            else
            {
                float dist = grave->GetDistance(player);
                float distOld = closestGY->GetDistance(player);
                if (dist < distOld)
                    closestGY = grave;
            }
        }
    }

    if (closestGY)
        return sDBCStoresMgr->GetWorldSafeLocsDBC(closestGY->GetGraveyardId());

    return nullptr;
}

WGGraveyardId WintergraspMgr::GetSpiritGraveyardIdForCreature(Creature* creature) const
{
    if (creature->IsInDist2d(Grave_NW->Loc.X, Grave_NW->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_WORKSHOP_NW;

    if (creature->IsInDist2d(Grave_NE->Loc.X, Grave_NE->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_WORKSHOP_NE;

    if (creature->IsInDist2d(Grave_SW->Loc.X, Grave_SW->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_WORKSHOP_SW;

    if (creature->IsInDist2d(Grave_SE->Loc.X, Grave_SE->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_WORKSHOP_SE;

    if (creature->IsInDist2d(Grave_Keep->Loc.X, Grave_Keep->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_KEEP;

    if (creature->IsInDist2d(Grave_Horde->Loc.X, Grave_Horde->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_HORDE;

    if (creature->IsInDist2d(Grave_Alliance->Loc.X, Grave_Alliance->Loc.Y, 20.f))
        return BATTLEFIELD_WG_GY_ALLIANCE;

    FMT_LOG_ERROR("bg.battlefield", "BattlefieldWG::GetSpiritGraveyardIdForCreature: Unexpected SpiritCreature Id {}", creature->GetSpawnId());
    return BATTLEFIELD_WG_GY_WORKSHOP_SE;
}

// Called when a tower is damaged, used for honor reward calcul
void WintergraspMgr::UpdateDamagedTowerCount(TeamId team)
{
    if (team == GetAttackerTeam())
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT, 1);
    else
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF, 1);
}

// Called when a tower is broke
void WintergraspMgr::UpdateDestroyedTowerCount(TeamId team)
{
    // Southern tower
    if (team == GetAttackerTeam())
    {
        // Update counter
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT, -1);
        UpdateData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT, 1);

        for (PlayerHolderContainer::const_iterator itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
        {
            if (itr->second.inWar)
            {
                if (itr->second.team == GetAttackerTeam())
                {
                    // Remove buff stack on attackers
                    if (Player* player = ObjectAccessor::FindPlayerByLowGUID(itr->first))
                        player->RemoveAuraFromStack(SPELL_TOWER_CONTROL);
                }
                else
                {
                    // Add buff stack to defenders and give achievement/quest credit
                    if (Player* player = ObjectAccessor::FindPlayerByLowGUID(itr->first))
                    {
                        player->CastSpell(player, SPELL_TOWER_CONTROL, true);
                        player->KilledMonsterCredit(QUEST_CREDIT_TOWERS_DESTROYED);
                        DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WG_TOWER_DESTROY, player);
                    }
                }
            }
        }

        // If all three south towers are destroyed (ie. all attack towers), remove ten minutes from battle time
        if (GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT) == 3)
        {
            if (int32(m_Timer - 600000) < 0)
                m_Timer = 0;
            else
                m_Timer -= 600000;
            SendInitWorldStatesToAll();
        }
    }
    else // Keep tower
    {
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF, -1);
        UpdateData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF, 1);
    }
}

void WintergraspMgr::UpdateCounterVehicle(bool init)
{
    SetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H, 0);
    SetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A, 0);

    if (init)
    {
        SetData(BATTLEFIELD_WG_DATA_VEHICLE_H, 0);
        SetData(BATTLEFIELD_WG_DATA_VEHICLE_A, 0);
    }    

    for (auto pointer : m_workshopAndCaptures)
    {
        if (pointer.second._workshopPoint->GetTeamControl() == TEAM_ALLIANCE)
            UpdateData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A, 4);
        else if (pointer.second._workshopPoint->GetTeamControl() == TEAM_HORDE)
            UpdateData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H, 4);
    }

    UpdateVehicleCountWG();
}

// Update vehicle count WorldState to player
void WintergraspMgr::UpdateVehicleCountWG()
{
    SendUpdateWorldState(WS_BATTLEFIELD_WG_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_VEHICLE_H));
    SendUpdateWorldState(WS_BATTLEFIELD_WG_MAX_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H));
    SendUpdateWorldState(WS_BATTLEFIELD_WG_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_VEHICLE_A));
    SendUpdateWorldState(WS_BATTLEFIELD_WG_MAX_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A));
}
