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

#include "BattlegroundRL.h"
#include "Log.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldStatePackets.h"

BattlegroundGOSpawnPoint const BG_RL_GameObjects[BG_RL_OBJECT_MAX] =
{
    { BG_RL_OBJECT_TYPE_DOOR_1, { 1293.561000f, 1601.938000f, 31.605570f, -1.45734900f }, { 0.f, 0.f, -0.66588130f,  0.7460576f }, RESPAWN_IMMEDIATELY },
    { BG_RL_OBJECT_TYPE_DOOR_2, { 1278.648000f, 1730.557000f, 31.605570f,  1.68424500f }, { 0.f, 0.f,  0.74605820f,  0.6658807f }, RESPAWN_IMMEDIATELY },
    { BG_RL_OBJECT_TYPE_BUFF_1, { 1328.719971f, 1632.719971f, 36.730400f, -1.44862400f }, { 0.f, 0.f,  0.66262010f, -0.7489557f }, 2 * MINUTE          },
    { BG_RL_OBJECT_TYPE_BUFF_2, { 1243.300049f, 1699.170044f, 34.872601f, -0.06981307f }, { 0.f, 0.f,  0.03489945f, -0.9993908f }, 2 * MINUTE          }
};

BattlegroundRL::BattlegroundRL()
{
    SetGameObjectsNumber(BG_RL_OBJECT_MAX);
}

void BattlegroundRL::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    _events.Update(diff);

    while (uint32 eventId = _events.ExecuteEvent())
    {
        switch (eventId)
        {
            case BG_RL_EVENT_REMOVE_DOORS:
                for (uint32 i = BG_RL_OBJECT_DOOR_1; i <= BG_RL_OBJECT_DOOR_2; ++i)
                    DelObject(i);
                break;
            default:
                break;
        }
    }
}

void BattlegroundRL::StartingEventCloseDoors()
{
    for (uint32 i = BG_RL_OBJECT_DOOR_1; i <= BG_RL_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
}

void BattlegroundRL::StartingEventOpenDoors()
{
    for (uint32 i = BG_RL_OBJECT_BUFF_1; i <= BG_RL_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, 60);

    for (uint32 i = BG_RL_OBJECT_DOOR_1; i <= BG_RL_OBJECT_DOOR_2; ++i)
        DoorOpen(i);
    _events.ScheduleEvent(BG_RL_EVENT_REMOVE_DOORS, BG_RL_REMOVE_DOORS_TIMER);
}

void BattlegroundRL::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (trigger)
    {
        case 4696:                                          // buff trigger?
        case 4697:                                          // buff trigger?
            break;
        default:
            Battleground::HandleAreaTrigger(player, trigger);
            break;
    }
}

void BattlegroundRL::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(3002, 1); // BATTELGROUND_RUINS_OF_LORDAERNON_SHOW

    Arena::FillInitialWorldStates(packet);
}

bool BattlegroundRL::SetupBattleground()
{
    for (uint32 i = 0; i < BG_RL_OBJECT_MAX; ++i)
    {
        BattlegroundGOSpawnPoint const& object = BG_RL_GameObjects[i];
        if (!AddObject(i, object.Entry, object.Pos, object.Rot, object.SpawnTime))
        {
            FMT_LOG_ERROR("bg.battleground", "BattleGroundRL: Failed to spawn GameObject! (Entry: {}). Battleground not created!", object.Entry);
            return false;
        }
    }

    return true;
}
