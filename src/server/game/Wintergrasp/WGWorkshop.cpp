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
#include "WGWorkshop.h"
#include "WGCapturePoint.h"
#include "WGGraveyard.h"
#include "Battleground.h"
#include "GameObject.h"
#include "World.h"

struct WintergraspWorkshopData
{
    uint32 WorkshopEntry;                                // id of Workshop
    std::vector<WintergraspGameObjectData> GameObject;   // Gameobject position and entry (Horde/Alliance)
};

uint8 const WG_MAX_WORKSHOPS = 6;
WintergraspWorkshopData const WorksshopGO[WG_MAX_WORKSHOPS] =
{
    // South East Workshop
    {
        BATTLEFIELD_WG_WORKSHOP_SE,
        {
            { { 4416.59f, 2414.08f, 377.196f, 0 }, { 0.f, 0.f, 0, 1.f }, 192452, 192417 },                        // Flag on wall west
            { { 4408.57f, 2422.61f, 377.179f, 1.58825f }, { 0.f, 0.f, 0.713251f, 0.700909f }, 192451, 192416 },   // Flag on wall west
            { { 4417.94f, 2324.81f, 371.577f, 3.08051f }, { 0.f, 0.f, 0.999534f, 0.0305366f }, 192272, 192273 },  // small Flag
            { { 4417.25f, 2301.14f, 377.214f, 0.026179f }, { 0.f, 0.f, 0.0130891f, 0.999914f }, 192453, 192418 }  // Flag on wall east
        }
    },
    // South West Workshop
    {
        BATTLEFIELD_WG_WORKSHOP_SW,
        {
            { { 4399.64f, 3231.54f, 368.898f, 1.49226f }, { 0.f, 0.f, 0.678802f, 0.734321f }, 192408, 192409 },    // Flag on west side
            { { 4424.15f, 3286.54f, 371.546f, 3.12412f }, { 0.f, 0.f, 0.999962f, 0.00873622f }, 192275, 192274 },  // Flag on middle side
            { { 4401.62f, 3377.48f, 363.12f, 1.53589f }, { 0.f, 0.f, 0.694658f, 0.71934f }, 192432, 192433 },      // Flag on east side

            { { 4438.38f, 3361.01f, 371.814f, 0.0348707f }, { 0.f, 0.f, 0.0174345f, 0.999848f }, 192440, 192406 }, // banner on west side
            { { 4459.36f, 3273.83f, 370.175f, 0.0348707f }, { 0.f, 0.f, 0.0174345f, 0.999848f }, 192440, 192406 }, // banner on middle side
            { { 4448.15f, 3235.61f, 370.617f, -1.54462f }, { 0.f, 0.f, -0.704015f, 0.710185f }, 192441, 192407 }   // banner on ease side
        }
    },
    // North East Workshop
    {
        BATTLEFIELD_WG_WORKSHOP_NE,
        {
            { { 4991.08f, 2525.76f, 340.918f, 4.04044f }, { 0.f, 0.f, 0.900698f, -0.434447f }, 192290, 192291 },    // Flag on west side
            { { 5004.35f, 2486.36f, 358.449f, 2.17294f }, { 0.f, 0.f, 0.884989f, 0.465612f }, 192460, 192427 },     // banner on west side (north)
            { { 4983.22f, 2503.27f, 357.96f, -0.436332f }, { 0.f, 0.f, -0.216439f, 0.976296f }, 192461, 192428 },   // banner on west side (south)

            { { 4778.19f, 2438.06f, 345.644f, -2.94088f }, { 0.f, 0.f, -0.994969f, 0.100188f }, 192289, 192288 },   // Flag on south side
            { { 4811.44f, 2441.85f, 357.982f, -2.02458f }, { 0.f, 0.f, -0.848048f, 0.52992f }, 192458, 192425 },    // banner on west side (west)
            { { 4805.67f, 2407.48f, 358.191f, 1.78023f }, { 0.f, 0.f, 0.777144f, 0.629323f }, 192459, 192426 }      // banner on west side (east)
        }
    },
    // North West Workshop
    {
        BATTLEFIELD_WG_WORKSHOP_NW,
        {
            { { 4857.97f, 3335.44f, 368.881f, -2.94959f }, { 0.f, 0.f, -0.995395f, 0.0958539f }, 192280, 192281 },    // Flag on south side
            { { 4855.44f, 3297.6f, 376.496f, -3.11539f }, { 0.f, 0.f, -0.999914f, 0.0131009f }, 192435, 192401 },     // banner on south side
            { { 5006.32f, 3280.36f, 371.242f, 2.24275f }, { 0.f, 0.f, 0.900699f, 0.434444f }, 192283, 192282 },       // Flag on east side
            { { 5041.65f, 3294.32f, 381.92f, -1.6057f }, { 0.f, 0.f, -0.719339f, 0.694659f }, 192434, 192400 }        // banner on east side
        }
    },
    // fortness West Workshop
    {
        BATTLEFIELD_WG_WORKSHOP_KEEP_WEST,
        {

        }
    },
    // fortness West Workshop
    {
        BATTLEFIELD_WG_WORKSHOP_KEEP_EAST,
        {

        }
    }
};

uint32 WGWorkshop::_GetWorldStateForType(WintergraspWorkshopIds type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_WORKSHOP_SE: return WS_BATTLEFIELD_WG_WORKSHOP_SE; break;
        case BATTLEFIELD_WG_WORKSHOP_SW: return WS_BATTLEFIELD_WG_WORKSHOP_SW; break;
        case BATTLEFIELD_WG_WORKSHOP_NE: return WS_BATTLEFIELD_WG_WORKSHOP_NE; break;
        case BATTLEFIELD_WG_WORKSHOP_NW: return WS_BATTLEFIELD_WG_WORKSHOP_NW; break;
        case BATTLEFIELD_WG_WORKSHOP_KEEP_WEST: return WS_BATTLEFIELD_WG_WORKSHOP_K_W; break;
        case BATTLEFIELD_WG_WORKSHOP_KEEP_EAST: return WS_BATTLEFIELD_WG_WORKSHOP_K_E; break;
        default:
            break;
    }

    return 0;
}

uint8 WGWorkshop::_GetAllianceCapture(WintergraspWorkshopIds type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_WORKSHOP_SE: return BATTLEFIELD_WG_TEXT_EASTSPARK_CAPTURE_ALLIANCE; break;
        case BATTLEFIELD_WG_WORKSHOP_SW: return BATTLEFIELD_WG_TEXT_WESTSPARK_CAPTURE_ALLIANCE; break;
        case BATTLEFIELD_WG_WORKSHOP_NE: return BATTLEFIELD_WG_TEXT_SUNKEN_RING_CAPTURE_ALLIANCE; break;
        case BATTLEFIELD_WG_WORKSHOP_NW: return BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_CAPTURE_ALLIANCE; break;
        default:
            break;
    }
    return 0;
}

uint8 WGWorkshop::_GetAllianceAttack(WintergraspWorkshopIds type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_WORKSHOP_SE: return BATTLEFIELD_WG_TEXT_EASTSPARK_ATTACK_ALLIANCE; break;
        case BATTLEFIELD_WG_WORKSHOP_SW: return BATTLEFIELD_WG_TEXT_WESTSPARK_ATTACK_ALLIANCE; break;
        case BATTLEFIELD_WG_WORKSHOP_NE: return BATTLEFIELD_WG_TEXT_SUNKEN_RING_ATTACK_ALLIANCE; break;
        case BATTLEFIELD_WG_WORKSHOP_NW: return BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_ATTACK_ALLIANCE; break;
        default:
            break;
    }
    return 0;
}

uint8 WGWorkshop::_GetHordeCapture(WintergraspWorkshopIds type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_WORKSHOP_SE: return BATTLEFIELD_WG_TEXT_EASTSPARK_CAPTURE_HORDE; break;
        case BATTLEFIELD_WG_WORKSHOP_SW: return BATTLEFIELD_WG_TEXT_WESTSPARK_CAPTURE_HORDE; break;
        case BATTLEFIELD_WG_WORKSHOP_NE: return BATTLEFIELD_WG_TEXT_SUNKEN_RING_CAPTURE_HORDE; break;
        case BATTLEFIELD_WG_WORKSHOP_NW: return BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_CAPTURE_HORDE; break;
        default:
            break;
    }
    return 0;
}

uint8 WGWorkshop::_GetHordeAttack(WintergraspWorkshopIds type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_WORKSHOP_SE: return BATTLEFIELD_WG_TEXT_EASTSPARK_ATTACK_HORDE; break;
        case BATTLEFIELD_WG_WORKSHOP_SW: return BATTLEFIELD_WG_TEXT_WESTSPARK_ATTACK_HORDE; break;
        case BATTLEFIELD_WG_WORKSHOP_NE: return BATTLEFIELD_WG_TEXT_SUNKEN_RING_ATTACK_HORDE; break;
        case BATTLEFIELD_WG_WORKSHOP_NW: return BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_ATTACK_HORDE; break;
        default:
            break;
    }
    return 0;
}

WGWorkshop::WGWorkshop(WintergraspMgr* wg, WintergraspWorkshopIds type)
{
    ASSERT(wg && type < BATTLEFIELD_WG_WORKSHOP_MAX);
    _wg              = wg;
    _buildGUID.Clear();
    _state           = BATTLEFIELD_WG_OBJECTSTATE_NONE;
    _teamControl     = TEAM_NEUTRAL;
    _type            = type;
    _worldStateId    = _GetWorldStateForType(type);
    _AllianceCapture = _GetAllianceCapture(type);
    _AllianceAttack  = _GetAllianceAttack(type);
    _HordeCapture    = _GetHordeCapture(type);
    _HordeAttack     = _GetHordeAttack(type);

    if (type < BATTLEFIELD_WG_WORKSHOP_KEEP_WEST)
    {
        // Spawn associate gameobjects
        for (WintergraspGameObjectData const& gobData : WorksshopGO[type].GameObject)
        {
            if (GameObject* goHorde = _wg->SpawnGameObject(gobData.HordeEntry, gobData.Pos, gobData.Rot))
                m_GOList[TEAM_HORDE].push_back(goHorde->GetGUID());

            if (GameObject* goAlliance = _wg->SpawnGameObject(gobData.AllianceEntry, gobData.Pos, gobData.Rot))
                m_GOList[TEAM_ALLIANCE].push_back(goAlliance->GetGUID());
        }
    }
}

WGWorkshop::~WGWorkshop()
{
    _wg = nullptr;
    _buildGUID.Clear();
    _state = BATTLEFIELD_WG_OBJECTSTATE_NONE;
    _teamControl = TEAM_ALLIANCE;
    _type = BATTLEFIELD_WG_WORKSHOP_SE;
    // note : in WintergraspWorkshop  in m_GameObjectList we have 3 team (TEAM_NEUTRALL) too
    CleanBeforeDelete();
}

void WGWorkshop::GiveControlTo(TeamId teamId, bool init /*= false*/)
{
    switch (teamId)
    {
        case TEAM_NEUTRAL:
        {
            // Updating worldstate
            _state = WintergraspGameObjectState(BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_INTACT);
            _wg->SendUpdateWorldState(_worldStateId, _state);

            if (!init)
            {
                if (_teamControl == TEAM_ALLIANCE)
                    _wg->SendWarning(_HordeAttack); // workshop taken - horde
                else
                    _wg->SendWarning(_AllianceAttack); // workshop taken - alliance
            }
            break;
        }
        case TEAM_ALLIANCE:
        {
            // Updating worldstate
            _state = WintergraspGameObjectState(BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_INTACT);
            _wg->SendUpdateWorldState(_worldStateId, _state);

            // Warning message
            if (!init)
                _wg->SendWarning(_AllianceCapture); // workshop taken - alliance
            break;
        }
        case TEAM_HORDE:
        {
            // Update worldstate
            _state = WintergraspGameObjectState(BATTLEFIELD_WG_OBJECTSTATE_HORDE_INTACT);
            _wg->SendUpdateWorldState(_worldStateId, _state);

            // Warning message
            if (!init)
                _wg->SendWarning(_HordeCapture); // workshop taken - horde
            break;
        }
    }

    // Found associate graveyard and update it
    if (_type < BATTLEFIELD_WG_WORKSHOP_KEEP_WEST)
    {
        uint32 gyID = _wg->_GetGraveyardIDByType(WGGraveyardId(_type));
        if (WGGraveyard* gy = _wg->GetGraveyardById(gyID))
            gy->GiveControlTo(teamId);
    }

    _teamControl = teamId;

    if (!init)
        _wg->UpdateCounterVehicle(false);

    UpdateCreatureAndGo();
    Save();
}

void WGWorkshop::UpdateCreatureAndGo()
{
    switch (_type)
    {
        case BATTLEFIELD_WG_WORKSHOP_SE:
        {
            _wg->ShowCreatureByNPCType(WG_WORKSHOP_SE, _teamControl);
            if (_teamControl != TEAM_NEUTRAL)
                _wg->HideCreatureByNPCType(WG_WORKSHOP_SE, _wg->GetOtherTeam(_teamControl));
            else
            {
                _wg->HideCreatureByNPCType(WG_WORKSHOP_SE, TEAM_ALLIANCE);
                _wg->HideCreatureByNPCType(WG_WORKSHOP_SE, TEAM_HORDE);
            }
            break;
        }
        case BATTLEFIELD_WG_WORKSHOP_SW:
        {
            _wg->ShowCreatureByNPCType(WG_WORKSHOP_SW, _teamControl);
            if (_teamControl != TEAM_NEUTRAL)
                _wg->HideCreatureByNPCType(WG_WORKSHOP_SW, _wg->GetOtherTeam(_teamControl));
            else
            {
                _wg->HideCreatureByNPCType(WG_WORKSHOP_SW, TEAM_ALLIANCE);
                _wg->HideCreatureByNPCType(WG_WORKSHOP_SW, TEAM_HORDE);
            }
            break;
        }
        case BATTLEFIELD_WG_WORKSHOP_NE:
        {
            _wg->ShowCreatureByNPCType(WG_WORKSHOP_NE, _teamControl);
            if (_teamControl != TEAM_NEUTRAL)
                _wg->HideCreatureByNPCType(WG_WORKSHOP_NE, _wg->GetOtherTeam(_teamControl));
            else
            {
                _wg->HideCreatureByNPCType(WG_WORKSHOP_NE, TEAM_ALLIANCE);
                _wg->HideCreatureByNPCType(WG_WORKSHOP_NE, TEAM_HORDE);
            }
            break;
        }
        case BATTLEFIELD_WG_WORKSHOP_NW:
        {
            _wg->ShowCreatureByNPCType(WG_WORKSHOP_NW, _teamControl);
            if (_teamControl != TEAM_NEUTRAL)
                _wg->HideCreatureByNPCType(WG_WORKSHOP_NW, _wg->GetOtherTeam(_teamControl));
            else
            {
                _wg->HideCreatureByNPCType(WG_WORKSHOP_NW, TEAM_ALLIANCE);
                _wg->HideCreatureByNPCType(WG_WORKSHOP_NW, TEAM_HORDE);
            }
            break;
        }
        case BATTLEFIELD_WG_WORKSHOP_KEEP_WEST:
        {
            _wg->ShowCreatureByNPCType(WG_WORKSHOP_KEEP_WEST, _teamControl);
            if (_teamControl != TEAM_NEUTRAL)
                _wg->HideCreatureByNPCType(WG_WORKSHOP_KEEP_WEST, _wg->GetOtherTeam(_teamControl));
            else
            {
                _wg->HideCreatureByNPCType(WG_WORKSHOP_KEEP_WEST, TEAM_ALLIANCE);
                _wg->HideCreatureByNPCType(WG_WORKSHOP_KEEP_WEST, TEAM_HORDE);
            }
            break;
        }
        case BATTLEFIELD_WG_WORKSHOP_KEEP_EAST:
        {
            _wg->ShowCreatureByNPCType(WG_WORKSHOP_KEEP_EAST, _teamControl);
            if (_teamControl != TEAM_NEUTRAL)
                _wg->HideCreatureByNPCType(WG_WORKSHOP_KEEP_EAST, _wg->GetOtherTeam(_teamControl));
            else
            {
                _wg->HideCreatureByNPCType(WG_WORKSHOP_KEEP_EAST, TEAM_ALLIANCE);
                _wg->HideCreatureByNPCType(WG_WORKSHOP_KEEP_EAST, TEAM_HORDE);
            }
            break;
        }
        default:
            break;
    }

    // Show GameObjects for TeamControll
    if (!m_GOList[_teamControl].empty())
        for (ObjectGuid guid : m_GOList[_teamControl])
            if (GameObject* go = _wg->GetGameObject(guid))
            {
                go->SetRespawnTime(RESPAWN_IMMEDIATELY);
                go->setActive(true);
                go->SetFarVisible(true);
            }

    // Hide GO for another teams
    if (_teamControl != TEAM_NEUTRAL)
    {
        TeamId otherTeam = _wg->GetOtherTeam(_teamControl);
        if (!m_GOList[otherTeam].empty())
            for (ObjectGuid guid : m_GOList[otherTeam])
                if (GameObject* go = _wg->GetGameObject(guid))
                    go->SetRespawnTime(RESPAWN_ONE_DAY);
    }
    else
    {
        // Need to hide both GOs for alliance and horde
        if (!m_GOList[TEAM_ALLIANCE].empty())
            for (ObjectGuid guid : m_GOList[TEAM_ALLIANCE])
                if (GameObject* go = _wg->GetGameObject(guid))
                    go->SetRespawnTime(RESPAWN_ONE_DAY);

        if (!m_GOList[TEAM_HORDE].empty())
            for (ObjectGuid guid : m_GOList[TEAM_HORDE])
                if (GameObject* go = _wg->GetGameObject(guid))
                    go->SetRespawnTime(RESPAWN_ONE_DAY);
    }
}

void WGWorkshop::UpdateGraveyardAndWorkshop()
{
    TeamId team = TEAM_NEUTRAL;
    if (_type < BATTLEFIELD_WG_WORKSHOP_NE)
        team = _wg->GetAttackerTeam();
    else
        team = _wg->GetDefenderTeam();

    GiveControlTo(team, true);
    if (_wg->GetCapturePoint(_type))
        _wg->GetCapturePoint(_type)->SetInitialData(team);
}

void WGWorkshop::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(_worldStateId, _state);
}

void WGWorkshop::Save()
{
    sWorld->setWorldState(_worldStateId, _state);
}

void WGWorkshop::CleanBeforeDelete()
{
    for (int8 i = 0; i < 3; i++)
    {
        for (ObjectGuid guid : m_GOList[i])
            if (GameObject* go = _wg->GetGameObject(guid))
                go->DespawnOrUnsummon();
        m_GOList[i].clear();
    }
}
