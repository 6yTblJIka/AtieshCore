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

#include "BattlegroundWS.h"
#include "BattlegroundMgr.h"
#include "DBCStoresMgr.h"
#include "GameObject.h"
#include "Language.h"
#include "Log.h"
#include "Map.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldStatePackets.h"

// these variables aren't used outside of this file, so declare them only here
enum BG_WSG_Rewards
{
    BG_WSG_WIN = 0,
    BG_WSG_FLAG_CAP,
    BG_WSG_MAP_COMPLETE,
    BG_WSG_REWARD_NUM
};

enum BG_WSG_BuffEntries
{
    BG_WSG_SPEED_BUFF_1     = 179871,
    BG_WSG_SPEED_BUFF_2     = 179899,

    BG_WSG_FOOD_BUFF_1      = 179904,
    BG_WSG_FOOD_BUFF_2      = 179906,

    BG_WSG_BERSERK_BUFF_1   = 179905,
    BG_WSG_BERSERK_BUFF_2   = 179907
};

uint32 BG_WSG_Honor[BG_HONOR_MODE_NUM][BG_WSG_REWARD_NUM] =
{
    { 20, 40, 40 }, // normal honor
    { 60, 40, 80 }  // holiday
};

BattlegroundGOSpawnPoint const BG_WSG_GameObjects[BG_WS_OBJECT_MAX] =
{
    // alliance gates
    { BG_OBJECT_DOOR_A_1_WS_ENTRY,     { 1503.336f, 1493.466f, 352.1888f,  3.11538800f }, {  0.00000000f,  0.000000000f,  0.999914200f,  0.01310196f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_A_2_WS_ENTRY,     { 1492.479f, 1457.912f, 342.9689f,  3.11538800f }, {  0.00000000f,  0.000000000f,  0.999914200f,  0.01310196f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_A_3_WS_ENTRY,     { 1468.504f, 1494.357f, 351.8618f,  3.11538800f }, {  0.00000000f,  0.000000000f,  0.999914200f,  0.01310196f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_A_4_WS_ENTRY,     { 1471.556f, 1458.778f, 362.6332f,  3.11538800f }, {  0.00000000f,  0.000000000f,  0.999914200f,  0.01310196f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_A_5_WS_ENTRY,     { 1492.347f, 1458.340f, 342.3712f, -0.03490669f }, {  0.00000000f,  0.000000000f,  0.017452460f, -0.99984770f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_A_6_WS_ENTRY,     { 1503.466f, 1493.367f, 351.7352f, -0.03490669f }, {  0.00000000f,  0.000000000f,  0.017452460f, -0.99984770f }, RESPAWN_IMMEDIATELY     },
    // horde gates
    { BG_OBJECT_DOOR_H_1_WS_ENTRY,     { 949.1663f, 1423.773f, 345.6241f, -0.57567980f }, { -0.01673365f, -0.004955292f, -0.283971800f,  0.95867380f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_H_2_WS_ENTRY,     { 953.0507f, 1459.843f, 340.6526f,  4.28657000f }, { -0.19718220f,  0.157508900f, -0.823947900f,  0.50736580f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_H_3_WS_ENTRY,     { 949.9523f, 1422.751f, 344.9273f,  0.00000000f }, {  0.00000000f,  0.000000000f,  0.000000000f,  1.00000000f }, RESPAWN_IMMEDIATELY     },
    { BG_OBJECT_DOOR_H_4_WS_ENTRY,     { 950.7952f, 1459.583f, 342.1523f,  0.05235888f }, {  0.00000000f,  0.000000000f,  0.026176450f,  0.99965730f }, RESPAWN_IMMEDIATELY     },
    // flags
    { BG_OBJECT_A_FLAG_WS_ENTRY,       { 1540.423f, 1481.325f, 351.8284f,  3.08918300f }, {  0.00000000f,  0.000000000f,  0.999656700f,  0.02620165f }, BG_WS_FLAG_RESPAWN_TIME },
    { BG_OBJECT_H_FLAG_WS_ENTRY,       { 916.5073f, 1433.826f, 346.3796f,  0.01745246f }, {  0.00000000f,  0.000000000f,  0.008726120f,  0.99996190f }, BG_WS_FLAG_RESPAWN_TIME },
    // buffs
    { BG_WSG_SPEED_BUFF_1,             { 1449.930f, 1470.710f, 342.6346f, -1.64061000f }, {  0.00000000f,  0.000000000f, -0.731353800f,  0.68199830f }, BUFF_RESPAWN_TIME       },
    { BG_WSG_SPEED_BUFF_2,             { 1005.105f, 1448.015f, 335.9118f,  1.60570200f }, {  0.00000000f,  0.000000000f,  0.719339400f,  0.69465880f }, BUFF_RESPAWN_TIME       },
    { BG_WSG_FOOD_BUFF_1,              { 1317.506f, 1550.851f, 313.2344f, -0.26179820f }, {  0.00000000f,  0.000000000f, -0.130525600f,  0.99144490f }, BUFF_RESPAWN_TIME       },
    { BG_WSG_FOOD_BUFF_2,              { 1110.451f, 1353.656f, 316.5181f, -0.68067870f }, {  0.00000000f,  0.000000000f, -0.333807000f,  0.94264140f }, BUFF_RESPAWN_TIME       },
    { BG_WSG_BERSERK_BUFF_1,           { 1320.094f, 1378.790f, 314.7532f,  1.18682300f }, {  0.00000000f,  0.000000000f,  0.559192700f,  0.82903770f }, BUFF_RESPAWN_TIME       },
    { BG_WSG_BERSERK_BUFF_2,           { 1139.688f, 1560.288f, 306.8432f, -2.44346000f }, {  0.00000000f,  0.000000000f, -0.939692500f,  0.34202050f }, BUFF_RESPAWN_TIME       }
};

BattlegroundWS::BattlegroundWS()
{
    SetGameObjectsNumber(BG_WS_OBJECT_MAX);
    SetCreaturesNumber(BG_CREATURES_MAX_WS);

    StartMessageIds[BG_STARTING_EVENT_SECOND] = BG_WS_TEXT_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD] = BG_WS_TEXT_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = BG_WS_TEXT_BATTLE_HAS_BEGUN;

    /*StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_BG_WS_START_TWO_MINUTES;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;*/
    _flagSpellForceTimer = 0;
    _bothFlagsKept = false;
    _flagDebuffState = 0;
    m_FlagKeepers[TEAM_ALLIANCE].Clear();
    m_FlagKeepers[TEAM_HORDE].Clear();
    m_DroppedFlagGUID[TEAM_ALLIANCE].Clear();
    m_DroppedFlagGUID[TEAM_HORDE].Clear();
    _flagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_BASE;
    _flagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_BASE;
    _flagsTimer[TEAM_ALLIANCE] = 0;
    _flagsTimer[TEAM_HORDE] = 0;
    _flagsDropTimer[TEAM_ALLIANCE] = 0;
    _flagsDropTimer[TEAM_HORDE] = 0;
    _lastFlagCaptureTeam = 0;
    m_ReputationCapture = 0;
    m_HonorWinKills = 0;
    m_HonorEndKills = 0;
    _minutesElapsed = 0;
}

void BattlegroundWGScore::BuildObjectivesBlock(WorldPacket& data)
{
    data << uint32(2); // Objectives Count
    data << uint32(FlagCaptures);
    data << uint32(FlagReturns);
}

BattlegroundWS::~BattlegroundWS() { }

void BattlegroundWS::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (GetStartTime() >= 27 * MINUTE * IN_MILLISECONDS)
        {
            if (GetTeamScore(TEAM_ALLIANCE) == 0)
            {
                if (GetTeamScore(TEAM_HORDE) == 0)        // No one scored - result is tie
                    EndBattleground(0);
                else                                 // Horde has more points and thus wins
                    EndBattleground(HORDE);
            }
            else if (GetTeamScore(TEAM_HORDE) == 0)
                EndBattleground(ALLIANCE);           // Alliance has > 0, Horde has 0, alliance wins
            else if (GetTeamScore(TEAM_HORDE) == GetTeamScore(TEAM_ALLIANCE)) // Team score equal, winner is team that scored the last flag
                EndBattleground(_lastFlagCaptureTeam);
            else if (GetTeamScore(TEAM_HORDE) > GetTeamScore(TEAM_ALLIANCE))  // Last but not least, check who has the higher score
                EndBattleground(HORDE);
            else
                EndBattleground(ALLIANCE);
        }
        // first update needed after 1 minute of game already in progress
        else if (GetStartTime() > uint32(_minutesElapsed * MINUTE * IN_MILLISECONDS) + 3 * MINUTE * IN_MILLISECONDS)
        {
            ++_minutesElapsed;
            UpdateWorldState(BG_WS_STATE_TIMER, 25 - _minutesElapsed);
        }

        if (_flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            _flagsTimer[TEAM_ALLIANCE] -= diff;

            if (_flagsTimer[TEAM_ALLIANCE] < 0)
            {
                _flagsTimer[TEAM_ALLIANCE] = 0;
                RespawnFlag(ALLIANCE, true);
            }
        }

        if (_flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_GROUND)
        {
            _flagsDropTimer[TEAM_ALLIANCE] -= diff;

            if (_flagsDropTimer[TEAM_ALLIANCE] < 0)
            {
                _flagsDropTimer[TEAM_ALLIANCE] = 0;
                RespawnFlagAfterDrop(ALLIANCE);
                _bothFlagsKept = false;
            }
        }

        if (_flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            _flagsTimer[TEAM_HORDE] -= diff;

            if (_flagsTimer[TEAM_HORDE] < 0)
            {
                _flagsTimer[TEAM_HORDE] = 0;
                RespawnFlag(HORDE, true);
            }
        }

        if (_flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_GROUND)
        {
            _flagsDropTimer[TEAM_HORDE] -= diff;

            if (_flagsDropTimer[TEAM_HORDE] < 0)
            {
                _flagsDropTimer[TEAM_HORDE] = 0;
                RespawnFlagAfterDrop(HORDE);
                _bothFlagsKept = false;
            }
        }

        if (_bothFlagsKept)
        {
            _flagSpellForceTimer += diff;
            if (_flagDebuffState == 0 && _flagSpellForceTimer >= 10 * MINUTE * IN_MILLISECONDS)  //10 minutes
            {
                // Apply Stage 1 (Focused Assault)
                if (Player* player = ObjectAccessor::FindPlayer(m_FlagKeepers[0]))
                    player->CastSpell(player, WS_SPELL_FOCUSED_ASSAULT, true);
                if (Player* player = ObjectAccessor::FindPlayer(m_FlagKeepers[1]))
                    player->CastSpell(player, WS_SPELL_FOCUSED_ASSAULT, true);
                _flagDebuffState = 1;
            }
            else if (_flagDebuffState == 1 && _flagSpellForceTimer >= 15 * MINUTE * IN_MILLISECONDS) //15 minutes
            {
                // Apply Stage 2 (Brutal Assault)
                if (Player* player = ObjectAccessor::FindPlayer(m_FlagKeepers[0]))
                {
                    player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
                    player->CastSpell(player, WS_SPELL_BRUTAL_ASSAULT, true);
                }
                if (Player* player = ObjectAccessor::FindPlayer(m_FlagKeepers[1]))
                {
                    player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
                    player->CastSpell(player, WS_SPELL_BRUTAL_ASSAULT, true);
                }
                _flagDebuffState = 2;
            }
        }
        else if ((_flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_BASE || _flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_WAIT_RESPAWN) &&
         (_flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_BASE || _flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_WAIT_RESPAWN))
        {
            // Both flags are in base or awaiting respawn.
            // Remove assault debuffs, reset timers

            if (Player* player = ObjectAccessor::FindPlayer(m_FlagKeepers[0]))
            {
                player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
                player->RemoveAurasDueToSpell(WS_SPELL_BRUTAL_ASSAULT);
            }
            if (Player* player = ObjectAccessor::FindPlayer(m_FlagKeepers[1]))
            {
                player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
                player->RemoveAurasDueToSpell(WS_SPELL_BRUTAL_ASSAULT);
            }

            _flagSpellForceTimer = 0; //reset timer.
            _flagDebuffState = 0;
        }
    }
}

void BattlegroundWS::StartingEventCloseDoors()
{
    for (uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_H_4; ++i)
    {
        SpawnBGObject(i, BG_WSG_GameObjects[i].SpawnTime);
        DoorClose(i);
    }

    for (uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);

    UpdateWorldState(BG_WS_STATE_TIMER_ACTIVE, 1);
    UpdateWorldState(BG_WS_STATE_TIMER, 25);
}

void BattlegroundWS::StartingEventOpenDoors()
{
    for (uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    SpawnBGObject(BG_WS_OBJECT_DOOR_A_5, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_WS_OBJECT_DOOR_A_6, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_WS_OBJECT_DOOR_H_3, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_WS_OBJECT_DOOR_H_4, RESPAWN_ONE_DAY);

    for (uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_H_4; ++i)
        DoorOpen(i);

    // players joining later are not eligibles
    StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, WS_EVENT_START_BATTLE);
}

void BattlegroundWS::AddPlayer(Player* player)
{
    bool const isInBattleground = IsPlayerInBattleground(player->GetGUID());
    Battleground::AddPlayer(player);
    if (!isInBattleground)
        PlayerScores[player->GetGUID().GetCounter()] = new BattlegroundWGScore(player->GetGUID());
}

void BattlegroundWS::RespawnFlag(uint32 Team, bool captured)
{
    if (Team == ALLIANCE)
    {
        FMT_LOG_DEBUG("bg.battleground", "Respawn Alliance flag");
        _flagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_BASE;
    }
    else
    {
        FMT_LOG_DEBUG("bg.battleground", "Respawn Horde flag");
        _flagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_BASE;
    }

    if (captured)
    {
        //when map_update will be allowed for battlegrounds this code will be useless
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
        SendBroadcastText(BG_WS_TEXT_FLAGS_PLACED, CHAT_MSG_BG_SYSTEM_NEUTRAL);
        PlaySoundToAll(BG_WS_SOUND_FLAGS_RESPAWNED);        // flag respawned sound...
    }
    _bothFlagsKept = false;
}

void BattlegroundWS::RespawnFlagAfterDrop(uint32 team)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    RespawnFlag(team, false);
    if (team == ALLIANCE)
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
    else
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);

    SendBroadcastText(BG_WS_TEXT_FLAGS_PLACED, CHAT_MSG_BG_SYSTEM_NEUTRAL);

    PlaySoundToAll(BG_WS_SOUND_FLAGS_RESPAWNED);

    if (GameObject* obj = GetBgMap()->GetGameObject(GetDroppedFlagGUID(team)))
        obj->Delete();
    else
        FMT_LOG_ERROR("bg.battleground", "unknown dropped flag ({})", GetDroppedFlagGUID(team).ToString());

    SetDroppedFlagGUID(ObjectGuid::Empty, GetTeamIndexByTeamId(team));
    _bothFlagsKept = false;
    // Check opposing flag if it is in capture zone; if so, capture it
    HandleFlagRoomCapturePoint(team == ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE);
}

void BattlegroundWS::EventPlayerCapturedFlag(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 winner = 0;

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    if (player->GetTeam() == ALLIANCE)
    {
        if (!IsHordeFlagPickedup())
            return;
        SetHordeFlagPicker(ObjectGuid::Empty);              // must be before aura remove to prevent 2 events (drop+capture) at the same time
                                                            // horde flag in base (but not respawned yet)
        _flagState[TEAM_HORDE] = BG_WS_FLAG_STATE_WAIT_RESPAWN;
                                                            // Drop Horde Flag from Player
        player->RemoveAurasDueToSpell(BG_WS_SPELL_WARSONG_FLAG);
        if (_flagDebuffState == 1)
          player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
        else if (_flagDebuffState == 2)
          player->RemoveAurasDueToSpell(WS_SPELL_BRUTAL_ASSAULT);

        if (GetTeamScore(TEAM_ALLIANCE) < BG_WS_MAX_TEAM_SCORE)
            AddPoint(ALLIANCE, 1);
        PlaySoundToAll(BG_WS_SOUND_FLAG_CAPTURED_ALLIANCE);
    }
    else
    {
        if (!IsAllianceFlagPickedup())
            return;
        SetAllianceFlagPicker(ObjectGuid::Empty);           // must be before aura remove to prevent 2 events (drop+capture) at the same time
                                                            // alliance flag in base (but not respawned yet)
        _flagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_WAIT_RESPAWN;
                                                            // Drop Alliance Flag from Player
        player->RemoveAurasDueToSpell(BG_WS_SPELL_SILVERWING_FLAG);
        if (_flagDebuffState == 1)
          player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
        else if (_flagDebuffState == 2)
          player->RemoveAurasDueToSpell(WS_SPELL_BRUTAL_ASSAULT);

        if (GetTeamScore(TEAM_HORDE) < BG_WS_MAX_TEAM_SCORE)
            AddPoint(HORDE, 1);
        PlaySoundToAll(BG_WS_SOUND_FLAG_CAPTURED_HORDE);
    }
    RewardReputationToTeam(890, 889, m_ReputationCapture, player->GetTeam());
    //for flag capture is reward 2 honorable kills
    RewardHonorToTeam(GetBonusHonorFromKill(2), player->GetTeam());

    SpawnBGObject(BG_WS_OBJECT_H_FLAG, BG_WS_FLAG_RESPAWN_TIME);
    SpawnBGObject(BG_WS_OBJECT_A_FLAG, BG_WS_FLAG_RESPAWN_TIME);

    if (player->GetTeam() == ALLIANCE)
        SendBroadcastText(BG_WS_TEXT_CAPTURED_HORDE_FLAG, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
    else
        SendBroadcastText(BG_WS_TEXT_CAPTURED_ALLIANCE_FLAG, CHAT_MSG_BG_SYSTEM_HORDE, player);

    UpdateFlagState(player->GetTeam(), 1);                  // flag state none
    UpdateTeamScore(player->GetTeamId());
    // only flag capture should be updated
    UpdatePlayerScore(player, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures

    // update last flag capture to be used if teamscore is equal
    SetLastFlagCapture(player->GetTeam());

    if (GetTeamScore(TEAM_ALLIANCE) == BG_WS_MAX_TEAM_SCORE)
        winner = ALLIANCE;

    if (GetTeamScore(TEAM_HORDE) == BG_WS_MAX_TEAM_SCORE)
        winner = HORDE;

    if (winner)
    {
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 0);
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 0);
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, 1);
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, 1);
        UpdateWorldState(BG_WS_STATE_TIMER_ACTIVE, 0);

        RewardHonorToTeam(BG_WSG_Honor[m_HonorMode][BG_WSG_WIN], winner);
        EndBattleground(winner);
    }
    else
        _flagsTimer[GetTeamIndexByTeamId(player->GetTeam()) ? 0 : 1] = BG_WS_FLAG_RESPAWN_TIME * IN_MILLISECONDS;
}
void BattlegroundWS::HandleFlagRoomCapturePoint(int32 team)
{
    Player* flagCarrier = ObjectAccessor::GetPlayer(GetBgMap(), GetFlagPickerGUID(team));
    uint32 areaTrigger = team == TEAM_ALLIANCE ? 3647 : 3646;
    if (flagCarrier && flagCarrier->IsInAreaTriggerRadius(sDBCStoresMgr->GetAreaTriggerDBC(areaTrigger)))
        EventPlayerCapturedFlag(flagCarrier);
}

void BattlegroundWS::EventPlayerDroppedFlag(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        // if not running, do not cast things at the dropper player (prevent spawning the "dropped" flag), neither send unnecessary messages
        // just take off the aura
        if (player->GetTeam() == ALLIANCE)
        {
            if (!IsHordeFlagPickedup())
                return;

            if (GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
            {
                SetHordeFlagPicker(ObjectGuid::Empty);
                player->RemoveAurasDueToSpell(BG_WS_SPELL_WARSONG_FLAG);
            }
        }
        else
        {
            if (!IsAllianceFlagPickedup())
                return;

            if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
            {
                SetAllianceFlagPicker(ObjectGuid::Empty);
                player->RemoveAurasDueToSpell(BG_WS_SPELL_SILVERWING_FLAG);
            }
        }
        return;
    }

    bool set = false;

    if (player->GetTeam() == ALLIANCE)
    {
        if (!IsHordeFlagPickedup())
            return;
        if (GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
        {
            SetHordeFlagPicker(ObjectGuid::Empty);
            player->RemoveAurasDueToSpell(BG_WS_SPELL_WARSONG_FLAG);
            if (_flagDebuffState == 1)
              player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
            else if (_flagDebuffState == 2)
              player->RemoveAurasDueToSpell(WS_SPELL_BRUTAL_ASSAULT);
            _flagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_GROUND;
            player->CastSpell(player, BG_WS_SPELL_WARSONG_FLAG_DROPPED, true);
            set = true;
        }
    }
    else
    {
        if (!IsAllianceFlagPickedup())
            return;
        if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
        {
            SetAllianceFlagPicker(ObjectGuid::Empty);
            player->RemoveAurasDueToSpell(BG_WS_SPELL_SILVERWING_FLAG);
            if (_flagDebuffState == 1)
              player->RemoveAurasDueToSpell(WS_SPELL_FOCUSED_ASSAULT);
            else if (_flagDebuffState == 2)
              player->RemoveAurasDueToSpell(WS_SPELL_BRUTAL_ASSAULT);
            _flagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_GROUND;
            player->CastSpell(player, BG_WS_SPELL_SILVERWING_FLAG_DROPPED, true);
            set = true;
        }
    }

    if (set)
    {
        player->CastSpell(player, SPELL_RECENTLY_DROPPED_FLAG, true);
        UpdateFlagState(player->GetTeam(), 1);

        if (player->GetTeam() == ALLIANCE)
        {
            SendBroadcastText(BG_WS_TEXT_HORDE_FLAG_DROPPED, CHAT_MSG_BG_SYSTEM_HORDE, player);
            UpdateWorldState(BG_WS_FLAG_UNK_HORDE, uint32(-1));
        }
        else
        {
            SendBroadcastText(BG_WS_TEXT_ALLIANCE_FLAG_DROPPED, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
            UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, uint32(-1));
        }

        _flagsDropTimer[GetTeamIndexByTeamId(player->GetTeam()) ? 0 : 1] = BG_WS_FLAG_DROP_TIME;
    }
}

void BattlegroundWS::EventPlayerClickedOnFlag(Player* player, GameObject* target_obj)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    //alliance flag picked up from base
    if (player->GetTeam() == HORDE && GetFlagState(ALLIANCE) == BG_WS_FLAG_STATE_ON_BASE)
    {
        SendBroadcastText(BG_WS_TEXT_ALLIANCE_FLAG_PICKED_UP, CHAT_MSG_BG_SYSTEM_HORDE, player);
        PlaySoundToAll(BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_ONE_DAY);
        SetAllianceFlagPicker(player->GetGUID());
        _flagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_PLAYER;
        //update world state to show correct flag carrier
        UpdateFlagState(HORDE, BG_WS_FLAG_STATE_ON_PLAYER);
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
        player->CastSpell(player, BG_WS_SPELL_SILVERWING_FLAG, true);
        player->StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_SPELL_TARGET, BG_WS_SPELL_SILVERWING_FLAG_PICKED);
        if (_flagState[1] == BG_WS_FLAG_STATE_ON_PLAYER)
          _bothFlagsKept = true;

        if (_flagDebuffState == 1)
          player->CastSpell(player, WS_SPELL_FOCUSED_ASSAULT, true);
        else if (_flagDebuffState == 2)
          player->CastSpell(player, WS_SPELL_BRUTAL_ASSAULT, true);
    }

    //horde flag picked up from base
    if (player->GetTeam() == ALLIANCE && GetFlagState(HORDE) == BG_WS_FLAG_STATE_ON_BASE)
    {
        SendBroadcastText(BG_WS_TEXT_HORDE_FLAG_PICKED_UP, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
        PlaySoundToAll(BG_WS_SOUND_HORDE_FLAG_PICKED_UP);
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_ONE_DAY);
        SetHordeFlagPicker(player->GetGUID());
        _flagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_PLAYER;
        //update world state to show correct flag carrier
        UpdateFlagState(ALLIANCE, BG_WS_FLAG_STATE_ON_PLAYER);
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
        player->CastSpell(player, BG_WS_SPELL_WARSONG_FLAG, true);
        player->StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_SPELL_TARGET, BG_WS_SPELL_WARSONG_FLAG_PICKED);
        if (_flagState[0] == BG_WS_FLAG_STATE_ON_PLAYER)
          _bothFlagsKept = true;

        if (_flagDebuffState == 1)
          player->CastSpell(player, WS_SPELL_FOCUSED_ASSAULT, true);
        else if (_flagDebuffState == 2)
          player->CastSpell(player, WS_SPELL_BRUTAL_ASSAULT, true);
    }

    //Alliance flag on ground(not in base) (returned or picked up again from ground!)
    if (GetFlagState(ALLIANCE) == BG_WS_FLAG_STATE_ON_GROUND && player->IsWithinDistInMap(target_obj, 10)
        && target_obj->GetGOInfo()->entry == BG_OBJECT_A_FLAG_GROUND_WS_ENTRY)
    {
        if (player->GetTeam() == ALLIANCE)
        {
            SendBroadcastText(BG_WS_TEXT_ALLIANCE_FLAG_RETURNED, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
            UpdateFlagState(HORDE, BG_WS_FLAG_STATE_WAIT_RESPAWN);
            RespawnFlag(ALLIANCE, false);
            SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
            PlaySoundToAll(BG_WS_SOUND_FLAG_RETURNED);
            UpdatePlayerScore(player, SCORE_FLAG_RETURNS, 1);
            _bothFlagsKept = false;

            HandleFlagRoomCapturePoint(TEAM_HORDE); // Check Horde flag if it is in capture zone; if so, capture it
        }
        else
        {
            SendBroadcastText(BG_WS_TEXT_ALLIANCE_FLAG_PICKED_UP, CHAT_MSG_BG_SYSTEM_HORDE, player);
            PlaySoundToAll(BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP);
            SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_ONE_DAY);
            SetAllianceFlagPicker(player->GetGUID());
            player->CastSpell(player, BG_WS_SPELL_SILVERWING_FLAG, true);
            _flagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_PLAYER;
            UpdateFlagState(HORDE, BG_WS_FLAG_STATE_ON_PLAYER);
            if (_flagDebuffState == 1)
              player->CastSpell(player, WS_SPELL_FOCUSED_ASSAULT, true);
            else if (_flagDebuffState == 2)
              player->CastSpell(player, WS_SPELL_BRUTAL_ASSAULT, true);
            UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
        }
        //called in HandleGameObjectUseOpcode:
        //target_obj->Delete();
    }

    //Horde flag on ground(not in base) (returned or picked up again)
    if (GetFlagState(HORDE) == BG_WS_FLAG_STATE_ON_GROUND && player->IsWithinDistInMap(target_obj, 10)
        && target_obj->GetGOInfo()->entry == BG_OBJECT_H_FLAG_GROUND_WS_ENTRY)
    {
        if (player->GetTeam() == HORDE)
        {
            SendBroadcastText(BG_WS_TEXT_HORDE_FLAG_RETURNED, CHAT_MSG_BG_SYSTEM_HORDE, player);
            UpdateFlagState(ALLIANCE, BG_WS_FLAG_STATE_WAIT_RESPAWN);
            RespawnFlag(HORDE, false);
            SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
            PlaySoundToAll(BG_WS_SOUND_FLAG_RETURNED);
            UpdatePlayerScore(player, SCORE_FLAG_RETURNS, 1);
            _bothFlagsKept = false;

            HandleFlagRoomCapturePoint(TEAM_ALLIANCE); // Check Alliance flag if it is in capture zone; if so, capture it
        }
        else
        {
            SendBroadcastText(BG_WS_TEXT_HORDE_FLAG_PICKED_UP, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
            PlaySoundToAll(BG_WS_SOUND_HORDE_FLAG_PICKED_UP);
            SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_ONE_DAY);
            SetHordeFlagPicker(player->GetGUID());
            player->CastSpell(player, BG_WS_SPELL_WARSONG_FLAG, true);
            _flagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_PLAYER;
            UpdateFlagState(ALLIANCE, BG_WS_FLAG_STATE_ON_PLAYER);
            if (_flagDebuffState == 1)
              player->CastSpell(player, WS_SPELL_FOCUSED_ASSAULT, true);
            else if (_flagDebuffState == 2)
              player->CastSpell(player, WS_SPELL_BRUTAL_ASSAULT, true);
            UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
        }
        //called in HandleGameObjectUseOpcode:
        //target_obj->Delete();
    }

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
}

void BattlegroundWS::RemovePlayer(Player* player, ObjectGuid guid, uint32 /*team*/)
{
    // sometimes flag aura not removed :(
    if (IsAllianceFlagPickedup() && m_FlagKeepers[TEAM_ALLIANCE] == guid)
    {
        if (!player)
        {
            FMT_LOG_ERROR("bg.battleground", "BattlegroundWS: Removing offline player who has the FLAG!!");
            SetAllianceFlagPicker(ObjectGuid::Empty);
            RespawnFlag(ALLIANCE, false);
        }
        else
            EventPlayerDroppedFlag(player);
    }
    if (IsHordeFlagPickedup() && m_FlagKeepers[TEAM_HORDE] == guid)
    {
        if (!player)
        {
            FMT_LOG_ERROR("bg.battleground", "BattlegroundWS: Removing offline player who has the FLAG!!");
            SetHordeFlagPicker(ObjectGuid::Empty);
            RespawnFlag(HORDE, false);
        }
        else
            EventPlayerDroppedFlag(player);
    }
}

void BattlegroundWS::UpdateFlagState(uint32 team, uint32 value)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, value);
    else
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, value);
}

void BattlegroundWS::UpdateTeamScore(uint32 team)
{
    if (team == TEAM_ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_CAPTURES_ALLIANCE, GetTeamScore(team));
    else
        UpdateWorldState(BG_WS_FLAG_CAPTURES_HORDE, GetTeamScore(team));
}

void BattlegroundWS::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    //uint32 SpellId = 0;
    //uint64 buff_guid = 0;
    switch (trigger)
    {
        case 3686:                                          // Alliance elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in Battleground::Update().
            //buff_guid = BgObjects[BG_WS_OBJECT_SPEEDBUFF_1];
            break;
        case 3687:                                          // Horde elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in Battleground::Update().
            //buff_guid = BgObjects[BG_WS_OBJECT_SPEEDBUFF_2];
            break;
        case 3706:                                          // Alliance elixir of regeneration spawn
            //buff_guid = BgObjects[BG_WS_OBJECT_REGENBUFF_1];
            break;
        case 3708:                                          // Horde elixir of regeneration spawn
            //buff_guid = BgObjects[BG_WS_OBJECT_REGENBUFF_2];
            break;
        case 3707:                                          // Alliance elixir of berserk spawn
            //buff_guid = BgObjects[BG_WS_OBJECT_BERSERKBUFF_1];
            break;
        case 3709:                                          // Horde elixir of berserk spawn
            //buff_guid = BgObjects[BG_WS_OBJECT_BERSERKBUFF_2];
            break;
        case 3646:                                          // Alliance Flag spawn
            if (_flagState[TEAM_HORDE] && !_flagState[TEAM_ALLIANCE])
                if (GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
                    EventPlayerCapturedFlag(player);
            break;
        case 3647:                                          // Horde Flag spawn
            if (_flagState[TEAM_ALLIANCE] && !_flagState[TEAM_HORDE])
                if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
                    EventPlayerCapturedFlag(player);
            break;
        case 3649:                                          // unk1
        case 3688:                                          // unk2
        case 4628:                                          // unk3
        case 4629:                                          // unk4
            break;
        default:
            Battleground::HandleAreaTrigger(player, trigger);
            break;
    }
}

bool BattlegroundWS::SetupBattleground()
{
    for (uint32 i = 0; i < BG_WS_OBJECT_MAX; ++i)
    {
        BattlegroundGOSpawnPoint const& object = BG_WSG_GameObjects[i];
        if (!AddObject(i, object.Entry, object.Pos, object.Rot, object.SpawnTime))
        {
            FMT_LOG_ERROR("bg.battleground", "BattlegroundWS: Failed to spawn some object (Entry: {}). Battleground not created!", object.Entry);
            return false;
        }
    }

    WorldSafeLocsDBC const* sg = sDBCStoresMgr->GetWorldSafeLocsDBC(WS_GRAVEYARD_MAIN_ALLIANCE);
    if (!sg || !AddSpiritGuide(WS_SPIRIT_MAIN_ALLIANCE, Position(sg->Loc.X, sg->Loc.Y, sg->Loc.Z, 3.124139f), TEAM_ALLIANCE))
    {
        FMT_LOG_ERROR("bg.battleground", "BattlegroundWS: Failed to spawn Alliance spirit guide! Battleground not created!");
        return false;
    }

    sg = sDBCStoresMgr->GetWorldSafeLocsDBC(WS_GRAVEYARD_MAIN_HORDE);
    if (!sg || !AddSpiritGuide(WS_SPIRIT_MAIN_HORDE, Position(sg->Loc.X, sg->Loc.Y, sg->Loc.Z, 3.193953f), TEAM_HORDE))
    {
        FMT_LOG_ERROR("bg.battleground", "BattlegroundWS: Failed to spawn Horde spirit guide! Battleground not created!");
        return false;
    }

    FMT_LOG_DEBUG("bg.battleground", "BattlegroundWS: BG objects and spirit guides spawned");
    return true;
}

void BattlegroundWS::Reset()
{
    //call parent's class reset
    Battleground::Reset();

    m_FlagKeepers[TEAM_ALLIANCE].Clear();
    m_FlagKeepers[TEAM_HORDE].Clear();
    m_DroppedFlagGUID[TEAM_ALLIANCE].Clear();
    m_DroppedFlagGUID[TEAM_HORDE].Clear();
    _flagState[TEAM_ALLIANCE]        = BG_WS_FLAG_STATE_ON_BASE;
    _flagState[TEAM_HORDE]           = BG_WS_FLAG_STATE_ON_BASE;
    m_TeamScores[TEAM_ALLIANCE]      = 0;
    m_TeamScores[TEAM_HORDE]         = 0;

    if (sBattlegroundMgr->IsBGWeekend(GetTypeID()))
    {
        m_ReputationCapture = 45;
        m_HonorWinKills = 3;
        m_HonorEndKills = 4;
    }
    else
    {
        m_ReputationCapture = 35;
        m_HonorWinKills = 1;
        m_HonorEndKills = 2;
    }
    _minutesElapsed                  = 0;
    _lastFlagCaptureTeam             = 0;
    _bothFlagsKept                   = false;
    _flagDebuffState                 = 0;
    _flagSpellForceTimer             = 0;
    _flagsDropTimer[TEAM_ALLIANCE]   = 0;
    _flagsDropTimer[TEAM_HORDE]      = 0;
    _flagsTimer[TEAM_ALLIANCE]       = 0;
    _flagsTimer[TEAM_HORDE]          = 0;
}

void BattlegroundWS::EndBattleground(uint32 winner)
{
    // Win reward
    if (winner == ALLIANCE)
        RewardHonorToTeam(GetBonusHonorFromKill(m_HonorWinKills), ALLIANCE);
    if (winner == HORDE)
        RewardHonorToTeam(GetBonusHonorFromKill(m_HonorWinKills), HORDE);
    // Complete map_end rewards (even if no team wins)
    RewardHonorToTeam(GetBonusHonorFromKill(m_HonorEndKills), ALLIANCE);
    RewardHonorToTeam(GetBonusHonorFromKill(m_HonorEndKills), HORDE);

    Battleground::EndBattleground(winner);
}

void BattlegroundWS::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);

    Battleground::HandleKillPlayer(player, killer);
}

bool BattlegroundWS::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, WS_OBJECTIVE_CAPTURE_FLAG);
            break;
        case SCORE_FLAG_RETURNS:                            // flags returned
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, WS_OBJECTIVE_RETURN_FLAG);
            break;
        default:
            break;
    }
    return true;
}

WorldSafeLocsDBC const* BattlegroundWS::GetClosestGraveyard(Player* player)
{
    //if status in progress, it returns main graveyards with spiritguides
    //else it will return the graveyard in the flagroom - this is especially good
    //if a player dies in preparation phase - then the player can't cheat
    //and teleport to the graveyard outside the flagroom
    //and start running around, while the doors are still closed
    if (player->GetTeam() == ALLIANCE)
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sDBCStoresMgr->GetWorldSafeLocsDBC(WS_GRAVEYARD_MAIN_ALLIANCE);
        else
            return sDBCStoresMgr->GetWorldSafeLocsDBC(WS_GRAVEYARD_FLAGROOM_ALLIANCE);
    }
    else
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sDBCStoresMgr->GetWorldSafeLocsDBC(WS_GRAVEYARD_MAIN_HORDE);
        else
            return sDBCStoresMgr->GetWorldSafeLocsDBC(WS_GRAVEYARD_FLAGROOM_HORDE);
    }
}

void BattlegroundWS::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(BG_WS_FLAG_CAPTURES_ALLIANCE, GetTeamScore(TEAM_ALLIANCE));
    packet.Worldstates.emplace_back(BG_WS_FLAG_CAPTURES_HORDE, GetTeamScore(TEAM_HORDE));

    if (_flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_GROUND)
        packet.Worldstates.emplace_back(BG_WS_FLAG_UNK_ALLIANCE, uint32(-1)); // ??
    else if (_flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
        packet.Worldstates.emplace_back(BG_WS_FLAG_UNK_ALLIANCE, 1);
    else
        packet.Worldstates.emplace_back(BG_WS_FLAG_UNK_ALLIANCE, 0);

    if (_flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_GROUND)
        packet.Worldstates.emplace_back(BG_WS_FLAG_UNK_HORDE, uint32(-1)); // ??
    else if (_flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
        packet.Worldstates.emplace_back(BG_WS_FLAG_UNK_HORDE, 1);
    else
        packet.Worldstates.emplace_back(BG_WS_FLAG_UNK_HORDE, 0);

    packet.Worldstates.emplace_back(BG_WS_FLAG_CAPTURES_MAX, BG_WS_MAX_TEAM_SCORE);

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        packet.Worldstates.emplace_back(BG_WS_STATE_TIMER_ACTIVE, 1);
        packet.Worldstates.emplace_back(BG_WS_STATE_TIMER, 25 - _minutesElapsed);
    }
    else
        packet.Worldstates.emplace_back(BG_WS_STATE_TIMER_ACTIVE, 0);

    if (_flagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
        packet.Worldstates.emplace_back(BG_WS_FLAG_STATE_HORDE, 2);
    else
        packet.Worldstates.emplace_back(BG_WS_FLAG_STATE_HORDE, 1);

    if (_flagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
        packet.Worldstates.emplace_back(BG_WS_FLAG_STATE_ALLIANCE, 2);
    else
        packet.Worldstates.emplace_back(BG_WS_FLAG_STATE_ALLIANCE, 1);
}

uint32 BattlegroundWS::GetPrematureWinner()
{
    if (GetTeamScore(TEAM_ALLIANCE) > GetTeamScore(TEAM_HORDE))
        return ALLIANCE;
    else if (GetTeamScore(TEAM_HORDE) > GetTeamScore(TEAM_ALLIANCE))
        return HORDE;

    return Battleground::GetPrematureWinner();
}

bool BattlegroundWS::CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* player, Unit const* target, uint32 miscValue)
{
    switch (criteriaId)
    {
        case BG_CRITERIA_CHECK_SAVE_THE_DAY:
            if (target)
                if (Player const* playerTarget = target->ToPlayer())
                    return GetFlagState(playerTarget->GetTeam()) == BG_WS_FLAG_STATE_ON_BASE;
            return false;
    }

    return Battleground::CheckAchievementCriteriaMeet(criteriaId, player, target, miscValue);
}
