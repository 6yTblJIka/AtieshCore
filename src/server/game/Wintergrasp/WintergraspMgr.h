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

#ifndef WINTERGRASP_MGR_H_
#define WINTERGRASP_MGR_H_

#include "ZoneScript.h"
#include <unordered_map>
//#include "WGShared.h"
#include "WorldPacket.h"
#include "WorldStatePackets.h"
#include "ObjectGuid.h"

enum BattlefieldTypes
{
    BATTLEFIELD_WG = 1, // Wintergrasp
    BATTLEFIELD_MAX
};

enum BattlefieldTimers
{
    BATTLEFIELD_OBJECTIVE_UPDATE_INTERVAL = 1000
};

enum WintergraspSpells
{
    // Wartime auras
    SPELL_RECRUIT                               = 37795,
    SPELL_CORPORAL                              = 33280,
    SPELL_LIEUTENANT                            = 55629,
    SPELL_TENACITY                              = 58549,
    SPELL_TENACITY_VEHICLE                      = 59911,
    SPELL_TOWER_CONTROL                         = 62064,
    SPELL_SPIRITUAL_IMMUNITY                    = 58729,
    SPELL_GREAT_HONOR                           = 58555,
    SPELL_GREATER_HONOR                         = 58556,
    SPELL_GREATEST_HONOR                        = 58557,
    SPELL_ALLIANCE_FLAG                         = 14268,
    SPELL_HORDE_FLAG                            = 14267,
    SPELL_GRAB_PASSENGER                        = 61178,

    // Reward spells
    SPELL_VICTORY_REWARD                        = 56902,
    SPELL_DEFEAT_REWARD                         = 58494,
    SPELL_DAMAGED_TOWER                         = 59135,
    SPELL_DESTROYED_TOWER                       = 59136,
    SPELL_DAMAGED_BUILDING                      = 59201,
    SPELL_INTACT_BUILDING                       = 59203,

    SPELL_TELEPORT_BRIDGE                       = 59096,
    SPELL_TELEPORT_FORTRESS                     = 60035,

    SPELL_TELEPORT_DALARAN                      = 53360,
    SPELL_VICTORY_AURA                          = 60044,

    // Other spells
    SPELL_WINTERGRASP_WATER                     = 36444,
    SPELL_ESSENCE_OF_WINTERGRASP                = 58045,
    SPELL_WINTERGRASP_RESTRICTED_FLIGHT_AREA    = 58730,

    // Phasing spells
    SPELL_HORDE_CONTROLS_FACTORY_PHASE_SHIFT    = 56618, // ADDS PHASE 16
    SPELL_ALLIANCE_CONTROLS_FACTORY_PHASE_SHIFT = 56617, // ADDS PHASE 32

    SPELL_HORDE_CONTROL_PHASE_SHIFT             = 55773, // ADDS PHASE 64
    SPELL_ALLIANCE_CONTROL_PHASE_SHIFT          = 55774  // ADDS PHASE 128
};

enum BattlefieldSounds
{
    BF_SOUND_HORDE_WINS    = 8454,
    BF_SOUND_ALLIANCE_WINS = 8455,
    BF_SOUND_START         = 3439
};

enum WintergraspGameObject : uint32
{
    // Walls
    GO_WINTERGRASP_WALL_1 = 190219,
    GO_WINTERGRASP_WALL_2 = 190220,
    GO_WINTERGRASP_WALL_3 = 191795,
    GO_WINTERGRASP_WALL_4 = 191796,
    GO_WINTERGRASP_WALL_5 = 191799,
    GO_WINTERGRASP_WALL_6 = 191800,
    GO_WINTERGRASP_WALL_7 = 191801,
    GO_WINTERGRASP_WALL_8 = 191802,
    GO_WINTERGRASP_WALL_9 = 191803,
    GO_WINTERGRASP_WALL_10 = 191804,
    GO_WINTERGRASP_WALL_11 = 191806,
    GO_WINTERGRASP_WALL_12 = 191807,
    GO_WINTERGRASP_WALL_13 = 191808,
    GO_WINTERGRASP_WALL_14 = 191809,
    GO_WINTERGRASP_WALL_15 = 190369,
    GO_WINTERGRASP_WALL_16 = 190370,
    GO_WINTERGRASP_WALL_17 = 190371,
    GO_WINTERGRASP_WALL_18 = 190372,
    GO_WINTERGRASP_WALL_19 = 190374,
    GO_WINTERGRASP_WALL_20 = 190376,
    GO_WINTERGRASP_WALL_21 = 191797,
    GO_WINTERGRASP_WALL_22 = 191798,
    GO_WINTERGRASP_WALL_23 = 191805,

    // Tower of keep (spawned in db)
    GO_WINTERGRASP_FORTRESS_TOWER_NW = 190221,
    GO_WINTERGRASP_FORTRESS_TOWER_SW = 190373,
    GO_WINTERGRASP_FORTRESS_TOWER_SE = 190377,
    GO_WINTERGRASP_FORTRESS_TOWER_NE = 190378,    

    // South tower (spawned in db)
    GO_WINTERGRASP_SHADOWSIGHT_TOWER             = 190356, // W
    GO_WINTERGRASP_WINTER_S_EDGE_TOWER           = 190357, // S
    GO_WINTERGRASP_FLAMEWATCH_TOWER              = 190358, // E

    // Workshop banners (spawned in db)
    GO_WINTERGRASP_FACTORY_BANNER_NE             = 190475,
    GO_WINTERGRASP_FACTORY_BANNER_NW             = 190487,
    GO_WINTERGRASP_FACTORY_BANNER_SE             = 194959,
    GO_WINTERGRASP_FACTORY_BANNER_SW             = 194962,

    // TitanRelic GO (not spawned in db)
    GO_WINTERGRASP_TITAN_S_RELIC                 = 192829,

    // Misc GO (spawned in db)
    GO_WINTERGRASP_FORTRESS_GATE                 = 190375,
    GO_WINTERGRASP_VAULT_GATE                    = 191810,
    GO_WINTERGRASP_KEEP_COLLISION_WALL           = 194323
};

enum WGGossipText
{
    BATTLEFIELD_WG_GOSSIPTEXT_GY_SE              = 20074,
    BATTLEFIELD_WG_GOSSIPTEXT_GY_SW              = 20073,
    BATTLEFIELD_WG_GOSSIPTEXT_GY_NE              = 20071,
    BATTLEFIELD_WG_GOSSIPTEXT_GY_NW              = 20072,
    BATTLEFIELD_WG_GOSSIPTEXT_GY_KEEP            = 20070,
    BATTLEFIELD_WG_GOSSIPTEXT_GY_HORDE           = 20075,
    BATTLEFIELD_WG_GOSSIPTEXT_GY_ALLIANCE        = 20076
};

enum WintergraspAchievements
{
    ACHIEVEMENTS_WIN_WG                          = 1717,
    ACHIEVEMENTS_WIN_WG_100                      = 1718, /// @todo: Has to be implemented
    ACHIEVEMENTS_WG_GNOMESLAUGHTER               = 1723, /// @todo: Has to be implemented
    ACHIEVEMENTS_WG_TOWER_DESTROY                = 1727,
    ACHIEVEMENTS_DESTRUCTION_DERBY_A             = 1737, /// @todo: Has to be implemented
    ACHIEVEMENTS_WG_TOWER_CANNON_KILL            = 1751, /// @todo: Has to be implemented
    ACHIEVEMENTS_WG_MASTER_A                     = 1752, /// @todo: Has to be implemented
    ACHIEVEMENTS_WIN_WG_TIMER_10                 = 1755,
    ACHIEVEMENTS_STONE_KEEPER_50                 = 2085, /// @todo: Has to be implemented
    ACHIEVEMENTS_STONE_KEEPER_100                = 2086, /// @todo: Has to be implemented
    ACHIEVEMENTS_STONE_KEEPER_250                = 2087, /// @todo: Has to be implemented
    ACHIEVEMENTS_STONE_KEEPER_500                = 2088, /// @todo: Has to be implemented
    ACHIEVEMENTS_STONE_KEEPER_1000               = 2089, /// @todo: Has to be implemented
    ACHIEVEMENTS_WG_RANGER                       = 2199, /// @todo: Has to be implemented
    ACHIEVEMENTS_DESTRUCTION_DERBY_H             = 2476, /// @todo: Has to be implemented
    ACHIEVEMENTS_WG_MASTER_H                     = 2776  /// @todo: Has to be implemented
};

enum WintergraspNpcs
{
    NPC_SPIRIT_HEALER                               = 6491,

    BATTLEFIELD_WG_NPC_GUARD_H                      = 30739,
    BATTLEFIELD_WG_NPC_GUARD_A                      = 30740,
    BATTLEFIELD_WG_NPC_STALKER                      = 15214,

    NPC_TAUNKA_SPIRIT_GUIDE                         = 31841, // Horde spirit guide for Wintergrasp
    NPC_DWARVEN_SPIRIT_GUIDE                        = 31842, // Alliance spirit guide for Wintergrasp

    NPC_WORKSHOP_MECHANIC_ALLIANCE                  = 30499, // Alliance mechanic for Wintergrasp
    NPC_WORKSHOP_MECHANIC_HORDE                     = 30400, // Horde mechanic for Wintergrasp

    NPC_WINTERGRASP_SIEGE_ENGINE_ALLIANCE           = 28312,
    NPC_WINTERGRASP_SIEGE_ENGINE_HORDE              = 32627,
    NPC_WINTERGRASP_CATAPULT                        = 27881,
    NPC_WINTERGRASP_DEMOLISHER                      = 28094,
    NPC_WINTERGRASP_TOWER_CANNON                    = 28366
};

enum WintergraspQuests
{
    QUEST_VICTORY_WINTERGRASP_A   = 13181,
    QUEST_VICTORY_WINTERGRASP_H   = 13183,
    QUEST_CREDIT_TOWERS_DESTROYED = 35074,
    QUEST_CREDIT_DEFEND_SIEGE     = 31284
};

enum WintergraspText : uint8
{
    // Invisible Stalker
    BATTLEFIELD_WG_TEXT_SOUTHERN_TOWER_DAMAGE           = 1,
    BATTLEFIELD_WG_TEXT_SOUTHERN_TOWER_DESTROY          = 2,
    BATTLEFIELD_WG_TEXT_EASTERN_TOWER_DAMAGE            = 3,
    BATTLEFIELD_WG_TEXT_EASTERN_TOWER_DESTROY           = 4,
    BATTLEFIELD_WG_TEXT_WESTERN_TOWER_DAMAGE            = 5,
    BATTLEFIELD_WG_TEXT_WESTERN_TOWER_DESTROY           = 6,
    BATTLEFIELD_WG_TEXT_NW_KEEPTOWER_DAMAGE             = 7,
    BATTLEFIELD_WG_TEXT_NW_KEEPTOWER_DESTROY            = 8,
    BATTLEFIELD_WG_TEXT_SE_KEEPTOWER_DAMAGE             = 9,
    BATTLEFIELD_WG_TEXT_SE_KEEPTOWER_DESTROY            = 10,
    BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_ATTACK_ALLIANCE   = 11,
    BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_CAPTURE_ALLIANCE  = 12,
    BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_ATTACK_HORDE      = 13,
    BATTLEFIELD_WG_TEXT_BROKEN_TEMPLE_CAPTURE_HORDE     = 14,
    BATTLEFIELD_WG_TEXT_EASTSPARK_ATTACK_ALLIANCE       = 15,
    BATTLEFIELD_WG_TEXT_EASTSPARK_CAPTURE_ALLIANCE      = 16,
    BATTLEFIELD_WG_TEXT_EASTSPARK_ATTACK_HORDE          = 17,
    BATTLEFIELD_WG_TEXT_EASTSPARK_CAPTURE_HORDE         = 18,
    BATTLEFIELD_WG_TEXT_SUNKEN_RING_ATTACK_ALLIANCE     = 19,
    BATTLEFIELD_WG_TEXT_SUNKEN_RING_CAPTURE_ALLIANCE    = 20,
    BATTLEFIELD_WG_TEXT_SUNKEN_RING_ATTACK_HORDE        = 21,
    BATTLEFIELD_WG_TEXT_SUNKEN_RING_CAPTURE_HORDE       = 22,
    BATTLEFIELD_WG_TEXT_WESTSPARK_ATTACK_ALLIANCE       = 23,
    BATTLEFIELD_WG_TEXT_WESTSPARK_CAPTURE_ALLIANCE      = 24,
    BATTLEFIELD_WG_TEXT_WESTSPARK_ATTACK_HORDE          = 25,
    BATTLEFIELD_WG_TEXT_WESTSPARK_CAPTURE_HORDE         = 26,

    BATTLEFIELD_WG_TEXT_START_GROUPING                  = 27,
    BATTLEFIELD_WG_TEXT_START_BATTLE                    = 28,
    BATTLEFIELD_WG_TEXT_FORTRESS_DEFEND_ALLIANCE        = 29,
    BATTLEFIELD_WG_TEXT_FORTRESS_CAPTURE_ALLIANCE       = 30,
    BATTLEFIELD_WG_TEXT_FORTRESS_DEFEND_HORDE           = 31,
    BATTLEFIELD_WG_TEXT_FORTRESS_CAPTURE_HORDE          = 32,

    BATTLEFIELD_WG_TEXT_NE_KEEPTOWER_DAMAGE             = 33,
    BATTLEFIELD_WG_TEXT_NE_KEEPTOWER_DESTROY            = 34,
    BATTLEFIELD_WG_TEXT_SW_KEEPTOWER_DAMAGE             = 35,
    BATTLEFIELD_WG_TEXT_SW_KEEPTOWER_DESTROY            = 36,

    BATTLEFIELD_WG_TEXT_RANK_CORPORAL                   = 37,
    BATTLEFIELD_WG_TEXT_RANK_FIRST_LIEUTENANT           = 38
};

enum NPCtype
{
    // Attacker sector
    WEST_TOWER_GUARD,
    MIDDLE_TOWER_GUARD,
    EAST_TOWER_GUARD,

    WEST_BRIDGE_GUARD,
    MIDDLE_BRIDGE_GUARD,
    EAST_BRIDGE_GUARD,

    WEST_TOWER_TURRET,
    MIDDLE_TOWER_TURRET,
    EAST_TOWER_TURRET,

    // Defender sector
    NE_TOWER_GUARD,
    NW_TOWER_GUARD,
    SE_TOWER_GUARD,
    SW_TOWER_GUARD,
    VAULT_KEEP,

    FORTRESS_GATE_TURRET,
    VAULT_GATE_TURRET,
    NE_TOWER_TURRET,
    NW_TOWER_TURRET,
    SE_TOWER_TURRET,
    SW_TOWER_TURRET,

    //workshop sector
    WG_WORKSHOP_SE,
    WG_WORKSHOP_SW,
    WG_WORKSHOP_NE,
    WG_WORKSHOP_NW,
    WG_WORKSHOP_KEEP_WEST,
    WG_WORKSHOP_KEEP_EAST,

    WG_ALLIANCE_GRAVEYARD,
    WG_HORDE_GRAVEYARD
};

enum BattlefieldObjectiveStates
{
    BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL = 0,
    BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE,
    BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE
};

enum WintergraspData
{
    BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF,
    BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF,
    BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT,
    BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT,
    BATTLEFIELD_WG_DATA_MAX_VEHICLE_A,
    BATTLEFIELD_WG_DATA_MAX_VEHICLE_H,
    BATTLEFIELD_WG_DATA_VEHICLE_A,
    BATTLEFIELD_WG_DATA_VEHICLE_H,
    BATTLEFIELD_WG_DATA_WON_A,
    BATTLEFIELD_WG_DATA_DEF_A,
    BATTLEFIELD_WG_DATA_WON_H,
    BATTLEFIELD_WG_DATA_DEF_H,
    BATTLEFIELD_WG_DATA_MAX,

    BATTLEFIELD_WG_MAPID = 571               // Northrend
};

enum WGGraveyardId
{
    BATTLEFIELD_WG_GY_WORKSHOP_SE,
    BATTLEFIELD_WG_GY_WORKSHOP_SW,
    BATTLEFIELD_WG_GY_WORKSHOP_NE,
    BATTLEFIELD_WG_GY_WORKSHOP_NW,
    BATTLEFIELD_WG_GY_KEEP,
    BATTLEFIELD_WG_GY_HORDE,
    BATTLEFIELD_WG_GY_ALLIANCE,
    BATTLEFIELD_WG_GRAVEYARD_MAX
};

enum WintergraspWorkshopIds : uint8
{
    BATTLEFIELD_WG_WORKSHOP_SE,
    BATTLEFIELD_WG_WORKSHOP_SW,
    BATTLEFIELD_WG_WORKSHOP_NE,
    BATTLEFIELD_WG_WORKSHOP_NW,
    BATTLEFIELD_WG_WORKSHOP_KEEP_WEST,
    BATTLEFIELD_WG_WORKSHOP_KEEP_EAST,
    BATTLEFIELD_WG_WORKSHOP_MAX
};

enum WintergraspTowerIds
{
    BATTLEFIELD_WG_TOWER_FORTRESS_NW,
    BATTLEFIELD_WG_TOWER_FORTRESS_SW,
    BATTLEFIELD_WG_TOWER_FORTRESS_SE,
    BATTLEFIELD_WG_TOWER_FORTRESS_NE,
    BATTLEFIELD_WG_TOWER_SHADOWSIGHT,
    BATTLEFIELD_WG_TOWER_WINTER_S_EDGE,
    BATTLEFIELD_WG_TOWER_FLAMEWATCH
};

enum WintergraspGameObjectBuildingType
{
    BATTLEFIELD_WG_OBJECTTYPE_DOOR,
    BATTLEFIELD_WG_OBJECTTYPE_TITANRELIC,
    BATTLEFIELD_WG_OBJECTTYPE_WALL,
    BATTLEFIELD_WG_OBJECTTYPE_DOOR_LAST,
    BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER,
    BATTLEFIELD_WG_OBJECTTYPE_TOWER
};

enum WintergraspGameObjectState
{
    BATTLEFIELD_WG_OBJECTSTATE_NONE,
    BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_INTACT,
    BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_DAMAGE,
    BATTLEFIELD_WG_OBJECTSTATE_NEUTRAL_DESTROY,
    BATTLEFIELD_WG_OBJECTSTATE_HORDE_INTACT,
    BATTLEFIELD_WG_OBJECTSTATE_HORDE_DAMAGE,
    BATTLEFIELD_WG_OBJECTSTATE_HORDE_DESTROY,
    BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_INTACT,
    BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DAMAGE,
    BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DESTROY
};

enum WintergraspTeamControl
{
    BATTLEFIELD_WG_TEAM_ALLIANCE,
    BATTLEFIELD_WG_TEAM_HORDE,
    BATTLEFIELD_WG_TEAM_NEUTRAL
};

// *****************************************************
// ************ Destructible (Wall, Tower..) ***********
// *****************************************************
struct WintergraspBuildingSpawnData
{
    uint32 entry;
    uint32 WorldState;
    Position pos;
    QuaternionData rot;
    WintergraspGameObjectBuildingType type;
};

uint8 const WG_MAX_OBJ = 32;
WintergraspBuildingSpawnData const WGGameObjectBuildings[WG_MAX_OBJ] =
{
    // Wall (spawned in db)
    // Entry  WS      X          Y          Z           O                rX   rY   rZ             rW             Type
    { 190219, 3749, { 5371.457f, 3047.472f, 407.5710f,  3.14159300f }, { 0.f, 0.f, -1.000000000f, 0.00000000f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190220, 3750, { 5331.264f, 3047.105f, 407.9228f,  0.05235888f }, { 0.f, 0.f,  0.026176450f, 0.99965730f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191795, 3764, { 5385.841f, 2909.490f, 409.7127f,  0.00872424f }, { 0.f, 0.f,  0.004362106f, 0.99999050f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191796, 3772, { 5384.452f, 2771.835f, 410.2704f,  3.14159300f }, { 0.f, 0.f, -1.000000000f, 0.00000000f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191799, 3762, { 5371.436f, 2630.610f, 408.8163f,  3.13285800f }, { 0.f, 0.f,  0.999990500f, 0.00436732f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191800, 3766, { 5301.838f, 2909.089f, 409.8661f,  0.00872424f }, { 0.f, 0.f,  0.004362106f, 0.99999050f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191801, 3770, { 5301.063f, 2771.411f, 409.9014f,  3.14159300f }, { 0.f, 0.f, -1.000000000f, 0.00000000f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191802, 3751, { 5280.197f, 2995.583f, 408.8249f,  1.61442800f }, { 0.f, 0.f,  0.722363500f, 0.69151360f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191803, 3752, { 5279.136f, 2956.023f, 408.6041f,  1.57079600f }, { 0.f, 0.f,  0.707106600f, 0.70710690f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191804, 3767, { 5278.685f, 2882.513f, 409.5388f,  1.57079600f }, { 0.f, 0.f,  0.707106600f, 0.70710690f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191806, 3769, { 5279.502f, 2798.945f, 409.9983f,  1.57079600f }, { 0.f, 0.f,  0.707106600f, 0.70710690f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191807, 3759, { 5279.937f, 2724.766f, 409.9452f,  1.56207000f }, { 0.f, 0.f,  0.704014800f, 0.71018530f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191808, 3760, { 5279.601f, 2683.786f, 409.8488f,  1.55334100f }, { 0.f, 0.f,  0.700908700f, 0.71325110f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191809, 3761, { 5330.955f, 2630.777f, 409.2826f,  3.13285800f }, { 0.f, 0.f,  0.999990500f, 0.00436732f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190369, 3753, { 5256.085f, 2933.963f, 409.3571f,  3.13285800f }, { 0.f, 0.f,  0.999990500f, 0.00436732f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190370, 3758, { 5257.463f, 2747.327f, 409.7427f, -3.13285800f }, { 0.f, 0.f, -0.999990500f, 0.00436732f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190371, 3754, { 5214.960f, 2934.089f, 409.1905f, -0.00872424f }, { 0.f, 0.f, -0.004362106f, 0.99999050f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190372, 3757, { 5215.821f, 2747.566f, 409.1884f, -3.13285800f }, { 0.f, 0.f, -0.999990500f, 0.00436732f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190374, 3755, { 5162.273f, 2883.043f, 410.2556f,  1.57952200f }, { 0.f, 0.f,  0.710185100f, 0.70401500f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 190376, 3756, { 5163.724f, 2799.838f, 409.2270f,  1.57952200f }, { 0.f, 0.f,  0.710185100f, 0.70401500f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191797, 3765, { 5343.290f, 2908.860f, 409.5757f, 0.00872424f }, { 0.f, 0.f,  0.004362106f, 0.9999905f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191798, 3771, { 5342.719f, 2771.386f, 409.6249f, 3.14159300f }, { 0.f, 0.f, -1.000000000f, 0.0000000f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },
    { 191805, 3768, { 5279.126f, 2840.797f, 409.7826f, 1.57952200f }, { 0.f, 0.f,  0.710185100f, 0.7040150f }, BATTLEFIELD_WG_OBJECTTYPE_WALL },

    // Tower of keep (spawned in db)
    { 190221, 3711, { 5281.154f, 3044.588f, 407.8434f,  3.115388f }, { 0.f, 0.f,  0.9999142f, 0.013101960f }, BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER }, // NW
    { 190373, 3713, { 5163.757f, 2932.228f, 409.1904f,  3.124123f }, { 0.f, 0.f,  0.9999619f, 0.008734641f }, BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER }, // SW
    { 190377, 3714, { 5166.397f, 2748.368f, 409.1884f, -1.570796f }, { 0.f, 0.f, -0.7071066f, 0.707106900f }, BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER }, // SE
    { 190378, 3712, { 5281.192f, 2632.479f, 409.0985f, -1.588246f }, { 0.f, 0.f, -0.7132492f, 0.700910500f }, BATTLEFIELD_WG_OBJECTTYPE_KEEP_TOWER }, // NE

    // South tower (spawned in db)
    { 190356, 3704, { 4557.173f, 3623.943f, 395.8828f,  1.675516f }, { 0.f, 0.f,  0.7431450f, 0.669130400f }, BATTLEFIELD_WG_OBJECTTYPE_TOWER }, // W
    { 190357, 3705, { 4398.172f, 2822.497f, 405.6270f, -3.124123f }, { 0.f, 0.f, -0.9999619f, 0.008734641f }, BATTLEFIELD_WG_OBJECTTYPE_TOWER }, // S
    { 190358, 3706, { 4459.105f, 1944.326f, 434.9912f, -2.002762f }, { 0.f, 0.f, -0.8422165f, 0.539139500f }, BATTLEFIELD_WG_OBJECTTYPE_TOWER }, // E

    // Door of forteress (spawned in db)
    { GO_WINTERGRASP_FORTRESS_GATE, 3763, { 5162.991f, 2841.232f, 410.1892f, -3.132858f }, { 0.f, 0.f, -0.9999905f, 0.00436732f }, BATTLEFIELD_WG_OBJECTTYPE_DOOR },

    // Last door (pawned in db)
    { GO_WINTERGRASP_VAULT_GATE, 3773, { 5397.108f, 2841.54f, 425.9014f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, BATTLEFIELD_WG_OBJECTTYPE_DOOR_LAST },
};

uint32 const ClockWorldState[] = { 3781, 4354 };
uint32 const WintergraspFaction[] = { FACTION_ALLIANCE_GENERIC_WG, FACTION_HORDE_GENERIC_WG, FACTION_FRIENDLY };
//TEAM_ALLIANCE/TEAM_HORDE
uint32 const NPCGuardFaction[] = { 1891, 1979 };
//TEAM_ALLIANCE/TEAM_HORDE
uint32 const NPCMechanicFaction[] = { 1732, 1735 };

Position const WintergraspStalkerPos = { 4948.985f, 2937.789f, 550.5172f,  1.815142f };

Position const WintergraspRelicPos = { 5440.379f, 2840.493f, 430.2816f, -1.832595f };
QuaternionData const WintergraspRelicRot = { 0.f, 0.f, -0.7933531f, 0.6087617f };

uint8 const WG_MAX_TELEPORTER = 12;
uint8 const WG_MAX_TOWER = 7;

struct WintergraspObjectPositionData
{
    Position Pos;
    uint32 HordeEntry;
    uint32 AllianceEntry;
};

struct WintergraspGameObjectData
{
    Position Pos;
    QuaternionData Rot;
    uint32 HordeEntry;
    uint32 AllianceEntry;
};

struct WintergraspGameObjectWorkshopData
{
    Position Pos;
    QuaternionData Rot;
    uint32 entry;
};

uint8 const WG_MAX_WORKSHOPGO = 4;
WintergraspGameObjectWorkshopData const WGworkshopData[WG_MAX_WORKSHOPGO] =
{
    // workshop GO
    // SE
    { { 4398.08f, 2356.5f, 376.19f, 0.525406f }, { 0.f, 0.f, 0.259692f, 0.965692f }, 194959 },
    // SW
    { { 4390.78f, 3304.09f, 372.429f, 6.09702f }, { 0.f, 0.f, 0.0929482f, -0.995671f }, 194962 },
    // NE
    { { 4949.34f, 2432.59f, 320.177f, 1.38621f }, { 0.f, 0.f, 0.638929f, 0.769266f }, 192626 },
    // NW
    { { 4948.52f, 3342.34f, 376.875f, 4.40057f }, { 0.f, 0.f, 0.808329f, -0.588731f }, 192627 }
};

struct WintergraspGameObjectKeepData
{
    Position Pos;
    QuaternionData Rot;
    uint32 entry;
};
uint8 const WG_MAX_KEEPGO_HORDE = 11;
WintergraspGameObjectKeepData const WGKeepHordeData[WG_MAX_KEEPGO_HORDE] =
{
    { { 5371.9f, 2805.48f, 409.307f, 0.0610866f }, { 0.f, 0.f, 0.0305386f, 0.999534f }, 193160 },   // GO east keep vault
    { { 5376.62f, 2875.1f, 409.255f, 1.59698f }, { 0.f, 0.f, 0.716303f, 0.697789f }, 193161 },      // GO west keep vault
    // horde hoodo master
    { { 5294.6f, 2786.85f, 409.888f, -2.35619f }, { 0.f, 0.f, -0.923879f, 0.382686f }, 193139 },
    { { 5294.24f, 2786.42f, 409.174f, 0.0174525f }, { 0.f, 0.f, 0.00872614f, 0.999962f }, 193142 },
    { { 5295.87f, 2787.77f, 409.192f, 2.15548f }, { 0.f, 0.f, 0.88089f, 0.47332f }, 193145 },
    { { 5293.21f, 2787.03f, 409.219f, 3.03687f }, { 0.f, 0.f, 0.998629f, 0.0523374f }, 193141 },
    { { 5293.56f, 2787.31f, 410.56f, 0.261798f }, { 0.f, 0.f, 0.130525f, 0.991445f }, 193146 },
    { { 5294.38f, 2785.04f, 409.175f, -2.33874f }, { 0.f, 0.f, -0.920505f, 0.390732f }, 193140 },
    { { 5293.03f, 2785.63f, 409.225f, -1.29154f }, { 0.f, 0.f, -0.601814f, 0.798637f }, 193144 },
    { { 5291.71f, 2785.87f, 409.282f, -2.03331f }, { 0.f, 0.f, -0.850353f, 0.526213f }, 193143 },
    // the westest - blacksmith forge
    { { 5293.35f, 2923.71f, 409.845f, -1.87623f }, { 0.f, 0.f, -0.806445f, 0.591309f }, 193127 }
};

uint8 const WG_MAX_KEEPGO_ALLIANCE = 21;
WintergraspGameObjectKeepData const WGKeepAllianceData[WG_MAX_KEEPGO_ALLIANCE] =
{
    { { 5378.92f, 2805.44f, 409.239f, 1.53589f }, { 0.f, 0.f, 0.694658f, 0.71934f }, 193164 },     // GO east keep
    { { 5378.07f, 2813.62f, 409.239f, 1.55334f }, { 0.f, 0.f, 0.700908f, 0.713252f }, 193163 },    // GO east keep
    { { 5378.45f, 2876.67f, 409.239f, 1.54462f }, { 0.f, 0.f, 0.697792f, 0.716301f }, 193165 },    // GO west keep
    { { 5377.55f, 2870.92f, 409.239f, -0.549777f }, { 0.f, 0.f, -0.27144f, 0.962455f }, 193162 },  // GO west keep
    // alliance kaylana
    { { 5294.09f, 2894.19f, 409.164f, -0.733038f }, { 0.f, 0.f, -0.358368f, 0.93358f }, 193130 },
    { { 5293.74f, 2894.48f, 409.183f, -2.72271f }, { 0.f, 0.f, -0.978147f, 0.207914f }, 193138 },
    { { 5295.19f, 2895.38f, 409.143f, -0.349065f }, { 0.f, 0.f, -0.173648f, 0.984808f }, 193131 },
    { { 5293.12f, 2895.23f, 409.209f, -0.98611f }, { 0.f, 0.f, -0.473319f, 0.880891f }, 193128 },
    { { 5295.39f, 2895.06f, 410.669f, 0.689403f }, { 0.f, 0.f, 0.0167089f, 0.0073185f }, 193133 },
    { { 5294.98f, 2895.34f, 410.658f, -2.73143f }, { 0.f, 0.f, -0.979044f, 0.203647f }, 193135 },
    { { 5295.56f, 2895.24f, 410.628f, -2.11185f }, { 0.f, 0.f, -0.870356f, 0.492423f }, 193137 },
    { { 5294.53f, 2895.57f, 410.659f, -1.92859f }, { 0.0222769f, -0.035924f, -0.82086f, 0.569563f }, 193132 },
    { { 5293.23f, 2895.46f, 410.413f, -0.802851f }, { 0.f, 0.f, -0.390731f, 0.920505f }, 193136 },
    { { 5295.14f, 2895.68f, 410.619f, -2.22529f }, { 0.f, 0.f, -0.896872f, 0.442291f }, 193134 },
    { { 5292.91f, 2895.54f, 410.42f, -0.122171f }, { 0.f, 0.f, -0.0610475f, 0.998135f }, 193129 },
    // the eastest - boweyr
    { { 5293.38f, 2750.9f, 409.235f, -0.0610866f }, { 0.f, 0.f, -0.0305386f, 0.999534f }, 193159 },
    { { 5292.43f, 2748.62f, 409.131f, 0.253072f }, { 0.f, 0.f, 0.126199f, 0.992005f }, 193158 },
    { { 5292.24f, 2753.59f, 409.087f, -0.35779f }, { 0.f, 0.f, -0.177942f, 0.984041f }, 193157 },
    { { 5293.38f, 2746.06f, 409.221f, -0.0698117f }, { 0.f, 0.f, -0.0348988f, 0.999391f }, 193155 },
    { { 5293.65f, 2755.68f, 409.191f, -0.436332f }, { 0.f, 0.f, -0.216439f, 0.976296f }, 193156 },
    // the westest - blacksmith forge
    { { 5293.66f, 2924.44f, 409.293f, 1.20427f }, { 0.f, 0.f, 0.566403f, 0.824128f }, 193124 }
};
class WGGraveyard;
typedef std::unordered_map<uint32, WGGraveyard*> GraveyardMap;
//typedef std::unordered_map<WintergraspWorkshopIds, WGCapturePoint*> CapturePointContainer;

class WGGameObjectBuilding;
typedef std::unordered_map<uint32 /*GOentry*/, WGGameObjectBuilding*> WGGameObjectBuildingMap;

class WGWorkshop;
class WGCapturePoint;
struct WorkshopAndCapturePointPAIR
{
    WGWorkshop* _workshopPoint;
    WGCapturePoint* _capturePoint;
};
typedef std::unordered_map<WintergraspWorkshopIds, WorkshopAndCapturePointPAIR> WorkshopAndCapturePointMap;

struct PlayerHolder
{
    bool inZone;
    bool inQueue;
    bool inWar;
    bool isWaitingQueue;
    bool isWaitingWar;
    bool isWaitingKick;
    TeamId team;
    time_t time;
};
typedef std::unordered_map<ObjectGuid, PlayerHolder> PlayerHolderContainer;

struct CreatureHolder
{
    uint8 m_npcType;
    TeamId m_team;
    bool m_isActive;
};
typedef std::unordered_map<ObjectGuid, CreatureHolder> CreatureHolderContainer;

struct Position;
struct QuaternionData;

class TC_GAME_API WintergraspMgr : public ZoneScript
{
private:
    WintergraspMgr();
    ~WintergraspMgr();
public:
    static WintergraspMgr* instance();

    void InitializeWG();
    void Update(uint32 diff);

    //used by commands
    void StartBattle();
    void EndBattle(bool endByTimer);
    // Enable or Disable battlefield
    void ToggleBattlefield(bool enable) { m_isEnabled = enable; }    
    
    void SetTimer(uint32 timer) { m_Timer = timer; }
    void SetupWG(TeamId defender, bool StartWar);
    
    // Return if battlefield is enable
    bool IsEnabled() const { return m_isEnabled; }
    // Return true if battle is start, false if battle is not started
    bool IsWarTime() const { return m_isActive; }

    uint32 GetData(uint32 dataId) const  override { return m_Data32[dataId]; }
    uint32 GetTimer() const { return m_Timer; }
    uint32 GetFreeslot(TeamId team) const { return m_freeslots[team]; }
    uint32 _GetGraveyardIDByType(WGGraveyardId type);
    WGGraveyard* GetGraveyardById(uint32 id) const;
    WorldSafeLocsDBC const* GetClosestGraveyard(Player* player);
    WGGraveyardId GetSpiritGraveyardIdForCreature(Creature* creature) const;
    GraveyardMap const& GetGraveyardMap() const { return m_graveyardMap; }
    Creature* GetCreature(ObjectGuid spawnid);
    GameObject* GetGameObject(ObjectGuid guid);
    GameObject* GetTowerGObyTowerID(WintergraspTowerIds towerID);
    // Return pointer to relic object
    GameObject* GetRelic() { return GetGameObject(m_TitansRelicGUID); }
    GameObject* GetKeepCollisionWall() { return GetGameObject(m_KeepCollisionWall); }
    WGGameObjectBuilding* GetBuildingByGOEntry(uint32 entry);
    WGCapturePoint* GetCapturePoint(WintergraspWorkshopIds workshop) const
    {
        WorkshopAndCapturePointMap::const_iterator itr = m_workshopAndCaptures.find(workshop);
        if (itr != m_workshopAndCaptures.end())
            return itr->second._capturePoint;
        return nullptr;
    }
    WGWorkshop* GetWGWorkshopByCapturePoint(WGCapturePoint* cp) const
    {
        for (auto const pointer : m_workshopAndCaptures)
            if (pointer.second._capturePoint == cp)
                return pointer.second._workshopPoint;
        return nullptr;
    }

    // Teams
    TeamId GetDefenderTeam() const { return m_DefenderTeam; }
    TeamId GetAttackerTeam() const { return TeamId(1 - m_DefenderTeam); }
    TeamId GetOtherTeam(TeamId team) const { return (team == TEAM_HORDE ? TEAM_ALLIANCE : TEAM_HORDE); }

    bool PlayerInBFPlayerMap(Player* plr);
    void InviteNewPlayerToQueue(Player* player, bool isInZone);
    void InviteNewPlayerToWar(Player* player, bool isInZone);
    void KickPlayerFromBattlefield(ObjectGuid guid);

    void PlayerAcceptInviteToQueue(Player* player);
    void PlayerAcceptInviteToWar(Player* player);
    void PlayerDeclineInviteToQueue(Player* player);
    void PlayerDeclineInviteToWar(Player* player);
    void AskToLeaveQueue(Player* player);
    void PlayerAskToLeave(Player* player);

    // Called when player (player) enter in zone
    void HandlePlayerEnterZone(Player* player, uint32 /*zone*/);
    // Called when player (player) leave the zone
    void HandlePlayerLeaveZone(Player* player, uint32 /*zone*/);
    // Called when a Unit is kill in battlefield zone
    void HandleKill(Player* killer, Unit* killed);
    // Called when a ressurect in battlefield zone
    void HandleResurrectedPlayer(ObjectGuid playerGuid);

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet);
    void SendInitWorldStatesToAll();
    void SendUpdateWorldState(uint32 field, uint32 value);
    void SendAreaSpiritHealerQueryOpcode(Player* player, ObjectGuid source);
    void SendWarning(uint8 id, WorldObject const* target = nullptr);

    Creature* SpawnCreature(uint32 entry, Position const& pos);
    GameObject* SpawnGameObject(uint32 entry, Position const& pos, QuaternionData const& rot);

    void SetRelicInteractible(bool allow) { m_isRelicInteractible = allow; }

    void ShowCreatureByNPCType(uint8 npcType, TeamId team);
    void HideCreatureByNPCType(uint8 npcType, TeamId team);
    void HideCreatureTurretByNPCType(uint8 npcType);

    void UpdateDamagedTowerCount(TeamId team);
    void UpdateDestroyedTowerCount(TeamId team);
    void UpdateCounterVehicle(bool init);
    void BrokenWallOrTower(TeamId team, WGGameObjectBuilding* building);

private:    

    void OnBattleStart();
    void _OnBattleStartPlayers();
    void OnBattleEnd(bool endByTimer);    
    // Define if player can interact with the relic
    void SetRelic(ObjectGuid relicGUID) { m_TitansRelicGUID = relicGUID; }
    void InitStalker(uint32 entry, Position const& pos);
    void SetDefenderTeam(TeamId team) { m_DefenderTeam = team; } 
    void UpdateVehicleCountWG();

    /// <Zone section>
        // All-purpose data storage 32 bit
        void SetData(uint32 dataId, uint32 value) override { m_Data32[dataId] = value; }
        void UpdateData(uint32 index, int32 pad) { m_Data32[index] += pad; }
        void OnCreatureCreate(Creature* creature) override;
        void OnCreatureRemove(Creature* creature) override;
        void OnGameObjectCreate(GameObject* go) override;
        void OnGameObjectRemove(GameObject* go) override;
        void RecheckImportantGORespawn(GameObject* go);
        //helpers
        void OnPlayerLeaveZone(Player* player);
        void OnPlayerEnterZone(Player* player);
    /// </Zone section>

    /// <Player section>
        void AddPlayer(Player* plr, bool InZone, bool IsWaitingQueue, bool IsWaitingWar, bool IsWaitingKick, time_t time = 0);
        void RemovePlayer(Player* plr);
        void UpdateZoneStatusInPlayerMap(Player* plr, bool inZone);
        // Invite all players in zone to join the queue, called x minutes before battle start in Update()
        void InvitePlayersInZoneToQueue();
        // Invite all players in queue to join battle on battle start
        void InvitePlayersInQueueToWar();
        // Invite all players not in queue to join battle on battle start
        void TryInvitePlayersNotInQueueToWarOrKickThem();
        // use for switch off all worldstate for client
        void SendRemoveWorldStates(Player* /*player*/); //UNUSED
        void SendInitWorldStatesTo(Player* player);
        void RemoveAurasFromPlayer(Player* player);
        void UpdateTenacity();        
        void DoCompleteOrIncrementAchievement(uint32 achievement, Player* player, uint8 incrementNumber = 1);
        //helpers
        void OnPlayerJoinWar(Player* player);
        void OnPlayerLeaveWar(Player* player);
        void HandlePromotion(Player* killer, Unit* killed);
        void PromotePlayer(Player* killer);
    /// </Player section>

    /// <Spawn section>
        bool IsCreatureInHolder(ObjectGuid guid);
        bool AddCreatureInHolderByGUID(Creature* creature, uint8 npcType, TeamId team = TEAM_NEUTRAL);
        void UpdateStatusForCreature(Creature* creature, uint8 npcType, TeamId team);
        void UpdateCreatureTurretByNPCType(uint8 npcType, TeamId team);
        void UpdateAllGuardsAndTurretsBeforeBattle();
        void InitAllGOforKeep();
        void UpdateAllGOforKeep();
        void HideNpc(Creature* creature);
        void ShowNpc(Creature* creature, bool aggressive);
        void _UpdateCreatureForBuildGO(WintergraspGameObject GOentry, Creature* creature, TeamId team);
        void _UpdateCreatureForWorkshop(WintergraspWorkshopIds workshopType, Creature* creature, TeamId team);
    /// </Spawn section>

    /// <Group section>
        void OnStartGrouping();
        bool AddOrSetPlayerToCorrectBfGroup(Player* player);
        Group* GetFreeBfRaid(TeamId TeamId);
    /// </Group section>

    WGGameObjectBuildingMap    m_buildingsInZone;
    WorkshopAndCapturePointMap m_workshopAndCaptures;  // Workshop and CapturePoint PAIR
    GraveyardMap               m_graveyardMap;         // Graveyards
    PlayerHolderContainer      m_PlayerMap;
    CreatureHolderContainer    m_CreatureMap;

    //vectors
    GuidVector DefenderPortalList[PVP_TEAMS_COUNT];
    GuidVector m_KeepHordeGameObjectList;
    GuidVector m_KeepAllianceGameObjectList;
    GuidVector m_KeepHordeNPCList;
    GuidVector m_KeepAllianceNPCList;

    //unorderedset
    GuidUnorderedSet m_vehicles[PVP_TEAMS_COUNT];
    GuidUnorderedSet m_Groups[PVP_TEAMS_COUNT];             // Contain different raid group

    Map* m_Map;

    bool m_isEnabled;
    bool m_isActive;
    bool m_isRelicInteractible;
    bool m_StartGrouping;                                   // bool for know if all players in area has been invited

    uint32 m_Timer;                                         // Global timer for event
    uint32 m_MinPlayer;                                     // Minimum number of player for Battlefield start
    uint32 m_MaxPlayer;                                     // Maximum number of player that participated to Battlefield
    uint32 m_freeslots[PVP_TEAMS_COUNT];                    // Count of free slots for new members
    uint32 m_TimeForAcceptInvite;
    uint32 m_tenacityStack;
    uint32 m_MinLevel;                                      // Required level to participate at Battlefield
    uint32 m_BattleTime;                                    // Length of a battle
    uint32 m_NoWarBattleTime;                               // Time between two battles
    uint32 m_StartGroupingTimer;                            // Timer for invite players in area 15 minute before start battle
    uint32 m_saveTimer;
    uint32 m_uiAcceptTimer;
    uint32 m_LastResurrectTimer;                            // Timer for resurrect player every 30 sec

    WorldLocation KickPosition;                             // Position where players are teleported if they switch to afk during the battle or if they don't accept invitation

    TeamId m_DefenderTeam;
    TeamId m_tenacityTeam;

    ObjectGuid m_StalkerGuid;
    ObjectGuid m_TitansRelicGUID;
    ObjectGuid m_KeepCollisionWall;

    WorldSafeLocsDBC const* Grave_NW;
    WorldSafeLocsDBC const* Grave_NE;
    WorldSafeLocsDBC const* Grave_SW;
    WorldSafeLocsDBC const* Grave_SE;
    WorldSafeLocsDBC const* Grave_Keep;
    WorldSafeLocsDBC const* Grave_Horde;
    WorldSafeLocsDBC const* Grave_Alliance;

    std::vector<uint32> m_Data32;
};

#define sWintergraspMgr WintergraspMgr::instance()

#endif // WINTERGRASP_MGR_H_
