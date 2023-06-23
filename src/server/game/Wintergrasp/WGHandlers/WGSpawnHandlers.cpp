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
#include "Creature.h"
#include "GameObject.h"
#include "MapManager.h"
#include "WGGOBuilding.h"
#include "WGWorkshop.h"

void WintergraspMgr::InitStalker(uint32 entry, Position const& pos)
{
    if (Creature* creature = SpawnCreature(entry, pos))
        m_StalkerGuid = creature->GetGUID();
    else
        FMT_LOG_ERROR("bg.battlefield", "Battlefield::InitStalker: Could not spawn Stalker (Creature entry {}), zone messages will be unavailable!", entry);
}

Creature* WintergraspMgr::SpawnCreature(uint32 entry, Position const& pos)
{
    //Get map object
    Map* map = sMapMgr->CreateBaseMap(BATTLEFIELD_WG_MAPID);
    if (!map)
    {
        FMT_LOG_ERROR("bg.battlefield", "Battlefield::SpawnCreature: Can't create creature entry: {}, map not found.", entry);
        return nullptr;
    }

    Creature* creature = new Creature();
    if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, entry, pos))
    {
        FMT_LOG_ERROR("bg.battlefield", "Battlefield::SpawnCreature: Can't create creature entry: {}", entry);
        delete creature;
        return nullptr;
    }

    creature->SetHomePosition(pos);

    // Set creature in world
    map->AddToMap(creature);
    creature->setActive(true);
    creature->SetFarVisible(true);

    return creature;
}

// Method for spawning gameobject on map
GameObject* WintergraspMgr::SpawnGameObject(uint32 entry, Position const& pos, QuaternionData const& rot)
{
    // Get map object
    Map* map = sMapMgr->CreateBaseMap(BATTLEFIELD_WG_MAPID);
    if (!map)
        return nullptr;

    // Create gameobject
    GameObject* go = new GameObject;
    if (!go->Create(map->GenerateLowGuid<HighGuid::GameObject>(), entry, map, PHASEMASK_NORMAL, pos, rot, 255, GO_STATE_READY))
    {
        FMT_LOG_ERROR("bg.battlefield", "Battlefield::SpawnGameObject: Gameobject template {} could not be found in the database! Battlefield has not been created!", entry);
        FMT_LOG_ERROR("bg.battlefield", "Battlefield::SpawnGameObject: Could not create gameobject template {}! Battlefield has not been created!", entry);
        delete go;
        return nullptr;
    }

    // Add to world
    map->AddToMap(go);
    go->setActive(true);
    go->SetFarVisible(true);

    return go;
}

Creature* WintergraspMgr::GetCreature(ObjectGuid guid)
{
    if (!m_Map)
        return nullptr;
    return m_Map->GetCreature(guid);
}

GameObject* WintergraspMgr::GetGameObject(ObjectGuid guid)
{
    if (!m_Map)
        return nullptr;
    return m_Map->GetGameObject(guid);
}

GameObject* WintergraspMgr::GetTowerGObyTowerID(WintergraspTowerIds towerID)
{
    if (!m_Map)
        return nullptr;
    for (WGGameObjectBuildingMap::const_iterator itr = m_buildingsInZone.begin(); itr != m_buildingsInZone.end(); ++itr)
    {
        if (itr->second->IsTower() && itr->second->GetTowerId() == towerID)
            if (itr->second->GetGUID())
                return m_Map->GetGameObject(itr->second->GetGUID());
    }
    return nullptr;
}

WGGameObjectBuilding* WintergraspMgr::GetBuildingByGOEntry(uint32 entry)
{
    WGGameObjectBuildingMap::const_iterator itr = m_buildingsInZone.find(entry);
    if (itr != m_buildingsInZone.end())
            return itr->second;
    return nullptr;
}

bool WintergraspMgr::IsCreatureInHolder(ObjectGuid guid)
{
    CreatureHolderContainer::const_iterator itr = m_CreatureMap.find(guid);
    if (itr != m_CreatureMap.end())
    {
        if (Creature* creature = GetCreature(guid))
        {
            if (itr->second.m_isActive)
                ShowNpc(creature, true);
            else
                HideNpc(creature);
            return true;
        }
    }
    return false;
}

bool WintergraspMgr::AddCreatureInHolderByGUID(Creature* creature, uint8 npcType, TeamId team)
{
    if (IsCreatureInHolder(creature->GetGUID()))
        return false;

    UpdateStatusForCreature(creature, npcType, team);

    CreatureHolder ch;
    ch.m_npcType = npcType;
    ch.m_team = team;
    ch.m_isActive = creature->IsVisible();
    m_CreatureMap[creature->GetGUID()] = ch;
    return true;
}

void WintergraspMgr::UpdateStatusForCreature(Creature* creature, uint8 npcType, TeamId team)
{
    switch (npcType)
    {
        // Attacker sector
        case WEST_TOWER_GUARD:
        case WEST_TOWER_TURRET:
        case WEST_BRIDGE_GUARD:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_SHADOWSIGHT_TOWER, creature, team);
            break;
        }    
        case MIDDLE_TOWER_GUARD:
        case MIDDLE_TOWER_TURRET:
        case MIDDLE_BRIDGE_GUARD:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_WINTER_S_EDGE_TOWER, creature, team);
            break;
        }
        case EAST_TOWER_GUARD:
        case EAST_TOWER_TURRET:
        case EAST_BRIDGE_GUARD:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_FLAMEWATCH_TOWER, creature, team);
            break;
        }
        // Defender sector
        case NE_TOWER_GUARD:
        case NE_TOWER_TURRET:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_FORTRESS_TOWER_NE, creature, team);
            break;
        }
        case NW_TOWER_GUARD:
        case NW_TOWER_TURRET:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_FORTRESS_TOWER_NW, creature, team);
            break;
        }
        case SE_TOWER_GUARD:
        case SE_TOWER_TURRET:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_FORTRESS_TOWER_SE, creature, team);
            break;
        }
        case SW_TOWER_GUARD:
        case SW_TOWER_TURRET:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_FORTRESS_TOWER_SW, creature, team);
            break;
        }
        // keep inside
        case VAULT_KEEP:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_VAULT_GATE, creature, team);
            break;
        }
        // keep gate (outside)
        case FORTRESS_GATE_TURRET:
        case VAULT_GATE_TURRET:
        {
            _UpdateCreatureForBuildGO(GO_WINTERGRASP_FORTRESS_GATE, creature, team);
            break;
        }
        //workshop sector
        case WG_WORKSHOP_SE:
        {
            _UpdateCreatureForWorkshop(BATTLEFIELD_WG_WORKSHOP_SE, creature, team);
            break;
        }
        case WG_WORKSHOP_SW:
        {
            _UpdateCreatureForWorkshop(BATTLEFIELD_WG_WORKSHOP_SW, creature, team);
            break;
        }
        case WG_WORKSHOP_NE:
        {
            _UpdateCreatureForWorkshop(BATTLEFIELD_WG_WORKSHOP_NE, creature, team);
            break;
        }
        case WG_WORKSHOP_NW:
        {
            _UpdateCreatureForWorkshop(BATTLEFIELD_WG_WORKSHOP_NW, creature, team);
            break;
        }
        case WG_WORKSHOP_KEEP_WEST:
        {
            _UpdateCreatureForWorkshop(BATTLEFIELD_WG_WORKSHOP_KEEP_WEST, creature, team);
            break;
        }
        case WG_WORKSHOP_KEEP_EAST:
        {
            _UpdateCreatureForWorkshop(BATTLEFIELD_WG_WORKSHOP_KEEP_EAST, creature, team);
            break;
        }
        case WG_ALLIANCE_GRAVEYARD:
        {
            if (creature->GetEntry() == NPC_DWARVEN_SPIRIT_GUIDE)
                ShowNpc(creature, true);
            else
                HideNpc(creature);
            break;
        }
        case WG_HORDE_GRAVEYARD:
        {
            if (creature->GetEntry() == NPC_TAUNKA_SPIRIT_GUIDE)
                ShowNpc(creature, true);
            else
                HideNpc(creature);
            break;
        }
    }
}

void WintergraspMgr::ShowCreatureByNPCType(uint8 npcType, TeamId team)
{
    for (CreatureHolderContainer::iterator itr = m_CreatureMap.begin(); itr != m_CreatureMap.end(); ++itr)
        if (itr->second.m_npcType == npcType &&
            itr->second.m_team == team)
        {
            if (Creature* creature = GetCreature(itr->first))
            {
                ShowNpc(creature, true);
                itr->second.m_isActive = true;
            }
        }
}

void WintergraspMgr::HideCreatureByNPCType(uint8 npcType, TeamId team)
{
    for (CreatureHolderContainer::iterator itr = m_CreatureMap.begin(); itr != m_CreatureMap.end(); ++itr)
        if (itr->second.m_npcType == npcType &&
            itr->second.m_team == team)
        {
            if (Creature* creature = GetCreature(itr->first))
            {
                HideNpc(creature);
                itr->second.m_isActive = false;
            }
        }
}

void WintergraspMgr::UpdateCreatureTurretByNPCType(uint8 npcType, TeamId team)
{
    for (CreatureHolderContainer::iterator itr = m_CreatureMap.begin(); itr != m_CreatureMap.end(); ++itr)
        if (itr->second.m_npcType == npcType)
        {
            if (Creature* creature = GetCreature(itr->first))
            {
                ShowNpc(creature, true);
                creature->SetFaction(WintergraspFaction[team]);
                itr->second.m_isActive = true;
            }
        }
}

void WintergraspMgr::HideCreatureTurretByNPCType(uint8 npcType)
{
    for (CreatureHolderContainer::iterator itr = m_CreatureMap.begin(); itr != m_CreatureMap.end(); ++itr)
        if (itr->second.m_npcType == npcType)
        {
            if (Creature* creature = GetCreature(itr->first))
            {
                HideNpc(creature);
                itr->second.m_isActive = false;
            }
        }
}

void WintergraspMgr::UpdateAllGuardsAndTurretsBeforeBattle()
{
    //              Attack Towers
    // guards
    ShowCreatureByNPCType(EAST_TOWER_GUARD, GetAttackerTeam());
    HideCreatureByNPCType(EAST_TOWER_GUARD, GetDefenderTeam());

    ShowCreatureByNPCType(MIDDLE_TOWER_GUARD, GetAttackerTeam());
    HideCreatureByNPCType(MIDDLE_TOWER_GUARD, GetDefenderTeam());

    ShowCreatureByNPCType(WEST_TOWER_GUARD, GetAttackerTeam());
    HideCreatureByNPCType(WEST_TOWER_GUARD, GetDefenderTeam());

    // turrets
    UpdateCreatureTurretByNPCType(WEST_TOWER_TURRET, GetAttackerTeam());
    UpdateCreatureTurretByNPCType(MIDDLE_TOWER_TURRET, GetAttackerTeam());
    UpdateCreatureTurretByNPCType(EAST_TOWER_TURRET, GetAttackerTeam());

    //              bridges
    // guards
    ShowCreatureByNPCType(WEST_BRIDGE_GUARD, GetAttackerTeam());
    HideCreatureByNPCType(WEST_BRIDGE_GUARD, GetDefenderTeam());

    ShowCreatureByNPCType(MIDDLE_BRIDGE_GUARD, GetAttackerTeam());
    HideCreatureByNPCType(MIDDLE_BRIDGE_GUARD, GetDefenderTeam());

    ShowCreatureByNPCType(EAST_BRIDGE_GUARD, GetAttackerTeam());
    HideCreatureByNPCType(EAST_BRIDGE_GUARD, GetDefenderTeam());

    //              workshops
    // guards
    ShowCreatureByNPCType(WG_WORKSHOP_SE, GetAttackerTeam()); // workshop on south should be owned by attacker team by default
    HideCreatureByNPCType(WG_WORKSHOP_SE, GetDefenderTeam());

    ShowCreatureByNPCType(WG_WORKSHOP_SW, GetAttackerTeam()); // workshop on south should be owned by attacker team by default
    HideCreatureByNPCType(WG_WORKSHOP_SW, GetDefenderTeam());

    ShowCreatureByNPCType(WG_WORKSHOP_NE, GetDefenderTeam()); // workshop on north should be owned by defender team by default
    HideCreatureByNPCType(WG_WORKSHOP_NE, GetAttackerTeam());

    ShowCreatureByNPCType(WG_WORKSHOP_NW, GetDefenderTeam()); // workshop on north should be owned by defender team by default
    HideCreatureByNPCType(WG_WORKSHOP_NW, GetAttackerTeam());

    // KEEP
    ShowCreatureByNPCType(WG_WORKSHOP_KEEP_WEST, GetDefenderTeam());
    HideCreatureByNPCType(WG_WORKSHOP_KEEP_WEST, GetAttackerTeam());

    ShowCreatureByNPCType(WG_WORKSHOP_KEEP_EAST, GetDefenderTeam());
    HideCreatureByNPCType(WG_WORKSHOP_KEEP_EAST, GetAttackerTeam());

    ShowCreatureByNPCType(NE_TOWER_GUARD, GetDefenderTeam());
    HideCreatureByNPCType(NE_TOWER_GUARD, GetAttackerTeam());

    ShowCreatureByNPCType(NW_TOWER_GUARD, GetDefenderTeam());
    HideCreatureByNPCType(NW_TOWER_GUARD, GetAttackerTeam());

    ShowCreatureByNPCType(SE_TOWER_GUARD, GetDefenderTeam());
    HideCreatureByNPCType(SE_TOWER_GUARD, GetAttackerTeam());

    ShowCreatureByNPCType(SW_TOWER_GUARD, GetDefenderTeam());
    HideCreatureByNPCType(SW_TOWER_GUARD, GetAttackerTeam());

    ShowCreatureByNPCType(VAULT_KEEP, GetDefenderTeam());
    HideCreatureByNPCType(VAULT_KEEP, GetAttackerTeam());
    // turrets
    UpdateCreatureTurretByNPCType(FORTRESS_GATE_TURRET, GetDefenderTeam());
    UpdateCreatureTurretByNPCType(VAULT_GATE_TURRET, GetDefenderTeam());
    UpdateCreatureTurretByNPCType(NE_TOWER_TURRET, GetDefenderTeam());
    UpdateCreatureTurretByNPCType(NW_TOWER_TURRET, GetDefenderTeam());
    UpdateCreatureTurretByNPCType(SE_TOWER_TURRET, GetDefenderTeam());
    UpdateCreatureTurretByNPCType(SW_TOWER_TURRET, GetDefenderTeam());
}

void WintergraspMgr::InitAllGOforKeep()
{
    // Spawn associate gameobjects
    // In this case we keep GO for horde and alliance by separataly (because count of GO for 1 faction != count for another)
    // for horde
    for (int8 i = 0; i < WG_MAX_KEEPGO_HORDE; i++)
    {
        if (GameObject* go = SpawnGameObject(WGKeepHordeData[i].entry, WGKeepHordeData[i].Pos, WGKeepHordeData[i].Rot))
            m_KeepHordeGameObjectList.push_back(go->GetGUID());
    }

    // for alliance
    for (int8 i = 0; i < WG_MAX_KEEPGO_ALLIANCE; i++)
    {
        if (GameObject* go = SpawnGameObject(WGKeepAllianceData[i].entry, WGKeepAllianceData[i].Pos, WGKeepAllianceData[i].Rot))
            m_KeepAllianceGameObjectList.push_back(go->GetGUID());
    }

    UpdateAllGOforKeep();
}

void WintergraspMgr::UpdateAllGOforKeep()
{
    if (GetDefenderTeam() == TEAM_HORDE)
    {
        for (ObjectGuid guid : m_KeepAllianceGameObjectList)
            if (GameObject* go = GetGameObject(guid))
                go->SetRespawnTime(RESPAWN_ONE_DAY);

        for (ObjectGuid guid : m_KeepHordeGameObjectList)
            if (GameObject* go = GetGameObject(guid))
                go->SetRespawnTime(RESPAWN_IMMEDIATELY);

        // Show Alliance NPC (vendor and etc)
        for (ObjectGuid guid : m_KeepHordeNPCList)
            if (Creature* cr = GetCreature(guid))
                ShowNpc(cr, true);

        for (ObjectGuid guid : m_KeepAllianceNPCList)
            if (Creature* cr = GetCreature(guid))
                HideNpc(cr);
    }
    else
    {
        for (ObjectGuid guid : m_KeepHordeGameObjectList)
            if (GameObject* go = GetGameObject(guid))
                go->SetRespawnTime(RESPAWN_ONE_DAY);

        for (ObjectGuid guid : m_KeepAllianceGameObjectList)
            if (GameObject* go = GetGameObject(guid))
                go->SetRespawnTime(RESPAWN_IMMEDIATELY);

        for (ObjectGuid guid : m_KeepAllianceNPCList)
            if (Creature* cr = GetCreature(guid))
                ShowNpc(cr, true);

        for (ObjectGuid guid : m_KeepHordeNPCList)
            if (Creature* cr = GetCreature(guid))
                HideNpc(cr);
    }
}

void WintergraspMgr::HideNpc(Creature* creature)
{
    creature->CombatStop();
    creature->SetReactState(REACT_PASSIVE);
    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_UNINTERACTIBLE);
    creature->SetPhaseMask(2, true);
    //creature->DisappearAndDie();
    creature->SetVisible(false);
}

void WintergraspMgr::ShowNpc(Creature* creature, bool aggressive)
{
    creature->SetPhaseMask(1, true);
    creature->SetVisible(true);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_UNINTERACTIBLE);
    if (!creature->IsAlive())
        creature->Respawn(true);
    if (aggressive)
        creature->SetReactState(REACT_AGGRESSIVE);
    else
    {
        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        creature->SetReactState(REACT_PASSIVE);
    }
}

void WintergraspMgr::_UpdateCreatureForBuildGO(WintergraspGameObject GOentry, Creature* creature, TeamId team)
{
    WGGameObjectBuildingMap::const_iterator itr = m_buildingsInZone.find(GOentry);
    if (itr != m_buildingsInZone.end())
    {
        if (!itr->second->IsAlive())
            HideNpc(creature);
        else
        {
            if (itr->second->GetTeamController() == team || creature->GetEntry() == NPC_WINTERGRASP_TOWER_CANNON)
            {
                creature->SetFaction(WintergraspFaction[itr->second->GetTeamController()]);
                ShowNpc(creature, true);
            }
            else
                HideNpc(creature);
        }
    }
}

void WintergraspMgr::_UpdateCreatureForWorkshop(WintergraspWorkshopIds workshopType, Creature* creature, TeamId team)
{
    WorkshopAndCapturePointMap::const_iterator itr = m_workshopAndCaptures.find(workshopType);
    if (itr != m_workshopAndCaptures.end())
    {
        switch (creature->GetEntry())
        {
            case NPC_WORKSHOP_MECHANIC_HORDE:
            case NPC_WORKSHOP_MECHANIC_ALLIANCE:
            {
                if (itr->second._workshopPoint->GetTeamControl() == team)
                    ShowNpc(creature, true);
                else
                    HideNpc(creature);
                break;
            }
            case BATTLEFIELD_WG_NPC_GUARD_H:
            case BATTLEFIELD_WG_NPC_GUARD_A:
            {
                if (itr->second._workshopPoint->GetTeamControl() == team)
                    ShowNpc(creature, true);
                else
                    HideNpc(creature);
                break;
            }
            case NPC_TAUNKA_SPIRIT_GUIDE:
            case NPC_DWARVEN_SPIRIT_GUIDE:
            {
                if (itr->second._workshopPoint->GetTeamControl() == team)
                    ShowNpc(creature, true);
                else
                    HideNpc(creature);
                break;
            }
            default:
                break;
        }
    }
}
