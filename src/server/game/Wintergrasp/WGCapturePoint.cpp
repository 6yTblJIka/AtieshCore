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

#include "WGCapturePoint.h"
#include "WintergraspMgr.h"
#include "WGWorkshop.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"

WGCapturePoint::WGCapturePoint(WintergraspMgr* WG, WintergraspWorkshopIds workshopType, TeamId team)
{
    m_team = team;
    m_value = 0;
    m_minValue = 0.0f;
    m_maxValue = 0.0f;
    m_State = BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL;
    m_OldState = BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL;
    m_capturePointEntry = 0;
    m_neutralValuePct = 0;
    m_maxSpeed = 0;

    m_capturePointGUID.Clear();
    m_WG = WG;

    GameObject* goWorkshop = ASSERT_NOTNULL(WG->SpawnGameObject(WGworkshopData[workshopType].entry, WGworkshopData[workshopType].Pos, WGworkshopData[workshopType].Rot));
    SetCapturePointData(goWorkshop);
    SetInitialData(team);
}

WGCapturePoint::~WGCapturePoint()
{
    for (uint8 i = 0; i < PVP_TEAMS_COUNT; i++)
        m_activePlayers[i].clear();

    m_team = TEAM_ALLIANCE;
    m_value = 0;
    m_minValue = 0.0f;
    m_maxValue = 0.0f;
    m_State = BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL;
    m_OldState = BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL;
    m_capturePointEntry = 0;
    m_neutralValuePct = 0;
    m_maxSpeed = 0;

    if (GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID))
        capturePoint->DespawnOrUnsummon();
    m_capturePointGUID.Clear();
    m_WG = nullptr;
}

bool WGCapturePoint::HandlePlayerEnter(Player* player)
{
    if (m_capturePointGUID)
    {
        if (GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID))
        {
            player->SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldState1, 1);
            player->SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldstate2, uint32(ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f)));
            player->SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldstate3, m_neutralValuePct);
        }
    }

    return m_activePlayers[player->GetTeamId()].insert(player->GetGUID()).second;
}

GuidSet::iterator WGCapturePoint::HandlePlayerLeave(Player* player)
{
    if (m_capturePointGUID)
        if (GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID))
            player->SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldState1, 0);

    GuidSet::iterator current = m_activePlayers[player->GetTeamId()].find(player->GetGUID());

    if (current == m_activePlayers[player->GetTeamId()].end())
        return current; // return end()

    m_activePlayers[player->GetTeamId()].erase(current++);
    return current;
}

void WGCapturePoint::SendChangePhase()
{
    if (!m_capturePointGUID)
        return;

    if (GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID))
    {
        // send this too, sometimes the slider disappears, dunno why :(
        SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldState1, 1);
        // send these updates to only the ones in this objective
        SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldstate2, (uint32)std::ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f));
        // send this too, sometimes it resets :S
        SendUpdateWorldState(capturePoint->GetGOInfo()->capturePoint.worldstate3, m_neutralValuePct);
    }
}

bool WGCapturePoint::ChangeState()
{
    bool result = false;
    if (!m_capturePointGUID)
        return result;

    if (GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID))
    {
        uint32 artkit = 0;
        switch (m_State)
        {
            case BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE: artkit = 1; break;
            case BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE: artkit = 2; break;
            default:
                break;
        }

        if (capturePoint->GetGoArtKit() != artkit)
        {
            switch (artkit)
            {
            case 0: m_team = TEAM_NEUTRAL; break;
            case 1: m_team = TEAM_HORDE; break;
            case 2: m_team = TEAM_ALLIANCE; break;
            }
            result = true;
        }
        capturePoint->SetGoArtKit(artkit);
    }

    return result;
}

void WGCapturePoint::SetInitialData(TeamId team)
{
    switch (team)
    {
        case TEAM_ALLIANCE:
        {
            m_value = m_maxValue;
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE);
            m_team = TEAM_ALLIANCE;
            break;
        }
        case TEAM_HORDE:
        {
            m_value = -m_maxValue;
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE);
            m_team = TEAM_HORDE;
            break;
        }
        case TEAM_NEUTRAL:
        {
            m_value = 0;
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL);
            m_team = TEAM_NEUTRAL;
        }
    }

    ChangeState();
}

bool WGCapturePoint::SetCapturePointData(GameObject* capturePoint)
{
    ASSERT(capturePoint);

    FMT_LOG_DEBUG("bg.battlefield", "Creating capture point {}", capturePoint->GetEntry());

    m_capturePointGUID = ObjectGuid(HighGuid::GameObject, capturePoint->GetEntry(), capturePoint->GetGUID().GetCounter());

    // check info existence
    GameObjectTemplate const* goinfo = capturePoint->GetGOInfo();
    if (goinfo->type != GAMEOBJECT_TYPE_CAPTURE_POINT)
    {
        FMT_LOG_ERROR("bg.battlefield", "SetCapturePointData: GO {} is not a capture point!", capturePoint->GetEntry());
        return false;
    }

    // get the needed values from goinfo
    m_maxValue = (float)goinfo->capturePoint.maxTime;
    m_maxSpeed = m_maxValue / (goinfo->capturePoint.minTime ? goinfo->capturePoint.minTime : 60);
    m_neutralValuePct = goinfo->capturePoint.neutralPercent;
    m_minValue = CalculatePct(m_maxValue, m_neutralValuePct);

    return true;
}

GameObject* WGCapturePoint::GetCapturePointGo()
{
    return m_WG->GetGameObject(m_capturePointGUID);
}

bool WGCapturePoint::DelCapturePoint()
{
    if (m_capturePointGUID)
    {
        if (GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID))
        {
            capturePoint->SetRespawnTime(0);                  // not save respawn time
            capturePoint->Delete();
            capturePoint = nullptr;
        }
        m_capturePointGUID.Clear();
    }

    return true;
}

bool WGCapturePoint::Update(uint32 diff)
{
    if (!m_WG->IsWarTime())
        return false;

    GameObject* capturePoint = m_WG->GetGameObject(m_capturePointGUID);
    if (!capturePoint)
        return false;

    float radius = (float)capturePoint->GetGOInfo()->capturePoint.radius / 2;

    for (uint32 team = 0; team < 2; ++team)
    {
        for (GuidSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end();)
        {
            ObjectGuid playerGuid = *itr;
            ++itr;

            if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
                if (!capturePoint->IsWithinDistInMap(player, radius) || !player->IsOutdoorPvPActive())
                    HandlePlayerLeave(player);
        }
    }

    std::vector<Player*> _players;
    Trinity::AnyPlayerInObjectRangeCheck checker(capturePoint, radius);
    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(capturePoint, _players, checker);
    Cell::VisitWorldObjects(capturePoint, searcher, radius);
    for (auto const& pointer : _players)
    {
        if (pointer->IsOutdoorPvPActive())
            if (m_activePlayers[pointer->GetTeamId()].insert(pointer->GetGUID()).second)
                HandlePlayerEnter(pointer);
    }
    _players.clear();
    // get the difference of numbers
    float fact_diff = ((float)m_activePlayers[0].size() - (float)m_activePlayers[1].size()) * diff / BATTLEFIELD_OBJECTIVE_UPDATE_INTERVAL;
    if (!fact_diff)
        return false;

    uint32 Challenger = 0;
    float maxDiff = m_maxSpeed * diff;

    if (fact_diff < 0)
    {
        // horde is in majority, but it's already horde-controlled -> no change
        if (m_State == BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE && m_value <= -m_maxValue)
            return false;

        if (fact_diff < -maxDiff)
            fact_diff = -maxDiff;

        Challenger = HORDE;
    }
    else
    {
        // ally is in majority, but it's already ally-controlled -> no change
        if (m_State == BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE && m_value >= m_maxValue)
            return false;

        if (fact_diff > maxDiff)
            fact_diff = maxDiff;

        Challenger = ALLIANCE;
    }

    float oldValue = m_value;
    //TeamId oldTeam = m_team;

    m_OldState = m_State;

    m_value += fact_diff;

    if (m_value < -m_minValue) // red
    {
        if (m_value < -m_maxValue)
            m_value = -m_maxValue;
        m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE);
        m_team = TEAM_HORDE;
    }
    else if (m_value > m_minValue) // blue
    {
        if (m_value > m_maxValue)
            m_value = m_maxValue;
        m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE);
        m_team = TEAM_ALLIANCE;
    }
    else if (oldValue * m_value <= 0) // grey, go through mid point
    {
        // if challenger is ally, then n->a challenge
        if (Challenger == ALLIANCE)
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE);
        // if challenger is horde, then n->h challenge
        else if (Challenger == HORDE)
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE);
        m_team = TEAM_NEUTRAL;
    }
    else // grey, did not go through mid point
    {
        // old phase and current are on the same side, so one team challenges the other
        if (Challenger == ALLIANCE && (m_OldState == BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE || m_OldState == BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE))
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE);
        else if (Challenger == HORDE && (m_OldState == BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE || m_OldState == BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE))
            m_State = BattlefieldObjectiveStates(BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE);
        m_team = TEAM_NEUTRAL;
    }

    if (m_value != oldValue)
        SendChangePhase();

    if (m_OldState != m_State)
    {
        //FMT_LOG_ERROR("bg.battlefield", "{}->{}", m_OldState, m_State);
        ChangeState();
        ChangeTeam(m_team);
        return true;
    }

    return false;
}

void WGCapturePoint::ChangeTeam(TeamId newTeam)
{
    WGWorkshop* ws = ASSERT_NOTNULL(m_WG->GetWGWorkshopByCapturePoint(this));
    ws->GiveControlTo(newTeam);
}

void WGCapturePoint::SendUpdateWorldState(uint32 field, uint32 value)
{
    for (uint8 team = 0; team < PVP_TEAMS_COUNT; ++team)
        for (GuidSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end(); ++itr)  // send to all players present in the area
            if (Player* player = ObjectAccessor::FindPlayer(*itr))
                player->SendUpdateWorldState(field, value);
}

void WGCapturePoint::SendObjectiveComplete(uint32 id, ObjectGuid guid)
{
    uint8 team;
    switch (m_State)
    {
        case BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE:
            team = TEAM_ALLIANCE;
            break;
        case BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE:
            team = TEAM_HORDE;
            break;
        default:
            return;
    }

    // send to all players present in the area
    for (GuidSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(*itr))
            player->KilledMonsterCredit(id, guid);
}

bool WGCapturePoint::IsInsideObjective(Player* player) const
{
    return m_activePlayers[player->GetTeamId()].find(player->GetGUID()) != m_activePlayers[player->GetTeamId()].end();
}
