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

#include "WGGraveyard.h"
#include "Battleground.h"
#include "WintergraspMgr.h"

WGGraveyard::WGGraveyard(WintergraspMgr* WG)
{
    _ControlTeam = TEAM_NEUTRAL;
    _GraveyardId = 0;
    _GossipTextId = 0;
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        _SpiritGuide[i].Clear();
    _resurrectQueue.clear();
    _WG = WG;
}

WGGraveyard::~WGGraveyard()
{
    _ControlTeam = TEAM_ALLIANCE;
    _GraveyardId = 0;
    _GossipTextId = 0;
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        _SpiritGuide[i].Clear();
    _resurrectQueue.clear();
    _WG = nullptr;
}

void WGGraveyard::Initialize(WGGraveyardId type)
{
    switch (type)
    {
        case BATTLEFIELD_WG_GY_WORKSHOP_SE:
            _ControlTeam = _WG->GetAttackerTeam();
            _GraveyardId = 1333;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_SE);
            break;
        case BATTLEFIELD_WG_GY_WORKSHOP_SW:
            _ControlTeam = _WG->GetAttackerTeam();
            _GraveyardId = 1334;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_SW);
            break;
        case BATTLEFIELD_WG_GY_WORKSHOP_NE:
            _ControlTeam = _WG->GetDefenderTeam();
            _GraveyardId = 1329;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_NE);
            break;
        case BATTLEFIELD_WG_GY_WORKSHOP_NW:
            _ControlTeam = _WG->GetDefenderTeam();
            _GraveyardId = 1330;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_NW);
            break;
        case BATTLEFIELD_WG_GY_KEEP:
            _ControlTeam = _WG->GetDefenderTeam();
            _GraveyardId = 1285;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_KEEP);
            break;
        case BATTLEFIELD_WG_GY_HORDE:
            _ControlTeam = TEAM_HORDE;
            _GraveyardId = 1331;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_HORDE);
            break;
        case BATTLEFIELD_WG_GY_ALLIANCE:
            _ControlTeam = TEAM_ALLIANCE;
            _GraveyardId = 1332;
            SetTextId(BATTLEFIELD_WG_GOSSIPTEXT_GY_ALLIANCE);
            break;
        default:
            break;
    }    
}

void WGGraveyard::SetSpirit(Creature* spirit, TeamId team)
{
    if (!spirit)
    {
        FMT_LOG_ERROR("bg.battlefield", "BfGraveyard::SetSpirit: Invalid Spirit.");
        return;
    }

    _SpiritGuide[team] = spirit->GetGUID();
    spirit->SetReactState(REACT_PASSIVE);
}

float WGGraveyard::GetDistance(Player* player)
{
    WorldSafeLocsDBC const* safeLoc = sDBCStoresMgr->GetWorldSafeLocsDBC(_GraveyardId);
    return player->GetDistance2d(safeLoc->Loc.X, safeLoc->Loc.Y);
}

void WGGraveyard::AddPlayer(ObjectGuid playerGuid)
{
    if (!HasPlayer(playerGuid))
    {
        _resurrectQueue.insert(playerGuid);

        if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
            player->CastSpell(player, SPELL_WAITING_FOR_RESURRECT, true);
    }
}

void WGGraveyard::RemovePlayer(ObjectGuid playerGuid)
{
    if (HasPlayer(playerGuid))
    {
        _resurrectQueue.erase(_resurrectQueue.find(playerGuid));

        if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
            player->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);
    }
}

void WGGraveyard::Resurrect()
{
    if (_resurrectQueue.empty())
        return;

    for (auto itr = _resurrectQueue.begin(); itr != _resurrectQueue.end(); ++itr)
    {
        // Get player object from his guid
        Player* player = ObjectAccessor::FindPlayer(*itr);
        if (!player)
            continue;

        // Check if the player is in world and on the good graveyard
        if (player->IsInWorld())
            if (Creature* spirit = _WG->GetCreature(_SpiritGuide[_ControlTeam]))
                spirit->CastSpell(spirit, SPELL_SPIRIT_HEAL, true);

        // Resurrect player
        player->CastSpell(player, SPELL_RESURRECTION_VISUAL, true);
        player->ResurrectPlayer(1.0f);
        player->CastSpell(player, 6962, true);
        player->CastSpell(player, SPELL_SPIRIT_HEAL_MANA, true);
        //player->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);
        player->SpawnCorpseBones(false);
    }

    _resurrectQueue.clear();
}

// For changing graveyard control
void WGGraveyard::GiveControlTo(TeamId team)
{
    _ControlTeam = team;
    // Teleport to other graveyard, player witch were on this graveyard
    RelocateDeadPlayers();
}

void WGGraveyard::RelocateDeadPlayers()
{
    WorldSafeLocsDBC const* closestGrave = nullptr;
    GuidSet teleportSet;
    for (auto itr = _resurrectQueue.begin(); itr != _resurrectQueue.end(); ++itr)
    {
        Player* player = ObjectAccessor::FindConnectedPlayer(*itr);
        if (!player)
            continue;

        //teleport only online-players
        teleportSet.insert(*itr);
    }
    _resurrectQueue.clear();

    for (auto const& Tguid : teleportSet)
    {
        if (Player* player = ObjectAccessor::FindConnectedPlayer(Tguid))
        {
            if (!closestGrave)
            {
                closestGrave = ASSERT_NOTNULL(_WG->GetClosestGraveyard(player));
                player->TeleportTo(player->GetMapId(), closestGrave->Loc.X, closestGrave->Loc.Y, closestGrave->Loc.Z, player->GetOrientation());
            }
            else
                player->TeleportTo(player->GetMapId(), closestGrave->Loc.X, closestGrave->Loc.Y, closestGrave->Loc.Z, player->GetOrientation());
        }
    }
    teleportSet.clear();
}

bool WGGraveyard::HasNpc(ObjectGuid guid)
{
    if (!_SpiritGuide[TEAM_ALLIANCE] || !_SpiritGuide[TEAM_HORDE])
        return false;

    if (!_WG->GetCreature(_SpiritGuide[TEAM_ALLIANCE]) ||
        !_WG->GetCreature(_SpiritGuide[TEAM_HORDE]))
        return false;

    return (_SpiritGuide[TEAM_ALLIANCE] == guid || _SpiritGuide[TEAM_HORDE] == guid);
}
