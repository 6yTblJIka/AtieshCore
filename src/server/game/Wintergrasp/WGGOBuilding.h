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

class WGGameObjectBuilding
{
public:
    WGGameObjectBuilding(WintergraspMgr* wg, WintergraspGameObjectBuildingType type, uint32 worldState);
    ~WGGameObjectBuilding();

    void Init(GameObject* go);
    ObjectGuid const& GetGUID() const { return _GOGUID; }

    void Rebuild();

    // Called when associated gameobject is damaged
    void Damaged();
    // Called when associated gameobject is destroyed
    void Destroyed();

    void UpdateGo();
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet);

    void Save();

    uint32 GetGOEntry() { return _GOentry; }
    bool IsTower() { return _isTower; }
    bool IsAttackTower() { return _type == BATTLEFIELD_WG_OBJECTTYPE_TOWER; }
    bool IsDefenseTower() { return _type == BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER; }
    bool IsAlive() { return _state != BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_DESTROY && _state != BATTLEFIELD_WG_OBJECTSTATE_HORDE_DESTROY && _state != BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DESTROY; }

    TeamId GetTeamController() { return _teamControl; }
    WintergraspGameObjectBuildingType GetType() { return _type; }
    WintergraspTowerIds GetTowerId() { return _TowerId; }
    ObjectGuid GetGUID() { return _GOGUID; }

    void CleanBeforeDelete();

private:
    uint8 _GetTextDamaged(WintergraspGameObjectBuildingType type);
    uint8 _GetTextDestroyed(WintergraspGameObjectBuildingType type);

    // WG object
    WintergraspMgr* _wg;
    // Linked gameobject
    ObjectGuid _GOGUID;
    // linked gameobject ID building
    uint32 _GOentry;
    // the team that controls this point
    TeamId _teamControl;
    WintergraspGameObjectBuildingType _type;
    uint32 _worldState;
    WintergraspGameObjectState _state;
    // GameObject associations
    GuidVector m_GameObjectList[3];

    bool _isTower;
    WintergraspTowerIds _TowerId;
    uint8 _TextDamaged;
    uint8 _TextDestroyed;
};
