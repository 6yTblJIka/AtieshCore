/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "BattlegroundNA.h"
#include "Log.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldStatePackets.h"

BattlegroundGOSpawnPoint const BG_NA_GameObjects[BG_NA_OBJECT_MAX] =
{
    // gates
    { BG_NA_OBJECT_TYPE_DOOR_1, { 4031.854000f, 2966.833000f, 12.646200f, -2.64878800f }, { 0.f, 0.f, 0.96979620f, -0.2439165f }, RESPAWN_IMMEDIATELY },
    { BG_NA_OBJECT_TYPE_DOOR_2, { 4081.179000f, 2874.970000f, 12.391710f,  0.49280450f }, { 0.f, 0.f, 0.24391650f,  0.9697962f }, RESPAWN_IMMEDIATELY },
    { BG_NA_OBJECT_TYPE_DOOR_3, { 4023.709000f, 2981.777000f, 10.701170f, -2.64878800f }, { 0.f, 0.f, 0.96979620f, -0.2439165f }, RESPAWN_IMMEDIATELY },
    { BG_NA_OBJECT_TYPE_DOOR_4, { 4090.064000f, 2858.438000f, 10.236310f,  0.49280450f }, { 0.f, 0.f, 0.24391650f,  0.9697962f }, RESPAWN_IMMEDIATELY },
    // buffs
    { BG_NA_OBJECT_TYPE_BUFF_1, { 4009.189941f, 2895.250000f, 13.052700f, -1.44862400f }, { 0.f, 0.f, 0.66262010f, -0.7489557f }, 2 * MINUTE          },
    { BG_NA_OBJECT_TYPE_BUFF_2, { 4103.330078f, 2946.350098f, 13.051300f, -0.06981307f }, { 0.f, 0.f, 0.03489945f, -0.9993908f }, 2 * MINUTE          }
};

BattlegroundNA::BattlegroundNA()
{
    SetGameObjectsNumber(BG_NA_OBJECT_MAX);
}

void BattlegroundNA::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    _events.Update(diff);

    while (uint32 eventId = _events.ExecuteEvent())
    {
        switch (eventId)
        {
            case BG_NA_EVENT_REMOVE_DOORS:
                for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; ++i)
                    DelObject(i);
                break;
            default:
                break;
        }
    }
}

void BattlegroundNA::StartingEventCloseDoors()
{
    for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_4; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
}

void BattlegroundNA::StartingEventOpenDoors()
{
    for (uint32 i = BG_NA_OBJECT_BUFF_1; i <= BG_NA_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, 60);

    for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; ++i)
        DoorOpen(i);
    _events.ScheduleEvent(BG_NA_EVENT_REMOVE_DOORS, BG_NA_REMOVE_DOORS_TIMER);
}

void BattlegroundNA::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (trigger)
    {
        case 4536:                                          // buff trigger?
        case 4537:                                          // buff trigger?
            break;
        default:
            Battleground::HandleAreaTrigger(player, trigger);
            break;
    }
}

void BattlegroundNA::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(2577, 1); // BATTLEGROUND_NAGRAND_ARENA_SHOW

    Arena::FillInitialWorldStates(packet);
}

bool BattlegroundNA::SetupBattleground()
{
    for (uint32 i = 0; i < BG_NA_OBJECT_MAX; ++i)
    {
        BattlegroundGOSpawnPoint const& object = BG_NA_GameObjects[i];
        if (!AddObject(i, object.Entry, object.Pos, object.Rot, object.SpawnTime))
        {
            FMT_LOG_ERROR("bg.battleground", "BattleGroundNA: Failed to spawn GameObject! (Entry: {}). Battleground not created!", object.Entry);
            return false;
        }
    }

    return true;
}
