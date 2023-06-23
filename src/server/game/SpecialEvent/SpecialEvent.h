/*
 * Copyright (C) 2018+ AtieshCore <https://at-wow.org/>
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

#ifndef SPECIALEVENT_H_
#define SPECIALEVENT_H_

#include "Timer.h"
#include "SharedDefines.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ZoneScript.h"

enum SpecialEventTypes
{
    SPECIALEVENT_EVENTID_DALARANCRATER  = 1,
    SPECIALEVENT_EVENTID_THELIGHTOFDAWN = 2,
    SPECIALEVENT_EVENTID_MAX
};

namespace WorldPackets
{
    namespace WorldState
    {
        class InitWorldStates;
    }
}

enum SpecialEventTimers
{
    SPECIALEVENT_OBJECTIVE_UPDATE_INTERVAL = 1000
};

class BattlefieldGraveyard;
class ObjectGuid;
class Player;
class Unit;

class TC_GAME_API SpecialEvent : public ZoneScript
{
    friend class SpecialEventMgr;
    public:

        // ctor
        SpecialEvent();
        // dtor
        virtual ~SpecialEvent();

        // enables or disables the special event
        void StartSpecialEvent();
        void EndSpecialEvent(bool endByTimer);

        bool IsEnabled() const { return _enabled; }
        bool IsActiveTime() const { return _active; }
        bool IsRepeatable() const { return _Repeatable; }

        uint32 GetCooldownTimer() { return _EventTime; }
        uint32 GetDurationTimer() { return _noEventTime; }
        time_t GetTimeOfNextEvent() { return _gameTimeNextEvent; }
        std::string GetComment() { return _comment; }
        void RegisterEvent(uint32 eventId);
        void RegisterZoneIdForEvent(uint32 zoneId);
        uint32 GetEventId() const { return _eventId; }
        // Battle timer
        uint32 GetTimer() const { return _timer.GetExpiry().count(); }

        // Functions for each Event Scripts
        virtual bool SetupSpecialEvent(bool enabled, bool active, bool repeatable, uint32 id, uint32 cooldownTimer, uint32 durationTimer, std::string comment);
        virtual void Update(uint32 diff);
        virtual void OnSpecialEventStart() { }
        virtual void OnSpecialEventEnd(bool /*endByTimer*/) { }
        virtual void AddPlayer(ObjectGuid /*playerGUID*/) { }
        virtual void RemovePlayer(ObjectGuid /*playerGUID*/) { }
        virtual bool IsPossibleToRegister() { return true; }
        virtual bool IsMemberOfEvent(Player* /*player*/) { return false; }
        virtual uint32 GetCountPlayerInEvent() { return 0; }
        virtual void DoActionForMember(ObjectGuid /*playerGUID*/, uint32 /*param*/) { }

        // Can players inside the battlefield zone use ground mounts?
        virtual bool IsMountAllowed() const { return true; }
        // Can players inside the battlefield zone use flying mounts?
        virtual bool IsFlyingMountAllowed() const { return true; }
        // Is the referenced SpellArea spellId allowed for the referenced player and newArea?
        virtual bool IsSpellAreaAllowed(uint32 /*spellId*/, Player const* /*player*/, uint32 /*newArea*/) const { return false; }

        // Zone Sector
        // Called when a player enters the zone
        virtual void HandlePlayerEnterZone(Player* /*player*/, uint32 /*zoneId*/) { }
        // Called when a player leaves the zone
        virtual void HandlePlayerLeaveZone(Player* /*player*/, uint32 /*zoneId*/) { }
        // Called when a player enters the zone and area
        virtual void HandlePlayerEnterArea(Player* /*player*/, uint32 /*zoneId*/, uint32 /*areaId*/) { }
        // Called when a player leaves the zone and area
        virtual void HandlePlayerLeaveArea(Player* /*player*/, uint32 /*zoneId*/, uint32 /*areaId*/) { }
        // Called when a player inside the battlefield zone kills a unit
        virtual void HandleKill(Player* /*killer*/, Unit* /*victim*/) { }
        // Called when a player queries a gossip from a spirit healer
        virtual void HandleAreaSpiritHealerQueryOpcode(Player* /*player*/, ObjectGuid /*source*/) { }
        // Called when a player moves into a resurrection queue
        virtual void HandleAddPlayerToResurrectionQueue(Player* /*player*/, ObjectGuid /*source*/) { }
        // Called when a player moves out of a resurrection queue
        virtual void HandleRemovePlayerFromResurrectionQueue(Player* /*player*/) { }

        virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }
        virtual void SendGlobalWorldStates(Player const* /*player*/) const { }

        // Closest available graveyard WorldSafeLocsEntry for Player's TeamId
        virtual WorldSafeLocsDBC const* GetClosestGraveyardLocation(Player* who)
        {
            return sObjectMgr->GetClosestGraveyard(who->GetPositionX(), who->GetPositionY(), who->GetPositionZ(), who->GetMapId(), who->GetTeam());
        }

        // enum PvPTeamId
        virtual PvPTeamId GetControllingTeam() const { return PVP_TEAM_NEUTRAL; }
        // enum PvPTeamId
        virtual PvPTeamId GetAttackingTeam() const { return PVP_TEAM_NEUTRAL; }
        // enum TeamId
        virtual TeamId GetControllingTeamId() const { return TEAM_NEUTRAL; }
        // enum TeamId
        virtual TeamId GetAttackingTeamId() const { return TEAM_NEUTRAL; }

    protected:
        // constant information
        uint32 _eventId;
        TimeTracker _timer;
        // from database
        bool _active;
        bool _enabled;
        bool _Repeatable;
        uint32 _EventTime;      // length of the event
        uint32 _noEventTime;    // time between two events
        std::string _comment;
        time_t _gameTimeNextEvent;
};

#endif
