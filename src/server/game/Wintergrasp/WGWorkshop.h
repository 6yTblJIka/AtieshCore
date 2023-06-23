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
#include "WorldStatePackets.h"

class WGWorkshop
{
public:
    WGWorkshop(WintergraspMgr* wg, WintergraspWorkshopIds type);
    ~WGWorkshop();

    WintergraspWorkshopIds GetType() const { return _type; }
    TeamId GetTeamControl() const { return _teamControl; }

    // Called on change faction in CapturePoint class
    void GiveControlTo(TeamId teamId, bool init = false);

    void UpdateGraveyardAndWorkshop();

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet);

    void Save();

    void UpdateCreatureAndGo();

    void CleanBeforeDelete();

private:
    uint32 _GetWorldStateForType(WintergraspWorkshopIds workshopType);
    uint8 _GetAllianceCapture(WintergraspWorkshopIds workshopType);
    uint8 _GetAllianceAttack(WintergraspWorkshopIds workshopType);
    uint8 _GetHordeCapture(WintergraspWorkshopIds workshopType);
    uint8 _GetHordeAttack(WintergraspWorkshopIds workshopType);

    WintergraspMgr* _wg;                            // Pointer to wintergrasp
    ObjectGuid _buildGUID;
    WintergraspGameObjectState _state;              // For worldstate
    TeamId _teamControl;                            // Team witch control the workshop
    WintergraspWorkshopIds _type;

    uint32 _worldStateId;
    uint8 _AllianceCapture;
    uint8 _AllianceAttack;
    uint8 _HordeCapture;
    uint8 _HordeAttack;

    // GameObject associations
    GuidVector m_GOList[3];
};
