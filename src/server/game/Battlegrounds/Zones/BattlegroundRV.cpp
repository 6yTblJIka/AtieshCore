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

#include "BattlegroundRV.h"
#include "GameObject.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldStatePackets.h"

BattlegroundGOSpawnPoint const BG_RV_GameObjects[BG_RV_OBJECT_MAX] =
{
    { BG_RV_OBJECT_TYPE_BUFF_1,            { 735.551819f, -284.794678f, 28.276682f, 0.034906f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_BUFF_2,            { 791.224487f, -284.794464f, 28.276682f, 2.600535f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_FIRE_1,            { 743.543457f, -283.799469f, 28.286655f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_FIRE_2,            { 782.971802f, -283.799469f, 28.286655f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_FIREDOOR_1,        { 743.711060f, -284.099609f, 27.542587f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_FIREDOOR_2,        { 783.221252f, -284.133362f, 27.535686f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_1,           { 763.632385f, -306.162384f, 25.909504f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_3,           { 763.611145f, -261.856750f, 25.909504f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_GEAR_1,            { 763.664551f, -261.872986f, 26.686588f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_GEAR_2,            { 763.578979f, -306.146149f, 26.665222f, 3.141593f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_2,           { 723.644287f, -284.493256f, 24.648525f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_4,           { 802.211609f, -284.493256f, 24.648525f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PULLEY_1,          { 700.722290f, -283.990662f, 39.517582f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PULLEY_2,          { 826.303833f, -283.996429f, 39.517582f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_COLLISION_1, { 763.632385f, -306.162384f, 30.639660f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_COLLISION_2, { 723.644287f, -284.493256f, 32.382710f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_COLLISION_3, { 763.611145f, -261.856750f, 30.639660f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_PILAR_COLLISION_4, { 802.211609f, -284.493256f, 32.382710f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_ELEVATOR_1,        { 763.536377f, -294.535767f, 0.5053830f, 3.141593f }, { 0.f, 0.f, -1.f, 0.f }, RESPAWN_IMMEDIATELY },
    { BG_RV_OBJECT_TYPE_ELEVATOR_2,        { 763.506348f, -273.873352f, 0.5053830f, 0.000000f }, { 0.f, 0.f,  0.f, 1.f }, RESPAWN_IMMEDIATELY }
};

BattlegroundRV::BattlegroundRV()
{
    SetGameObjectsNumber(BG_RV_OBJECT_MAX);

    _timer = BG_RV_TELEPORT_ON_ELEVATOR;
    _state = 0;
    _pillarCollision = false;
}

void BattlegroundRV::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (_timer <= diff)
    {
        switch (_state)
        {
            case BG_RV_STATE_OPEN_FENCES:
                // Open fire (only at game start)
                for (uint8 i = BG_RV_OBJECT_FIRE_1; i <= BG_RV_OBJECT_FIREDOOR_2; ++i)
                    DoorOpen(i);
                _timer = BG_RV_CLOSE_FIRE_TIMER;
                _state = BG_RV_STATE_CLOSE_FIRE;
                break;
            case BG_RV_STATE_CLOSE_FIRE:
                for (uint8 i = BG_RV_OBJECT_FIRE_1; i <= BG_RV_OBJECT_FIREDOOR_2; ++i)
                    DoorClose(i);
                // Fire got closed after five seconds, leaves twenty seconds before toggling pillars
                _timer = BG_RV_FIRE_TO_PILLAR_TIMER;
                _state = BG_RV_STATE_SWITCH_PILLARS;
                break;
            case BG_RV_STATE_SWITCH_PILLARS:
                TogglePillarCollision();
                _timer = BG_RV_PILLAR_SWITCH_TIMER;
                break;
        }
    }
    else
        _timer -= diff;
}

void BattlegroundRV::StartingEventOpenDoors()
{
    // Buff respawn
    SpawnBGObject(BG_RV_OBJECT_BUFF_1, 90);
    SpawnBGObject(BG_RV_OBJECT_BUFF_2, 90);
    // Elevators
    for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
        {
            // just safe check each togglePillars for players which droped underground
            if (player->GetPositionZ() < 2.f)
                player->NearTeleportTo(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ() + 3.0f, player->GetOrientation());
        }

    DoorOpen(BG_RV_OBJECT_ELEVATOR_1);
    DoorOpen(BG_RV_OBJECT_ELEVATOR_2);

    _state = BG_RV_STATE_OPEN_FENCES;
    _timer = BG_RV_FIRST_TIMER;

    // Should be false at first, TogglePillarCollision will do it.
    _pillarCollision = true;
    TogglePillarCollision();
}

void BattlegroundRV::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (trigger)
    {
        case 5224:
        case 5226:
        // fire was removed in 3.2.0
        case 5473:
        case 5474:
            break;
        default:
            Battleground::HandleAreaTrigger(player, trigger);
            break;
    }
}

void BattlegroundRV::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(BG_RV_WORLD_STATE, 1);

    Arena::FillInitialWorldStates(packet);
}

bool BattlegroundRV::SetupBattleground()
{
    for (uint32 i = 0; i < BG_RV_OBJECT_MAX; ++i)
    {
        BattlegroundGOSpawnPoint const& object = BG_RV_GameObjects[i];
        if (!AddObject(i, object.Entry, object.Pos, object.Rot, object.SpawnTime))
        {
            FMT_LOG_ERROR("bg.battleground", "BattleGroundRV: Failed to spawn GameObject! (Entry: {}). Battleground not created!", object.Entry);
            return false;
        }
    }

    return true;
}

void BattlegroundRV::TogglePillarCollision()
{
    // Toggle visual pillars, pulley, gear, and collision based on previous state
    for (uint8 i = BG_RV_OBJECT_PILAR_1; i <= BG_RV_OBJECT_GEAR_2; ++i)
        _pillarCollision ? DoorOpen(i) : DoorClose(i);

    for (uint8 i = BG_RV_OBJECT_PILAR_2; i <= BG_RV_OBJECT_PULLEY_2; ++i)
        _pillarCollision ? DoorClose(i) : DoorOpen(i);

    for (uint8 i = BG_RV_OBJECT_PILAR_1; i <= BG_RV_OBJECT_PILAR_COLLISION_4; ++i)
    {
        if (GameObject* go = GetBGObject(i))
        {
            if (i >= BG_RV_OBJECT_PILAR_COLLISION_1)
            {
                GOState state = ((go->GetGOInfo()->door.startOpen != 0) == _pillarCollision) ? GO_STATE_ACTIVE : GO_STATE_READY;
                go->SetGoState(state);
            }
        }
    }

    _pillarCollision = !_pillarCollision;
}
