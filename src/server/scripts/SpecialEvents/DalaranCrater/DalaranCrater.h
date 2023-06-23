/*
 * Copyright (C) 2019+ ATieshCore <https://at-wow.org/>
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

#ifndef _DALARANGEVENT_H
#define _DALARANGEVENT_H

#include "GameTime.h"
#include "ObjectGuid.h"
#include "SpecialEvent.h"
#include "Position.h"
#include "World.h"

WorldLocation const DalaranCrater(0, 276.82178f, 368.0858f, 121.2f, 4.135169f);
WorldLocation const DalaranCraterPoint(0, 287.2068f, 381.9773f, -67.5977f, 4.135169f);

struct PlayersData
{
    uint32 mapid;
    float x;
    float y;
    float z;
};
typedef std::unordered_map<ObjectGuid, PlayersData> PlayersDataContainer;

enum DalaranPhase
{
    PREPARE_PHASE_60  = 1,
    PREPARE_PHASE_30  = 2,
    PREPARE_PHASE_0   = 3,
    BATTLE_PHASE_0    = 4,
    BATTLE_PHASE_1    = 5,
    BATTLE_PHASE_2    = 6,
    BATTLE_PHASE_3    = 7,
    BATTLE_ENDED      = 8
};

class TC_GAME_API DalaranGEvent : public SpecialEvent
{
public:
    DalaranGEvent();
    ~DalaranGEvent();

    bool IsActiveDalaranEvent() const { return activeFight; }

    // phase
    DalaranPhase GetPhase() { return phaseStage; }
    void SetPhase(DalaranPhase phase) { phaseStage = phase; }

    void Update(uint32 diff) override;
    void OnSpecialEventStart() override;
    void OnSpecialEventEnd(bool /*endByTimer*/) override;
    void AddPlayer(ObjectGuid playerGUID) override;
    void RemovePlayer(ObjectGuid playerGUID) override;
    bool IsPossibleToRegister() override { return registration; }
    bool IsMemberOfEvent(Player* player) override;
    uint32 GetCountPlayerInEvent() override { return m_playersDataStore.size(); }
    bool SetupSpecialEvent(bool enabled, bool active, bool repeatable, uint32 id, uint32 cooldownTimer, uint32 durationTimer, std::string comment) override;

    void StartFightEvent();
    void StopFightEvent();

    void SpawnGOLight();

    void TeleportAllPlayersInZone();
    void TeleportAllPlayersBack();

    void BroadcastToMemberAboutLeavePlayer(std::string const& Name);
    void BroadcastToMemberPrepare();

    // Fight stage
    void RemovePlayerFromFight(Player* player, bool withteleport = false);
    void ReceiveWinnerName();
    void DecreaseAliveCounter() { --alivePlayerCount; }
    // Queue stage
    void InvitePlayerToQueue(Player* player);
    void RemovePlayerFromQueue(Player* player);

private:
    bool activeFight;
    bool registration;

    uint32 possibleDistance;
    DalaranPhase phaseStage;
    // timers
    TimeTracker prepareTimer;
    TimeTracker combatTimer;
    TimeTracker playersTimer;

    uint32 m_DurationTimer;
    uint32 m_TeleporterTimer;
    uint32 alivePlayerCount;
    std::string _winnername;

    PlayersDataContainer m_playersDataStore;
};

#endif // _DALARANGEVENT_H
