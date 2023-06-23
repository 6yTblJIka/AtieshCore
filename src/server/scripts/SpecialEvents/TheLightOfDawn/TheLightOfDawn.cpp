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

#include "CreatureAI.h"
#include "Chat.h"
#include "Creature.h"
#include "DBCStoresMgr.h"
#include "TheLightOfDawn.h"
#include "Group.h"
#include "Guild.h"
#include "Unit.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpecialEventMgr.h"
#include "ScriptMgr.h"
#include "RBAC.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MotionMaster.h"
#include "TemporarySummon.h"
#include "Language.h"
#include "Log.h"
#include "WorldSession.h"
#include "World.h"

enum QuestEvents
{
    // Intro Events
    EVENT_START_PREPARE                 = 1,
    EVENT_START_COUNTDOWN_1,
    EVENT_START_COUNTDOWN_2,
    EVENT_START_COUNTDOWN_3,
    EVENT_START_COUNTDOWN_4,
    EVENT_START_COUNTDOWN_5,

    EVENT_BEFORE_FIGHT_RISE,
    EVENT_BEFORE_FIGHT_PREVIOUS_LIFE,
    EVENT_BEFORE_FIGHT_RED_SKY,
    EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_1,
    EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_2,
    EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_3,
    EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_4,
    EVENT_BEFORE_FIGHT_START_MOVE_FOR_MONSTERS,
    EVENT_BEFORE_FIGHT_START_MOVE,

    // Fight Events
    EVENT_SPELL_ANTI_MAGIC_ZONE,
    EVENT_SPELL_DEATH_STRIKE,
    EVENT_SPELL_DEATH_EMBRACE,
    EVENT_SPELL_UNHOLY_BLIGHT,
    EVENT_SPELL_TALK,
    // Positioning
    EVENT_FINISH_FIGHT_1,
    EVENT_FINISH_FIGHT_2,
    EVENT_FINISH_FIGHT_3,
    EVENT_FINISH_FIGHT_4,
    EVENT_FINISH_FIGHT_5,
    // Outro
    EVENT_OUTRO_SCENE_1,
    EVENT_OUTRO_SCENE_2,
    EVENT_OUTRO_SCENE_3,
    EVENT_OUTRO_SCENE_4,
    EVENT_OUTRO_SCENE_5,
    EVENT_OUTRO_SCENE_6,
    EVENT_OUTRO_SCENE_7,
    EVENT_OUTRO_SCENE_8,
    EVENT_OUTRO_SCENE_9,
    EVENT_OUTRO_SCENE_10,
    EVENT_OUTRO_SCENE_11,
    EVENT_OUTRO_SCENE_12,
    EVENT_OUTRO_SCENE_13,
    EVENT_OUTRO_SCENE_14,
    EVENT_OUTRO_SCENE_15,
    EVENT_OUTRO_SCENE_16,
    EVENT_OUTRO_SCENE_17,
    EVENT_OUTRO_SCENE_18,
    EVENT_OUTRO_SCENE_19,
    EVENT_OUTRO_SCENE_20,
    EVENT_OUTRO_SCENE_21,
    EVENT_OUTRO_SCENE_22,
    EVENT_OUTRO_SCENE_23,
    EVENT_OUTRO_SCENE_24,
    EVENT_OUTRO_SCENE_25,
    EVENT_OUTRO_SCENE_26,
    EVENT_OUTRO_SCENE_27,
    EVENT_OUTRO_SCENE_28,
    EVENT_OUTRO_SCENE_29,
    EVENT_OUTRO_SCENE_30,
    EVENT_OUTRO_SCENE_31,
    EVENT_OUTRO_SCENE_32,
    EVENT_OUTRO_SCENE_33,
    EVENT_OUTRO_SCENE_34,
    EVENT_OUTRO_SCENE_35,
    EVENT_OUTRO_SCENE_36,
    EVENT_OUTRO_SCENE_37,
    EVENT_OUTRO_SCENE_38,
    EVENT_OUTRO_SCENE_39,
    EVENT_OUTRO_SCENE_40,
    EVENT_OUTRO_SCENE_41,
    EVENT_OUTRO_SCENE_42,
    EVENT_OUTRO_SCENE_43,
    EVENT_OUTRO_SCENE_44,
    EVENT_OUTRO_SCENE_45,
    EVENT_OUTRO_SCENE_46,
    EVENT_OUTRO_SCENE_47,
    EVENT_OUTRO_SCENE_48,
    EVENT_OUTRO_SCENE_49,
    EVENT_OUTRO_SCENE_50,
    EVENT_OUTRO_SCENE_51,
    EVENT_OUTRO_SCENE_52,
    EVENT_OUTRO_SCENE_53,
    EVENT_OUTRO_SCENE_54,
    EVENT_OUTRO_SCENE_55,
    EVENT_OUTRO_SCENE_56,
    EVENT_OUTRO_SCENE_57,
    EVENT_OUTRO_SCENE_58,
    EVENT_OUTRO_SCENE_59,
    EVENT_OUTRO_SCENE_60,
    EVENT_OUTRO_SCENE_61,
    EVENT_DESPAWN_ALL_NPC_AND_RESET
};

uint8 const MAX_START_GUARDS = 12;
Position const GuardLoc[MAX_START_GUARDS] =
{
    {2240.47f, -5303.68f, 82.1685f, 0.852276f},     // 0
    {2244.83f, -5298.7f, 82.1664f, 0.852276f},      // 1
    {2248.61f, -5294.37f, 82.1664f, 0.867984f},     // 2
    {2254.22f, -5288.19f, 82.2303f, 0.950859f},     // 3
    {2258.51f, -5282.07f, 82.0794f, 0.856611f},     // 4
    {2263.19f, -5276.79f, 81.7646f, 0.825195f},     // 5
    {2267.69f, -5272.04f, 81.2963f, 0.876255f},     // 6
    {2298.07f, -5276.43f, 81.7732f, 1.11191f},      // 7
    {2293.26f, -5272.92f, 81.8162f, 1.10798f},      // 8
    {2288.93f, -5270.41f, 81.8256f, 0.939123f},     // 9
    {2284.63f, -5267.3f, 81.6772f, 0.929698f},      // 10
    {2280.99f, -5264.41f, 81.287f, 0.733344f}       // 11
};

void TheLightOfDawnEvent::UpdateWorldState(uint32 id, uint32 state)
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            player->SendUpdateWorldState(id, state);
    }
}

void TheLightOfDawnEvent::UpdateAllWorldStates()
{
    // update world states to default
    UpdateWorldState(WORLD_STATE_FORCES_SHOW, show_soldiers_count);
    UpdateWorldState(WORLD_STATE_FORCES_LIGHT, defendersRemaining);
    UpdateWorldState(WORLD_STATE_FORCES_SCOURGE, scourgeRemaining);
    UpdateWorldState(WORLD_STATE_BATTLE_TIMER_SHOW, show_timer);
    UpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);
    UpdateWorldState(WORLD_STATE_BATTLE_BEGIN, show_event_begin);
}

void TheLightOfDawnEvent::SetupInitialStates()
{
    events.Reset();
    defendersRemaining      = ENCOUNTER_TOTAL_DEFENDERS;
    scourgeRemaining        = ENCOUNTER_TOTAL_SCOURGE;
    countdownTimerRemaining = 0;
    show_soldiers_count     = true;
    show_timer              = false;
    show_event_begin        = false;

    UpdateAllWorldStates();

    // return main npc at start pos
    if (Creature* temp = GetCreature(Darion_Mograine))
        temp->Respawn();
    if (Creature* temp = GetCreature(KoltiraGUID))
        temp->Respawn();
    if (Creature* temp = GetCreature(OrbazGUID))
        temp->Respawn();
    if (Creature* temp = GetCreature(ThassarianGUID))
        temp->Respawn();
}

void TheLightOfDawnEvent::DoPlaySoundToAll(uint32 soundId)
{
    // We should send data for players in area (only)
    BroadcastPacketToPlayersInArea(WorldPackets::Misc::PlaySound(soundId).Write());
}

void TheLightOfDawnEvent::BroadcastPacketToPlayersInArea(WorldPacket const* data) const
{
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (itr->second.isInArea)
            if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
                player->SendDirectMessage(data);
    }
}

void TheLightOfDawnEvent::StartTheLightOfDawnEvent()
{
    SetupInitialStates();
    events.ScheduleEvent(EVENT_START_PREPARE, 0s);
}

void TheLightOfDawnEvent::StopTheLightOfDawnEvent()
{
    // Complete condition for quest
    for (PlayersDataContainer::const_iterator itr = m_playersDataStore.begin(); itr != m_playersDataStore.end(); ++itr)
    {
        if (itr->second.isInArea && itr->second.isDoingQuest)
            if (Player* player = ObjectAccessor::FindConnectedPlayer(itr->first))
                player->KilledMonsterCredit(NPC_LIGHT_OF_DAWN_CREDIT);
    }

    // after 4 minutes npc will be despawned
    events.Reset();
    events.ScheduleEvent(EVENT_DESPAWN_ALL_NPC_AND_RESET, 240s);
}

void TheLightOfDawnEvent::SummonScorgeArmy()
{
    events.ScheduleEvent(EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_1, 500ms);
    events.ScheduleEvent(EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_2, 1s);
    events.ScheduleEvent(EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_3, 1500ms);
    events.ScheduleEvent(EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_4, 2s);
}

void TheLightOfDawnEvent::SummonDefenseArmy()
{
    if (Creature* Darion = GetCreature(Darion_Mograine))
    {
        // spawn Nicholas Zverenhoff
        Position spawnposN = { 2274.030518f, -5260.623047f, 79.698318f, 0.73f };
        if (Unit* temp = Darion->SummonCreature(NPC_DUKE_NICHOLAS_ZVERENHOFF, spawnposN.GetPositionX(), spawnposN.GetPositionY(), spawnposN.GetPositionZ(), spawnposN.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            ZverenhoffGUID = temp->GetGUID();

        // spawn Rimblat Earthshatter
        Position spawnposR = { 2300.981201f, -5285.847168f, 81.958527f, 1.48f };
        if (Unit* temp = Darion->SummonCreature(NPC_RIMBLAT_EARTHSHATTER, spawnposR.GetPositionX(), spawnposR.GetPositionY(), spawnposR.GetPositionZ(), spawnposR.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            RimblatGUID = temp->GetGUID();

        // spawn Rayne
        Position spawnposRa = { 2249.166992f, -5311.841797f, 82.167702f, 2.1f };
        if (Unit* temp = Darion->SummonCreature(NPC_RAYNE, spawnposRa.GetPositionX(), spawnposRa.GetPositionY(), spawnposRa.GetPositionZ(), spawnposRa.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            RayneGUID = temp->GetGUID();

        // spawn Korfax
        Position spawnposKo = { 2283.340820f, -5304.039551f, 86.036758f, 1.13f };
        if (Unit* temp = Darion->SummonCreature(NPC_KORFAX_CHAMPION_OF_THE_LIGHT, spawnposKo.GetPositionX(), spawnposKo.GetPositionY(), spawnposKo.GetPositionZ(), spawnposKo.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            KorfaxGUID = temp->GetGUID();

        // spawn Maxwell Tyrosus
        Position spawnposMa = { 2280.513184f, -5299.560547f, 84.962051f, 1.f };
        if (Unit* temp = Darion->SummonCreature(NPC_LORD_MAXWELL_TYROSUS, spawnposMa.GetPositionX(), spawnposMa.GetPositionY(), spawnposMa.GetPositionZ(), spawnposMa.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            MaxwellGUID = temp->GetGUID();

        // spawn Eligor
        Position spawnposEl = { 2277.664307f, -5294.874512f, 83.866936f, 0.98f };
        if (Unit* temp = Darion->SummonCreature(NPC_COMMANDER_ELIGOR_DAWNBRINGER, spawnposEl.GetPositionX(), spawnposEl.GetPositionY(), spawnposEl.GetPositionZ(), spawnposEl.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            EligorGUID = temp->GetGUID();

        // spawn Leonid Barthalomew
        Position spawnposLe = { 2282.226074f, -5317.552246f, 88.573433f, 1.48f };
        if (Unit* temp = Darion->SummonCreature(NPC_LEONID_BARTHALOMEW_THE_REVERED, spawnposLe.GetPositionX(), spawnposLe.GetPositionY(), spawnposLe.GetPositionZ(), spawnposLe.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            LeonidGUID = temp->GetGUID();

        for (uint8 i = 0; i < MAX_START_GUARDS; i++)
        {
            if (Unit* temp = Darion->SummonCreature(NPC_DEFENDER_OF_THE_LIGHT, GuardLoc[i].GetPositionX(), GuardLoc[i].GetPositionY(), GuardLoc[i].GetPositionZ(), GuardLoc[i].GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600s))
                DefenderGUID.insert(temp->GetGUID());
        }
    }
}

void TheLightOfDawnEvent::ClearAllNPC()
{
    // Light
    for (ObjectGuid guid : DefenderGUID)
        if (Creature* temp = GetCreature(guid))
            temp->setDeathState(JUST_DIED);
    DefenderGUID.clear();

    // Scourge
    for (ObjectGuid guid : AbominationGUID)
        if (Creature* temp = GetCreature(guid))
            temp->setDeathState(JUST_DIED);
    AbominationGUID.clear();

    for (ObjectGuid guid : BehemothGUID)
        if (Creature* temp = GetCreature(guid))
            temp->setDeathState(JUST_DIED);
    BehemothGUID.clear();

    for (ObjectGuid guid : GhoulGUID)
        if (Creature* temp = GetCreature(guid))
            temp->setDeathState(JUST_DIED);
    GhoulGUID.clear();

    for (ObjectGuid guid : WarriorGUID)
        if (Creature* temp = GetCreature(guid))
            temp->setDeathState(JUST_DIED);
    WarriorGUID.clear();

    // defender temp
    if (Creature* temp = GetCreature(TirionGUID))
        temp->setDeathState(JUST_DIED);
    TirionGUID.Clear();
    if (Creature* temp = GetCreature(AlexandrosGUID))
        temp->setDeathState(JUST_DIED);
    AlexandrosGUID.Clear();
    if (Creature* temp = GetCreature(DarionGUID))
        temp->setDeathState(JUST_DIED);
    DarionGUID.Clear();

    // scorge temp
    if (Creature* temp = GetCreature(LichKingGUID))
        temp->setDeathState(JUST_DIED);
    LichKingGUID.Clear();

    // despawn NPC with TEMPSUMMON_MANUAL_DESPAWN type
    if (Creature* temp = GetCreature(ZverenhoffGUID))
        temp->DespawnOrUnsummon();
    ZverenhoffGUID.Clear();
    if (Creature* temp = GetCreature(RimblatGUID))
        temp->DespawnOrUnsummon();
    RimblatGUID.Clear();
    if (Creature* temp = GetCreature(RayneGUID))
        temp->DespawnOrUnsummon();
    RayneGUID.Clear();
    if (Creature* temp = GetCreature(KorfaxGUID))
        temp->DespawnOrUnsummon();
    KorfaxGUID.Clear();
    if (Creature* temp = GetCreature(MaxwellGUID))
        temp->DespawnOrUnsummon();
    MaxwellGUID.Clear();
    if (Creature* temp = GetCreature(EligorGUID))
        temp->DespawnOrUnsummon();
    EligorGUID.Clear();
    if (Creature* temp = GetCreature(LeonidGUID))
        temp->DespawnOrUnsummon();
    LeonidGUID.Clear();
}

TheLightOfDawnEvent::TheLightOfDawnEvent()
{
    _eventId = SPECIALEVENT_EVENTID_THELIGHTOFDAWN;
    SetPhase(PHASE_IDLE);

    //prepareTimer.Reset(IN_MILLISECONDS);
}

bool TheLightOfDawnEvent::SetupSpecialEvent(bool enabled, bool active, bool repeatable, uint32 id, uint32 cooldownTimer, uint32 durationTimer, std::string comment)
{
    // override Setup for Set time of first run in cooldownTimer
    if (!SpecialEvent::SetupSpecialEvent(enabled, active, repeatable, id, cooldownTimer, durationTimer, comment))
        return false;

    _mapId = MAPID_THE_SCARLET_ENCLAVE;
    _map = sMapMgr->FindMap(_mapId, 0);
    if (!_map)
        _map = sMapMgr->CreateBaseMap(_mapId);

    RegisterZoneIdForEvent(ZONEID_THE_SCARLET_ENCLAVE);
    SetupInitialStates();
    return true;
}

void TheLightOfDawnEvent::DoActionForMember(ObjectGuid playerGUID, uint32 param)
{
    PlayersDataContainer::iterator itr = m_playersDataStore.find(playerGUID);
    if (itr != m_playersDataStore.end())
    {
        if (!param) // if param == 0, we should set true isDoingQuest
            itr->second.isDoingQuest = true;
        else        // if param == 1, we should set false isDoingQuest
            itr->second.isDoingQuest = false;
    }
}

TheLightOfDawnEvent::~TheLightOfDawnEvent()
{
    m_playersDataStore.clear();
    ClearAllNPC();
    Darion_Mograine.Clear();
    KoltiraGUID.Clear();
    OrbazGUID.Clear();
    ThassarianGUID.Clear();
}

void TheLightOfDawnEvent::Update(uint32 diff)
{
    SpecialEvent::Update(diff);

    events.Update(diff);

    while (uint32 eventId = events.ExecuteEvent())
    {
        switch (eventId)
        {
            case EVENT_START_PREPARE:
            {
                SetPhase(PHASE_INTRO);
                countdownTimerRemaining = ENCOUNTER_START_TIME;
                show_timer              = true;

                UpdateAllWorldStates();

                events.Reset();
                events.ScheduleEvent(EVENT_START_COUNTDOWN_1, 60s);
                events.ScheduleEvent(EVENT_START_COUNTDOWN_2, 120s);
                events.ScheduleEvent(EVENT_START_COUNTDOWN_3, 180s);
                events.ScheduleEvent(EVENT_START_COUNTDOWN_4, 240s);
                events.ScheduleEvent(EVENT_START_COUNTDOWN_5, 300s);
                SummonDefenseArmy();
                break;
            }
            case EVENT_START_COUNTDOWN_1:
            {
                --countdownTimerRemaining;  // 4 minutes
                UpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);
                break;
            }
            case EVENT_START_COUNTDOWN_2:
            {
                --countdownTimerRemaining;  // 3 minutes
                UpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);
                break;
            }
            case EVENT_START_COUNTDOWN_3:
            {
                --countdownTimerRemaining;  // 2 minutes
                UpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);
                break;
            }
            case EVENT_START_COUNTDOWN_4:
            {
                --countdownTimerRemaining;  // 1 minutes
                UpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);
                if (Creature* Darion = GetCreature(Darion_Mograine))
                    Darion->AI()->Talk(SAY_LIGHT_OF_DAWN02);    //The sky weeps at the glorious devastation of these lands! Soon, Azeroth's futile tears will rain down upon us!
                break;
            }
            case EVENT_START_COUNTDOWN_5:
            {
                --countdownTimerRemaining;  // 0 minutes
                UpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);

                show_timer       = false;
                show_event_begin = true;
                UpdateWorldState(WORLD_STATE_BATTLE_TIMER_SHOW, show_timer);
                UpdateWorldState(WORLD_STATE_BATTLE_BEGIN, show_event_begin);

                if (Creature* Darion = GetCreature(Darion_Mograine))
                    Darion->AI()->Talk(SAY_LIGHT_OF_DAWN04);    // Death knights of Acherus, the death march begins!

                events.ScheduleEvent(EVENT_BEFORE_FIGHT_RISE, 10s);
                break;
            }
            case EVENT_BEFORE_FIGHT_RISE:
            {
                if (Creature* Darion = GetCreature(Darion_Mograine))
                {
                    Darion->AI()->Talk(SAY_LIGHT_OF_DAWN05);    // RISE!
                    Darion->AI()->DoCast(SPELL_CAMERA_SHAKE_INIT);
                }

                SummonScorgeArmy();
                events.ScheduleEvent(EVENT_BEFORE_FIGHT_PREVIOUS_LIFE, 25s);
                break;
            }
            case EVENT_BEFORE_FIGHT_PREVIOUS_LIFE:
            {
                // only for Ru locale?
                // Need audio play (Nasha prejnyaa jisn bila bessmislenna)

                events.ScheduleEvent(EVENT_BEFORE_FIGHT_RED_SKY, 7s);
                break;
            }
            case EVENT_BEFORE_FIGHT_RED_SKY:
            {
                if (Creature* Darion = GetCreature(Darion_Mograine))
                    Darion->AI()->Talk(SAY_LIGHT_OF_DAWN06);    // The skies turn red with the blood of the fallen! The Lich King watches over us, minions! Leave only ashes and misery in your destructive wake!

                events.ScheduleEvent(EVENT_BEFORE_FIGHT_START_MOVE_FOR_MONSTERS, 3s);
                events.ScheduleEvent(EVENT_BEFORE_FIGHT_START_MOVE, 5s);
                break;
            }
            case EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_1:
            {
                while (AbominationGUID.size() < ENCOUNTER_ABOMINATION_NUMBER)
                    SummonAbomination();
                break;
            }
            case EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_2:
            {
                while (BehemothGUID.size() < ENCOUNTER_BEHEMOTH_NUMBER)
                    SummonBehemoth();
                break;
            }
            case EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_3:
            {
                while (GhoulGUID.size() < ENCOUNTER_GHOUL_NUMBER)
                    SummonGhoul();
                break;
            }
            case EVENT_BEFORE_FIGHT_SPAWN_SCOURGE_4:
            {
                while (WarriorGUID.size() < ENCOUNTER_WARRIOR_NUMBER)
                    SummonWarriorOfFrozenWastes();
                break;
            }
            case EVENT_BEFORE_FIGHT_START_MOVE_FOR_MONSTERS:
            {
                SetPhase(PHASE_FIGHT);

                Position attackPos = { 2249.166992f, -5311.841797f, 82.167702f, 2.1f }; // Rayne pos
                Creature* Rayne = GetCreature(RayneGUID);

                // monsters start move early then Morgraine
                for (ObjectGuid guid : AbominationGUID)
                    if (Creature* temp = GetCreature(guid))
                    {
                        if (Rayne)
                            attackPos = Rayne->GetRandomNearPosition(35.f);
                        temp->GetMotionMaster()->MovePoint(0, attackPos);
                        temp->SetHomePosition(attackPos);
                    }

                for (ObjectGuid guid : BehemothGUID)
                    if (Creature* temp = GetCreature(guid))
                    {
                        if (Rayne)
                            attackPos = Rayne->GetRandomNearPosition(35.f);
                        temp->GetMotionMaster()->MovePoint(0, attackPos);
                        temp->SetHomePosition(attackPos);
                    }

                for (ObjectGuid guid : GhoulGUID)
                    if (Creature* temp = GetCreature(guid))
                    {
                        if (Rayne)
                            attackPos = Rayne->GetRandomNearPosition(35.f);
                        temp->GetMotionMaster()->MovePoint(0, attackPos);
                        temp->SetHomePosition(attackPos);
                    }

                for (ObjectGuid guid : WarriorGUID)
                    if (Creature* temp = GetCreature(guid))
                    {
                        if (Rayne)
                            attackPos = Rayne->GetRandomNearPosition(35.f);
                        temp->GetMotionMaster()->MovePoint(0, attackPos);
                        temp->SetHomePosition(attackPos);
                    }

                break;
            }
            case EVENT_BEFORE_FIGHT_START_MOVE:
            {
                // Morgraine start moveing
                Position attackPos = { 2249.166992f, -5311.841797f, 82.167702f, 2.1f }; // Rayne pos
                Creature* Rayne = GetCreature(RayneGUID);

                if (Creature* temp = GetCreature(Darion_Mograine))
                {
                    if (Rayne)
                        attackPos = Rayne->GetRandomNearPosition(35.f);
                    temp->GetMotionMaster()->MovePoint(0, attackPos);
                }

                if (Creature* temp = GetCreature(KoltiraGUID))
                {
                    if (Rayne)
                        attackPos = Rayne->GetRandomNearPosition(35.f);
                    temp->GetMotionMaster()->MovePoint(0, attackPos);
                }

                if (Creature* temp = GetCreature(OrbazGUID))
                {
                    if (Rayne)
                        attackPos = Rayne->GetRandomNearPosition(35.f);
                    temp->GetMotionMaster()->MovePoint(0, attackPos);
                }

                if (Creature* temp = GetCreature(ThassarianGUID))
                {
                    if (Rayne)
                        attackPos = Rayne->GetRandomNearPosition(35.f);
                    temp->GetMotionMaster()->MovePoint(0, attackPos);
                }
                break;
            }
            case EVENT_DESPAWN_ALL_NPC_AND_RESET:
            {
                ClearAllNPC();
                SetupInitialStates();
                break;
            }
            default:
                break;
        }
    }
}

void TheLightOfDawnEvent::OnSpecialEventStart()
{
    StartTheLightOfDawnEvent();
}

void TheLightOfDawnEvent::OnSpecialEventEnd(bool /*endByTimer*/)
{
    StopTheLightOfDawnEvent();
}

void TheLightOfDawnEvent::AddPlayer(ObjectGuid playerGUID)
{
    PlayersData pd;
    pd.isDoingQuest = false;
    pd.isInArea     = false;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGUID))
    {
        if (player->GetQuestStatus(12801) == QUEST_STATUS_INCOMPLETE)
            pd.isDoingQuest = true;

        uint32 plrAreaid = player->GetAreaId();
        bool areaIdFounded = plrAreaid == 4298 || plrAreaid == 4361 || plrAreaid == 4364;
        if (areaIdFounded)
            pd.isInArea = true;
    }

    // add in active player list of event
    m_playersDataStore[playerGUID] = pd;
}

void TheLightOfDawnEvent::RemovePlayer(ObjectGuid playerGUID)
{
    bool founded = false;
    PlayersDataContainer::const_iterator itr = m_playersDataStore.find(playerGUID);
    if (itr != m_playersDataStore.end())
        founded = true;

    if (founded)
        m_playersDataStore.erase(playerGUID);
}

bool TheLightOfDawnEvent::IsMemberOfEvent(Player* player)
{
    PlayersDataContainer::const_iterator itr = m_playersDataStore.find(player->GetGUID());
    if (itr != m_playersDataStore.end())
        return true;

    return false;
}

void TheLightOfDawnEvent::HandlePlayerEnterZone(Player* player, uint32 zoneId)
{
    if (zoneId != ZONEID_THE_SCARLET_ENCLAVE)
        return;

    if (!IsMemberOfEvent(player))
        AddPlayer(player->GetGUID());

    player->SendUpdateWorldState(WORLD_STATE_FORCES_SHOW, show_soldiers_count);
    player->SendUpdateWorldState(WORLD_STATE_FORCES_LIGHT, defendersRemaining);
    player->SendUpdateWorldState(WORLD_STATE_FORCES_SCOURGE, scourgeRemaining);
    player->SendUpdateWorldState(WORLD_STATE_BATTLE_TIMER_SHOW, show_timer);
    player->SendUpdateWorldState(WORLD_STATE_BATTLE_TIMER, countdownTimerRemaining);
    player->SendUpdateWorldState(WORLD_STATE_BATTLE_BEGIN, show_event_begin);
}

void TheLightOfDawnEvent::HandlePlayerLeaveZone(Player* player, uint32 zoneId)
{
    if (zoneId != ZONEID_THE_SCARLET_ENCLAVE)
        return;

    if (IsMemberOfEvent(player))
        RemovePlayer(player->GetGUID());
}

void TheLightOfDawnEvent::HandlePlayerEnterArea(Player* player, uint32 zoneId, uint32 areaId)
{
    if (zoneId != ZONEID_THE_SCARLET_ENCLAVE)
        return;

    bool areaIdFounded = areaId == 4298 || areaId == 4361 || areaId == 4364;
    if (!areaIdFounded)
        return;

    PlayersDataContainer::iterator itr = m_playersDataStore.find(player->GetGUID());
    if (itr != m_playersDataStore.end())
    {
        if (!itr->second.isInArea)
            itr->second.isInArea = true;
    }
}

void TheLightOfDawnEvent::HandlePlayerLeaveArea(Player* player, uint32 zoneId, uint32 areaId)
{
    if (zoneId != ZONEID_THE_SCARLET_ENCLAVE)
        return;

    bool areaIdFounded = areaId == 4298 || areaId == 4361 || areaId == 4364;
    if (!areaIdFounded)
        return;

    PlayersDataContainer::iterator itr = m_playersDataStore.find(player->GetGUID());
    if (itr != m_playersDataStore.end())
    {
        if (itr->second.isInArea)
            itr->second.isInArea = false;
    }
}

Creature* TheLightOfDawnEvent::GetCreature(ObjectGuid guid)
{
    if (!_map)
        return nullptr;

    return _map->GetCreature(guid);
}

GameObject* TheLightOfDawnEvent::GetGameObject(ObjectGuid guid)
{
    if (!_map)
        return nullptr;

    return _map->GetGameObject(guid);
}

void TheLightOfDawnEvent::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_HIGHLORD_DARION_MOGRAINE: Darion_Mograine = creature->GetGUID(); break;
        case NPC_KOLTIRA_DEATHWEAVER: KoltiraGUID = creature->GetGUID(); break;
        case NPC_ORBAZ_BLOODBANE: OrbazGUID = creature->GetGUID(); break;
        case NPC_THASSARIAN: ThassarianGUID = creature->GetGUID(); break;
        default:
            break;
    }
}

void TheLightOfDawnEvent::OnUnitDeath(Unit* unit)
{
    switch (unit->GetEntry())
    {
        // Dawn
        case NPC_DEFENDER_OF_THE_LIGHT:
        {
            --defendersRemaining;
            DefenderGUID.erase(unit->GetGUID());
            UpdateWorldState(WORLD_STATE_FORCES_LIGHT, defendersRemaining);
            SummonDawnDefender();
            break;
        }
        // Scorge
        case NPC_RAMPAGING_ABOMINATION:
        {
            --scourgeRemaining;
            AbominationGUID.erase(unit->GetGUID());
            UpdateWorldState(WORLD_STATE_FORCES_SCOURGE, scourgeRemaining);
            SummonAbomination();
            break;
        }
        case NPC_ACHERUS_GHOUL:
        {
            --scourgeRemaining;
            GhoulGUID.erase(unit->GetGUID());
            UpdateWorldState(WORLD_STATE_FORCES_SCOURGE, scourgeRemaining);
            SummonGhoul();
            break;
        }
        case NPC_WARRIOR_OF_THE_FROZEN_WASTES:
        {
            --scourgeRemaining;
            WarriorGUID.erase(unit->GetGUID());
            UpdateWorldState(WORLD_STATE_FORCES_SCOURGE, scourgeRemaining);
            SummonWarriorOfFrozenWastes();
            break;
        }
        case NPC_FLESH_BEHEMOTH:
        {
            --scourgeRemaining;
            BehemothGUID.erase(unit->GetGUID());
            UpdateWorldState(WORLD_STATE_FORCES_SCOURGE, scourgeRemaining);
            SummonBehemoth();
            break;
        }
    }
}

void TheLightOfDawnEvent::SummonAbomination()
{
    if (Creature* Darion = GetCreature(Darion_Mograine))
    {
        Position spawnpos = Darion->GetNearPosition(frand(10.f, 130.f), frand(0, 6.14f));
        if (Unit* temp = Darion->SummonCreature(NPC_RAMPAGING_ABOMINATION, spawnpos.GetPositionX(), spawnpos.GetPositionY(), spawnpos.GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300s))
        {
            temp->SetWalk(false);
            temp->SetFaction(FACTION_UNDEAD_SCOURGE_3);
            AbominationGUID.insert(temp->GetGUID());
        }
    }
}

void TheLightOfDawnEvent::SummonBehemoth()
{
    if (Creature* Darion = GetCreature(Darion_Mograine))
    {
        Position spawnpos = Darion->GetNearPosition(frand(10.f, 130.f), frand(0, 6.14f));
        if (Unit* temp = Darion->SummonCreature(NPC_FLESH_BEHEMOTH, spawnpos.GetPositionX(), spawnpos.GetPositionY(), spawnpos.GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300s))
        {
            temp->SetWalk(false);
            temp->SetFaction(FACTION_UNDEAD_SCOURGE_3);
            BehemothGUID.insert(temp->GetGUID());
        }
    }
}

void TheLightOfDawnEvent::SummonGhoul()
{
    if (Creature* Darion = GetCreature(Darion_Mograine))
    {
        Position spawnpos = Darion->GetNearPosition(frand(10.f, 130.f), frand(0, 6.14f));
        if (Unit* temp = Darion->SummonCreature(NPC_ACHERUS_GHOUL, spawnpos.GetPositionX(), spawnpos.GetPositionY(), spawnpos.GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300s))
        {
            temp->SetWalk(false);
            temp->SetFaction(FACTION_UNDEAD_SCOURGE_3);
            GhoulGUID.insert(temp->GetGUID());
        }
    }
}

void TheLightOfDawnEvent::SummonWarriorOfFrozenWastes()
{
    if (Creature* Darion = GetCreature(Darion_Mograine))
    {
        Position spawnpos = Darion->GetNearPosition(frand(10.f, 130.f), frand(0, 6.14f));
        if (Unit* temp = Darion->SummonCreature(NPC_WARRIOR_OF_THE_FROZEN_WASTES, spawnpos.GetPositionX(), spawnpos.GetPositionY(), spawnpos.GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300s))
        {
            temp->SetWalk(false);
            temp->SetFaction(FACTION_UNDEAD_SCOURGE_3);
            WarriorGUID.insert(temp->GetGUID());
        }
    }
}

void TheLightOfDawnEvent::SummonDawnDefender()
{
    if (Creature* Darion = GetCreature(Darion_Mograine))
    {
        Position spawnpos = Darion->GetNearPosition(frand(10.f, 75.f), frand(0, 6.14f));
        if (Unit* temp = Darion->SummonCreature(NPC_DEFENDER_OF_THE_LIGHT, spawnpos.GetPositionX(), spawnpos.GetPositionY(), spawnpos.GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300s))
        {
            DefenderGUID.insert(temp->GetGUID());
            temp->ToCreature()->SetReactState(REACT_AGGRESSIVE);
            temp->GetMotionMaster()->MovePoint(0, Darion->GetPosition());
        }
    }
}

class SpecialEvent_TheLightOfDawn : public SpecialEventScript
{
public:
    SpecialEvent_TheLightOfDawn() : SpecialEventScript("the_light_of_dawn") { }

    SpecialEvent* GetSpecialEvent() const override
    {
        return new TheLightOfDawnEvent();
    }
};

class EndQuestTheLichKingCommand : public PlayerScript
{
public:
    EndQuestTheLichKingCommand() : PlayerScript("EndQuestTheLichKingCommand") { }

    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        if (questId != QUEST_THE_LIGHT_OF_DAWN)
            return;

        if (player->GetQuestStatus(QUEST_THE_LIGHT_OF_DAWN) == QUEST_STATUS_INCOMPLETE)
        {
            if (SpecialEvent* LoD = sSpecialEventMgr->GetEnabledSpecialEventByEventId(SPECIALEVENT_EVENTID_THELIGHTOFDAWN))
                LoD->DoActionForMember(player->GetGUID(), 0);  // Add player isDoingQuest (because player already in LoD area)
        }
        else if (player->GetQuestStatus(QUEST_THE_LIGHT_OF_DAWN) == QUEST_STATUS_NONE)
        {
            if (SpecialEvent* LoD = sSpecialEventMgr->GetEnabledSpecialEventByEventId(SPECIALEVENT_EVENTID_THELIGHTOFDAWN))
                LoD->DoActionForMember(player->GetGUID(), 1);  // Remove player isDoingQuest
        }
    }
};

void AddSC_TheLightOfDawnScripts()
{
    new SpecialEvent_TheLightOfDawn();
    new EndQuestTheLichKingCommand();
}
