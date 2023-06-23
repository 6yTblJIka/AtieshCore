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

 // some class predefs
class Creature;
class Player;

class TC_GAME_API WGGraveyard
{
public:
    WGGraveyard(WintergraspMgr* WG);
    ~WGGraveyard();

    // Method to changing who controls the graveyard
    void GiveControlTo(TeamId team);
    TeamId GetControlTeamId() const { return _ControlTeam; }

    // Find the nearest graveyard to a player
    float GetDistance(Player* player);

    // Initialize the graveyard
    void Initialize(WGGraveyardId type);

    // Set spirit service for the graveyard
    void SetSpirit(Creature* spirit, TeamId team);

    // Add a player to the graveyard
    void AddPlayer(ObjectGuid player_guid);

    // Remove a player from the graveyard
    void RemovePlayer(ObjectGuid player_guid);

    // Resurrect players
    void Resurrect();

    // Move players waiting to that graveyard on the nearest one
    void RelocateDeadPlayers();

    // Check if this graveyard has a spirit guide
    bool HasNpc(ObjectGuid guid);

    // Check if a player is in this graveyard's resurrect queue
    bool HasPlayer(ObjectGuid guid) const { return _resurrectQueue.find(guid) != _resurrectQueue.end(); }

    // Get the graveyard's ID.
    uint32 GetGraveyardId() const { return _GraveyardId; }

    void SetTextId(uint32 textId) { _GossipTextId = textId; }
    uint32 GetTextId() const { return _GossipTextId; }

private:
    TeamId _ControlTeam;
    uint32 _GraveyardId;
    uint32 _GossipTextId;
    ObjectGuid _SpiritGuide[PVP_TEAMS_COUNT];
    GuidUnorderedSet _resurrectQueue;
    WintergraspMgr* _WG;
};
