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
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "WorldStatePackets.h"

class GameObject;
class Player;
class WGWorkshop;

class TC_GAME_API WGCapturePoint
{
public:
    WGCapturePoint(WintergraspMgr* WG, WintergraspWorkshopIds workshoptype, TeamId team);
    ~WGCapturePoint();

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }

    // Send world state update to all players present
    void SendUpdateWorldState(uint32 field, uint32 value);

    // Send kill notify to players in the controlling faction
    void SendObjectiveComplete(uint32 id, ObjectGuid guid);

    // Used when player is activated/inactivated in the area
    bool HandlePlayerEnter(Player* player);
    GuidSet::iterator HandlePlayerLeave(Player* player);
    //virtual void HandlePlayerActivityChanged(Player* player);

    // Checks if player is in range of a capture credit marker
    bool IsInsideObjective(Player* player) const;

    // Returns true if the state of the objective has changed, in this case, the OutdoorPvP must send a world state ui update.
    bool Update(uint32 diff);
    void ChangeTeam(TeamId newTeam);
    void SendChangePhase();
    bool ChangeState();

    void SetInitialData(TeamId team);
    bool SetCapturePointData(GameObject* capturePoint);
    GameObject* GetCapturePointGo();
    uint32 GetCapturePointEntry() const { return m_capturePointEntry; }

    TeamId GetTeamId() const { return m_team; }
protected:
    bool DelCapturePoint();

    // active Players in the area of the objective, 0 - alliance, 1 - horde
    GuidSet m_activePlayers[PVP_TEAMS_COUNT];

    // Total shift needed to capture the objective
    float m_maxValue;
    float m_minValue;

    // Maximum speed of capture
    float m_maxSpeed;

    // The status of the objective
    float m_value;
    TeamId m_team;

    // Objective states
    BattlefieldObjectiveStates m_OldState;
    BattlefieldObjectiveStates m_State;

    // Neutral value on capture bar
    uint32 m_neutralValuePct;

    // Pointer to the Battlefield this objective belongs to
    WintergraspMgr* m_WG;

    // Capture point entry
    uint32 m_capturePointEntry;

    // Gameobject related to that capture point
    ObjectGuid m_capturePointGUID;
};
