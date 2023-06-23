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

#include "WGGOBuilding.h"
#include "Battleground.h"
#include "WintergraspMgr.h"
#include "GameObject.h"

 // *********************************************************
 // **********Tower Element(GameObject, Creature)************
 // *********************************************************
struct WintergraspTowerData
{
    uint32 towerEntry;                                   // Gameobject id of tower
    std::vector<WintergraspGameObjectData> GameObject;   // Gameobject position and entry (Horde/Alliance)
};

uint8 const WG_MAX_ATTACKTOWERS = 3;
// 192414 : 0 in sql, 1 in header
// 192278 : 0 in sql, 3 in header
WintergraspTowerData const AttackTowers[WG_MAX_ATTACKTOWERS] =
{
    // West tower
    {
        190356,
        {
            { { 4559.113f, 3606.216f, 419.9992f, 4.799657f }, { 0.f, 0.f, -0.67558960f, 0.73727790f }, 192488, 192501 },    // Flag on tower
            { { 4539.420f, 3622.490f, 420.0342f, 3.211419f }, { 0.f, 0.f, -0.99939060f, 0.03490613f }, 192488, 192501 },    // Flag on tower
            { { 4555.258f, 3641.648f, 419.9740f, 1.675514f }, { 0.f, 0.f,  0.74314400f, 0.66913150f }, 192488, 192501 },    // Flag on tower
            { { 4574.872f, 3625.911f, 420.0792f, 0.087266f }, { 0.f, 0.f,  0.04361916f, 0.99904820f }, 192488, 192501 },    // Flag on tower
            { { 4433.899f, 3534.142f, 360.2750f, 4.433136f }, { 0.f, 0.f, -0.79863550f, 0.60181500f }, 192276, 192278 },    // Flag near workshop
            { { 4572.933f, 3475.519f, 363.0090f, 1.422443f }, { 0.f, 0.f,  0.65275960f, 0.75756520f }, 192276, 192277 }     // Flag near bridge
        }
    },
    // South Tower
    {
        190357,
        {
            { { 4416.004f, 2822.666f, 429.8512f, 6.2657330f }, { 0.f, 0.f, -0.00872612f, 0.99996190f }, 192488, 192501 },    // Flag on tower
            { { 4398.819f, 2804.698f, 429.7920f, 4.6949370f }, { 0.f, 0.f, -0.71325020f, 0.70090960f }, 192488, 192501 },    // Flag on tower
            { { 4380.36f, 2822.38f, 429.882f, -3.10665f }, { 0.f, 0.f, -0.999847f, 0.0174704f }, 192488, 192501 },           // Flag on tower
            { { 4397.66f, 2840.3f, 429.922f, 1.58825f }, { 0.f, 0.f, 0.713251f, 0.700909f }, 192488, 192501 },               // Flag on tower

            { { 4425.516f, 2976.093f, 367.4235f, 3.2899610f }, { 0.f, 0.f, -0.99724960f, 0.07411628f }, 192276, 192278 },    // Flag in west side
            { { 4387.622f, 2719.566f, 389.9351f, 4.7385700f }, { 0.f, 0.f, -0.69779010f, 0.71630230f }, 192366, 192414 },    // Flag in east side
            { { 4452.76f, 2639.14f, 358.444f, 1.67552f }, { 0.f, 0.f, 0.743146f, 0.669129f }, 192267, 192266 },              // Flag in east side

            { { 4526.457f, 2810.181f, 391.1997f, 3.2899610f }, { 0.f, 0.f, -0.99724960f, 0.07411628f }, 192276, 192278 },    // Flag in north side (near bridge)

            { { 4349.9f, 2885.56f, 406.105f, 1.6057f }, { 0.f, 0.f,  0.719339f, 0.694659f }, 192366, 192429 },               // Flag big on west side
            { { 4517.75f, 2717.07f, 387.812f, -1.54462f }, { 0.f, 0.f, -0.697792f, 0.716301f }, 192449, 192415 },            // Flag big on east side
            { { 4434.285f, 2883.906f, 405.5880f, 0.8290324f }, { 0.f, 0.f,  0.40274720f, 0.91531130f }, 192366, 192429 },    // Flag big on north side
            { { 4464.124f, 2855.453f, 406.1106f, 0.8290324f }, { 0.f, 0.f,  0.40274720f, 0.91531130f }, 192366, 192429 }     // Flag big on north side
        }
    },
    // East Tower
    {
        190358,
        {
            { { 4475.35f, 1937.03f, 459.07f, -0.436332f }, { 0.f, 0.f,  -0.216439f, 0.976296f }, 192502, 192501 },     // Flag on tower
            { { 4466.79f, 1960.42f, 459.144f, 1.15192f }, { 0.f, 0.f,  0.54464f, 0.83867f }, 192502, 192501 },         // Flag on tower
            { { 4442.99f, 1951.9f, 459.093f, 2.74016f }, { 0.f, 0.f,  0.979924f, 0.199371f }, 192502, 192501 },        // Flag on tower
            { { 4451.76f, 1928.1f, 459.076f, -2.00713f }, { 0.f, 0.f,  -0.843392f, 0.537299f }, 192502, 192501 },      // Flag on tower
            { { 4563.73f, 2171.15f, 367.68f, 1.30027f }, { 0.f, 0.f,  0.605294f, 0.796002f }, 192424, 192423 }         // Flag on west side
        }
    }
};

uint8 const WG_MAX_DEFENSETOWERS = 4;
// 192414 : 0 in sql, 1 in header
// 192278 : 0 in sql, 3 in header
WintergraspTowerData const DefenseTowers[WG_MAX_DEFENSETOWERS] =
{
    // NW
    {
        190221,
        {
            // outside
            { { 5352.15f, 3054.77f, 444.61f, 1.57952f }, { 0.f, 0.f,  0.710184f, 0.704016f }, 192376, 192310 },     // banner on noth side
            { { 5280.89f, 3064.95f, 431.976f, 1.55334f }, { 0.f, 0.f,  0.700908f, 0.713252f }, 192488, 192487 },    // banner on west side
            { { 5262.54f, 3047.93f, 431.965f, 3.12412f }, { 0.f, 0.f,  0.999962f, 0.00873622f }, 192488, 192487 },  // banner on south side
            { { 5272.55f, 2976.54f, 444.493f, 3.13286f }, { 0.f, 0.f,  0.99999f, 0.00436634f }, 192374, 192309 },   // banner on east side
            // inside (NW workshop)
            { { 5392.64f, 3036.97f, 433.649f, -1.51843f }, { 0.f, 0.f,  -0.688352f, 0.725377f }, 192361, 192326 },
            { { 5352.38f, 3036.95f, 435.111f, -1.56207f }, { 0.f, 0.f,  -0.704015f, 0.710185f }, 192360, 192325 },
            { { 5290.51f, 2976.56f, 435.087f, 0.00872424f }, { 0.f, 0.f,  0.00436211f, 0.99999f }, 192373, 192324 },
            { { 5322.89f, 2917.14f, 445.154f, 1.56207f }, { 0.f, 0.f,  0.704015f, 0.710185f }, 192372, 192322 },
            { { 5364.28f, 2917.26f, 445.332f, 1.58825f }, { 0.f, 0.f,  0.713251f, 0.700909f }, 192371, 192323 },
            { { 5398.04f, 2873.01f, 455.204f, 3.13286f }, { 0.f, 0.f,  0.99999f, 0.00436634f }, 192338, 192304 },
            { { 5364.3f, 2899.22f, 435.691f, -1.55334f }, { 0.f, 0.f,  -0.700908f, 0.713252f }, 192359, 192319 },
            { { 5322.25f, 2898.95f, 435.643f, -1.57952f }, { 0.f, 0.f,  -0.710184f, 0.704016f }, 192358, 192318 },
            { { 5288.85f, 2861.82f, 435.59f, 0.0261791f }, { 0.f, 0.f,  0.0130892f, 0.999914f }, 192354, 192321 },
            { { 5372.42f, 2862.48f, 409.366f, 3.14159f }, { 0.f, 0.f,  1.f, 0.00000126759f }, 192284, 192287 }
        }
    },
    // SW
    {
        190373,
        {
            // outside
            { { 5235.13f, 2942.12f, 444.279f, 1.58825f }, { 0.f, 0.f,  0.713251f, 0.700909f }, 192375, 192308 },
            { { 5162.91f, 2952.6f, 433.368f, 1.5708f }, { 0.f, 0.f,  0.707108f, 0.707106f }, 192488, 192487 },
            { { 5145.11f, 2934.95f, 433.255f, -3.10665f }, { 0.f, 0.f,  -0.999847f, 0.0174704f }, 192488, 192487 },
            { { 5158.81f, 2883.13f, 431.618f, 3.14159f }, { 0.f, 0.f,  1.f, 0.00000126759f }, 192488, 192487 },
            { { 5154.35f, 2862.08f, 445.01f, 3.14159f }, { 0.f, 0.f,  1.f, 0.00000126759f }, 192336, 192292 },
            { { 5154.37f, 2853.23f, 409.183f, 3.14159f }, { 0.f, 0.f,  1.f, 0.00000126759f }, 192255, 192252 },
            // inside
            { { 5172.34f, 2862.58f, 435.658f, 0.f }, { 0.f, 0.f,  0.f, 1.f }, 192353, 192327 },
            { { 5235.32f, 2924.31f, 434.898f, -1.56207f }, { 0.f, 0.f,  -0.704015f, 0.710185f }, 192357, 192329 },
            { { 5270.55f, 2861.68f, 444.917f, -3.12412f }, { 0.f, 0.f,  -0.999962f, 0.00873622f }, 192350, 192306 },
        }
    },
    // SE
    {
        190377,
        {
            // outside
            { { 5154.42f, 2828.93f, 409.189f, 3.14159f }, { 0.f, 0.f,  1.0f, 0.00000126759f }, 192254, 192253 },
            { { 5155.22f, 2820.63f, 444.979f, -3.11539f }, { 0.f, 0.f,  -0.999914f, 0.0131009f }, 192349, 192299 },
            { { 5160.34f, 2798.61f, 430.769f, 3.14159f }, { 0.f, 0.f,  1.f, 0.00000126759f }, 192488, 192487 },
            { { 5146.04f, 2747.3f, 433.527f, 3.12412f }, { 0.f, 0.f,  0.999962f, 0.00873622f }, 192488, 192487 },
            { { 5163.85f, 2729.68f, 433.328f, -1.6057f }, { 0.f, 0.f,  -0.719339f, 0.694659f }, 192488, 192487 },
            { { 5236.31f, 2739.22f, 444.993f, -1.6057f }, { 0.f, 0.f,  -0.719339f, 0.694659f }, 192366, 192312 },
            // inside
            { { 5271.16f, 2820.11f, 445.109f, -3.13286f }, { 0.f, 0.f,  -0.99999f, 0.00436634f }, 192351, 192307 },
            { { 5237.02f, 2757.36f, 435.626f, 1.55334f }, { 0.f, 0.f,  0.700908f, 0.713252f }, 192356, 192330 },
            { { 5173.13f, 2820.96f, 435.658f, 0.0261791f }, { 0.f, 0.f,  0.0130892f, 0.999914f }, 192352, 192328 }
        }
    },
    // NE
    {
        190378,
        {
            // outside
            { { 5271.63f, 2704.83f, 445.183f, -3.12412f }, { 0.f, 0.f,  -0.999962f, 0.00873622f }, 192367, 192313 },
            { { 5260.82f, 2631.82f, 433.181f, 3.12412f }, { 0.f, 0.f,  0.999962f, 0.00873622f }, 192488, 192487 },
            { { 5278.43f, 2613.83f, 433.294f, -1.62316f }, { 0.f, 0.f,  -0.725376f, 0.688353f }, 192488, 192487 },
            { { 5350.9f, 2622.48f, 444.649f, -1.56207f }, { 0.f, 0.f,  -0.704015f, 0.710185f }, 192364, 192314 },
            // inside
            { { 5392.28f, 2639.84f, 435.208f, 1.52716f }, { 0.f, 0.f,  0.691512f, 0.722365f }, 192370, 192333 },
            { { 5350.94f, 2640.43f, 435.264f, 1.56207f }, { 0.f, 0.f,  0.704015f, 0.710185f }, 192369, 192332 },
            { { 5289.78f, 2704.62f, 435.714f, 0.00872424f }, { 0.f, 0.f,  0.00436211f, 0.99999f }, 192368, 192331 },
            { { 5322.17f, 2763.2f, 444.974f, -1.56207f }, { 0.f, 0.f,  -0.704015f, 0.710185f }, 192362, 192334 },
            { { 5363.72f, 2763.25f, 445.023f, -1.54462f }, { 0.f, 0.f,  -0.697792f, 0.716301f }, 192363, 192335 },
            { { 5397.31f, 2809.26f, 455.102f, 3.13286f }, { 0.f, 0.f,  0.99999f, 0.00436634f }, 192339, 192305 },
            { { 5363.39f, 2781.28f, 435.634f, 1.58825f }, { 0.f, 0.f,  0.713251f, 0.700909f }, 192379, 192317 },
            { { 5322.01f, 2781.13f, 435.673f, 1.57952f }, { 0.f, 0.f,  0.710184f, 0.704016f }, 192378, 192316 },
            { { 5289.05f, 2820.23f, 435.674f, 0.f }, { 0.f, 0.f,  0.f, 1.f }, 192355, 192320 },
            { { 5371.45f, 2820.79f, 409.427f, 3.12412f }, { 0.f, 0.f,  0.999962f, 0.00873622f }, 192285, 192286 }
        }
    }
};

uint8 WGGameObjectBuilding::_GetTextDamaged(WintergraspGameObjectBuildingType type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER:
        case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
        {
            switch (_TowerId)
            {
                case BATTLEFIELD_WG_TOWER_FORTRESS_NW: return BATTLEFIELD_WG_TEXT_NW_KEEPTOWER_DAMAGE; break;
                case BATTLEFIELD_WG_TOWER_FORTRESS_SW: return BATTLEFIELD_WG_TEXT_SW_KEEPTOWER_DAMAGE; break;
                case BATTLEFIELD_WG_TOWER_FORTRESS_SE: return BATTLEFIELD_WG_TEXT_SE_KEEPTOWER_DAMAGE; break;
                case BATTLEFIELD_WG_TOWER_FORTRESS_NE: return BATTLEFIELD_WG_TEXT_NE_KEEPTOWER_DAMAGE; break;
                case BATTLEFIELD_WG_TOWER_SHADOWSIGHT: return BATTLEFIELD_WG_TEXT_WESTERN_TOWER_DAMAGE; break;
                case BATTLEFIELD_WG_TOWER_WINTER_S_EDGE: return BATTLEFIELD_WG_TEXT_SOUTHERN_TOWER_DAMAGE; break;
                case BATTLEFIELD_WG_TOWER_FLAMEWATCH: return BATTLEFIELD_WG_TEXT_EASTERN_TOWER_DAMAGE; break;
            }
            break;
        }
        default:
            break;
    }
    return 0;
}

uint8 WGGameObjectBuilding::_GetTextDestroyed(WintergraspGameObjectBuildingType type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER:
        case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
        {
            switch (_TowerId)
            {
                case BATTLEFIELD_WG_TOWER_FORTRESS_NW: return BATTLEFIELD_WG_TEXT_NW_KEEPTOWER_DESTROY; break;
                case BATTLEFIELD_WG_TOWER_FORTRESS_SW: return BATTLEFIELD_WG_TEXT_SW_KEEPTOWER_DESTROY; break;
                case BATTLEFIELD_WG_TOWER_FORTRESS_SE: return BATTLEFIELD_WG_TEXT_SE_KEEPTOWER_DESTROY; break;
                case BATTLEFIELD_WG_TOWER_FORTRESS_NE: return BATTLEFIELD_WG_TEXT_NE_KEEPTOWER_DESTROY; break;
                case BATTLEFIELD_WG_TOWER_SHADOWSIGHT: return BATTLEFIELD_WG_TEXT_WESTERN_TOWER_DESTROY; break;
                case BATTLEFIELD_WG_TOWER_WINTER_S_EDGE: return BATTLEFIELD_WG_TEXT_SOUTHERN_TOWER_DESTROY; break;
                case BATTLEFIELD_WG_TOWER_FLAMEWATCH: return BATTLEFIELD_WG_TEXT_EASTERN_TOWER_DESTROY; break;
            }
            break;
        }
        default:
            break;
    }
    return 0;
}

WGGameObjectBuilding::WGGameObjectBuilding(WintergraspMgr* wg, WintergraspGameObjectBuildingType type, uint32 worldState)
{
    ASSERT(wg);
    _wg            = wg;
    _GOGUID.Clear();
    _GOentry       = 0;
    _teamControl   = TEAM_NEUTRAL;
    _type          = type;
    _worldState    = worldState;
    _state = WintergraspGameObjectState(sWorld->getWorldState(worldState));
    for (int8 i = 0; i < 3; i++)
        m_GameObjectList[i].clear();
    _isTower       = false;
    _TowerId       = BATTLEFIELD_WG_TOWER_FORTRESS_NW;
    _TextDamaged   = _GetTextDamaged(type);
    _TextDestroyed = _GetTextDestroyed(type);
}

WGGameObjectBuilding::~WGGameObjectBuilding()
{
    _wg            = nullptr;
    _GOGUID.Clear();
    _GOentry       = 0;
    _teamControl   = TEAM_ALLIANCE;
    _type          = BATTLEFIELD_WG_OBJECTTYPE_DOOR;
    _worldState    = 0;
    _state         = BATTLEFIELD_WG_OBJECTSTATE_NONE;
    CleanBeforeDelete();
    _isTower = false;
    _TowerId       = BATTLEFIELD_WG_TOWER_FORTRESS_NW;
    _TextDamaged   = 0;
    _TextDestroyed = 0;
}

void WGGameObjectBuilding::Rebuild()
{
    switch (_type)
    {
        case BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER:
        case BATTLEFIELD_WG_OBJECTTYPE_DOOR_LAST:
        case BATTLEFIELD_WG_OBJECTTYPE_DOOR:
        case BATTLEFIELD_WG_OBJECTTYPE_WALL:
            _teamControl = _wg->GetDefenderTeam();      // Objects that are part of the keep should be the defender's
            break;
        case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
            _teamControl = _wg->GetAttackerTeam();      // The towers in the south should be the attacker's
            break;
        default:
            _teamControl = TEAM_NEUTRAL;
            break;
    }

    // Update worldstate
    _state = WintergraspGameObjectState(BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_INTACT - (_teamControl * 3));
    _wg->SendUpdateWorldState(_worldState, _state);

    if (GameObject* build = _wg->GetGameObject(_GOGUID))
    {
        // Rebuild gameobject
        if (build->IsDestructibleBuilding())
        {
            build->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING, nullptr, true);
            if (build->GetEntry() == GO_WINTERGRASP_VAULT_GATE)
                if (GameObject* go = _wg->GetKeepCollisionWall())
                    go->SetGoState(GO_STATE_ACTIVE);
        }
        build->SetFaction(WintergraspFaction[_teamControl]);
        UpdateGo();
    }
    Save();
}

void WGGameObjectBuilding::Damaged()
{
    // Update worldstate
    _state = WintergraspGameObjectState(BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DAMAGE - (_teamControl * 3));
    _wg->SendUpdateWorldState(_worldState, _state);

    // Send warning message
    if (_isTower)
        _wg->SendWarning(_TextDamaged);

    switch (_GOentry)
    {
        // Attack Towers
        case GO_WINTERGRASP_SHADOWSIGHT_TOWER:
        {
            _wg->HideCreatureByNPCType(WEST_TOWER_GUARD, _teamControl);
            _wg->HideCreatureTurretByNPCType(WEST_TOWER_TURRET);
            break;
        }
        case GO_WINTERGRASP_WINTER_S_EDGE_TOWER:
        {
            _wg->HideCreatureByNPCType(MIDDLE_TOWER_GUARD, _teamControl);
            _wg->HideCreatureTurretByNPCType(MIDDLE_TOWER_TURRET);
            break;
        }
        case GO_WINTERGRASP_FLAMEWATCH_TOWER:
        {
            _wg->HideCreatureByNPCType(EAST_TOWER_GUARD, _teamControl);
            _wg->HideCreatureTurretByNPCType(EAST_TOWER_TURRET);
            break;
        }
        default:
            break;
    }

    if (_type == BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER)
        _wg->UpdateDamagedTowerCount(_wg->GetDefenderTeam());
    else if (_type == BATTLEFIELD_WG_OBJECTTYPE_TOWER)
        _wg->UpdateDamagedTowerCount(_wg->GetAttackerTeam());
    Save();
}

void WGGameObjectBuilding::Destroyed()
{
    // Update worldstate
    _state = WintergraspGameObjectState(BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DESTROY - (_teamControl * 3));
    _wg->SendUpdateWorldState(_worldState, _state);

    // Warn players
    if (_isTower)
        _wg->SendWarning(_TextDestroyed);

    switch (_type)
    {
        // Inform the global wintergrasp script of the destruction of this object
        case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
        case BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER:
            _wg->UpdateDestroyedTowerCount(_teamControl);
            break;
        case BATTLEFIELD_WG_OBJECTTYPE_DOOR_LAST:
            if (GameObject* go = _wg->GetKeepCollisionWall())
                    go->SetGoState(GO_STATE_ACTIVE);
            _wg->SetRelicInteractible(true);
            if (_wg->GetRelic())
                _wg->GetRelic()->RemoveFlag(GO_FLAG_IN_USE | GO_FLAG_NOT_SELECTABLE);
            else
                FMT_LOG_ERROR("bg.battlefield.wg", "Titan Relic not found.");
            break;
        default:
            break;
    }

    _wg->BrokenWallOrTower(_teamControl, this);
    Save();
}

void WGGameObjectBuilding::Init(GameObject* go)
{
    ASSERT_NOTNULL(go);
    // GameObject associated to object
    _GOGUID = go->GetGUID();
    switch (_type)
    {
        case BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER:
        case BATTLEFIELD_WG_OBJECTTYPE_DOOR_LAST:
        case BATTLEFIELD_WG_OBJECTTYPE_DOOR:
        case BATTLEFIELD_WG_OBJECTTYPE_WALL:
            _teamControl = _wg->GetDefenderTeam();           // Objects that are part of the keep should be the defender's
            break;
        case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
            _teamControl = _wg->GetAttackerTeam();           // The towers in the south should be the attacker's
            break;
        default:
            _teamControl = TEAM_NEUTRAL;
            break;
    }

    switch (_state)
    {
        case BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_INTACT:
        case BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_INTACT:
        case BATTLEFIELD_WG_OBJECTSTATE_HORDE_INTACT:
            go->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING, nullptr, true);
            break;
        case BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_DESTROY:
        case BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DESTROY:
        case BATTLEFIELD_WG_OBJECTSTATE_HORDE_DESTROY:
            go->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
            break;
        case BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_DAMAGE:
        case BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DAMAGE:
        case BATTLEFIELD_WG_OBJECTSTATE_HORDE_DAMAGE:
            go->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            break;
        default:
            break;
    }

    _GOentry = go->GetEntry();
    switch (_GOentry)
    {
        case GO_WINTERGRASP_FORTRESS_TOWER_NW:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_FORTRESS_NW;
            break;
        case GO_WINTERGRASP_FORTRESS_TOWER_SW:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_FORTRESS_SW;
            break;
        case GO_WINTERGRASP_FORTRESS_TOWER_SE:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_FORTRESS_SE;
            break;
        case GO_WINTERGRASP_FORTRESS_TOWER_NE:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_FORTRESS_NE;
            break;
        case GO_WINTERGRASP_SHADOWSIGHT_TOWER:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_SHADOWSIGHT;
            break;
        case GO_WINTERGRASP_WINTER_S_EDGE_TOWER:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_WINTER_S_EDGE;
            break;
        case GO_WINTERGRASP_FLAMEWATCH_TOWER:
            _isTower = true;
            _TowerId = BATTLEFIELD_WG_TOWER_FLAMEWATCH;
            break;
    }

    if (_isTower)
    {
        if (IsAttackTower())
        {
            // Spawn associate gameobjects
            for (WintergraspGameObjectData const& gobData : AttackTowers[_TowerId - 4].GameObject)
            {
                if (GameObject* goHorde = ASSERT_NOTNULL(_wg->SpawnGameObject(gobData.HordeEntry, gobData.Pos, gobData.Rot)))
                    m_GameObjectList[TEAM_HORDE].push_back(goHorde->GetGUID());

                if (GameObject* goAlliance = ASSERT_NOTNULL(_wg->SpawnGameObject(gobData.AllianceEntry, gobData.Pos, gobData.Rot)))
                    m_GameObjectList[TEAM_ALLIANCE].push_back(goAlliance->GetGUID());
            }
        }
        else
        {
            // Spawn associate gameobjects
            for (WintergraspGameObjectData const& gobData : DefenseTowers[_TowerId].GameObject)
            {
                if (GameObject* goHorde = ASSERT_NOTNULL(_wg->SpawnGameObject(gobData.HordeEntry, gobData.Pos, gobData.Rot)))
                    m_GameObjectList[TEAM_HORDE].push_back(goHorde->GetGUID());

                if (GameObject* goAlliance = ASSERT_NOTNULL(_wg->SpawnGameObject(gobData.AllianceEntry, gobData.Pos, gobData.Rot)))
                    m_GameObjectList[TEAM_ALLIANCE].push_back(goAlliance->GetGUID());
            }
        }        
    }
    UpdateGo();
}

void WGGameObjectBuilding::UpdateGo()
{
    // Show GameObjects for TeamControll
    if (!m_GameObjectList[_teamControl].empty())
        for (ObjectGuid guid : m_GameObjectList[_teamControl])
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
        if (!m_GameObjectList[otherTeam].empty())
            for (ObjectGuid guid : m_GameObjectList[otherTeam])
                if (GameObject* go = _wg->GetGameObject(guid))
                    go->SetRespawnTime(RESPAWN_ONE_DAY);
    }
    else
    {
        // Need to hide both GOs for alliance and horde
        if (!m_GameObjectList[TEAM_ALLIANCE].empty())
            for (ObjectGuid guid : m_GameObjectList[TEAM_ALLIANCE])
                if (GameObject* go = _wg->GetGameObject(guid))
                    go->SetRespawnTime(RESPAWN_ONE_DAY);

        if (!m_GameObjectList[TEAM_HORDE].empty())
            for (ObjectGuid guid : m_GameObjectList[TEAM_HORDE])
                if (GameObject* go = _wg->GetGameObject(guid))
                    go->SetRespawnTime(RESPAWN_ONE_DAY);
    }
}

void WGGameObjectBuilding::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(_worldState, _state);
}

void WGGameObjectBuilding::Save()
{
    sWorld->setWorldState(_worldState, _state);
}

void WGGameObjectBuilding::CleanBeforeDelete()
{
    for (int8 i = 0; i < 3; i++)
    {
        for (ObjectGuid guid : m_GameObjectList[i])
            if (GameObject* go = _wg->GetGameObject(guid))
                go->DespawnOrUnsummon();
        m_GameObjectList[i].clear();
    }
}
