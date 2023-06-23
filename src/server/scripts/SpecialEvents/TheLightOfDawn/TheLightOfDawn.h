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

#ifndef _TLODEVENT_H
#define _TLODEVENT_H

#include "GameTime.h"
#include "ObjectGuid.h"
#include "SpecialEvent.h"
#include "Position.h"
#include "EventMap.h"
#include "World.h"
#include "ZoneScript.h"

enum DataEnum
{
    QUEST_THE_LIGHT_OF_DAWN             = 12801,

    // encounters
    ENCOUNTER_START_TIME                = 5,
    ENCOUNTER_TOTAL_DEFENDERS           = 300,
    ENCOUNTER_TOTAL_SCOURGE             = 10000,

    // map_id
    MAPID_THE_SCARLET_ENCLAVE           = 609,
    // zone id
    ZONEID_THE_SCARLET_ENCLAVE          = 4298, // Zone: Plaguelands: The Scarlet Enclave

    // world states
    // basically world states should be shown to all players with phase mask = 128 as stated in DBC
    // because we don't have the possibility to do that we'll just iterate through the players and set the phase mask manually based on the battle status
    WORLD_STATE_FORCES_SHOW             = 3592,             // show the remaining units
    WORLD_STATE_FORCES_SCOURGE          = 3591,
    WORLD_STATE_FORCES_LIGHT            = 3590,
    WORLD_STATE_BATTLE_TIMER_SHOW       = 3603,             // countdown timer
    WORLD_STATE_BATTLE_TIMER            = 3604,
    WORLD_STATE_BATTLE_BEGIN            = 3605,             // battle has begun
};

enum LightOfDawnSays
{
    SAY_LIGHT_OF_DAWN01               = 0, // pre text
    SAY_LIGHT_OF_DAWN02               = 1,
    SAY_LIGHT_OF_DAWN03               = 2,
    SAY_LIGHT_OF_DAWN04               = 3, // intro
    SAY_LIGHT_OF_DAWN05               = 4,
    SAY_LIGHT_OF_DAWN06               = 5,
    SAY_LIGHT_OF_DAWN07               = 6, // During the fight - Korfax, Champion of the Light
    SAY_LIGHT_OF_DAWN08               = 7, // Lord Maxwell Tyrosus
    SAY_LIGHT_OF_DAWN09               = 8, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN25               = 24, // After the fight
    SAY_LIGHT_OF_DAWN26               = 25, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN27               = 26, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN28               = 27, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN29               = 28, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN30               = 29, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN31               = 30, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN32               = 31, // Highlord Alexandros Mograine
    SAY_LIGHT_OF_DAWN33               = 32, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN34               = 33, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN35               = 34, // Darion Mograine
    SAY_LIGHT_OF_DAWN36               = 35, // Darion Mograine
    SAY_LIGHT_OF_DAWN37               = 36, // Highlord Alexandros Mograine
    SAY_LIGHT_OF_DAWN38               = 37, // Darion Mograine
    SAY_LIGHT_OF_DAWN39               = 38, // Highlord Alexandros Mograine
    SAY_LIGHT_OF_DAWN40               = 39, // Darion Mograine
    SAY_LIGHT_OF_DAWN41               = 40, // Highlord Alexandros Mograine
    SAY_LIGHT_OF_DAWN42               = 41, // Highlord Alexandros Mograine
    SAY_LIGHT_OF_DAWN43               = 42, // The Lich King
    SAY_LIGHT_OF_DAWN44               = 43, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN45               = 44, // The Lich King
    SAY_LIGHT_OF_DAWN46               = 45, // The Lich King
    SAY_LIGHT_OF_DAWN47               = 46, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN48               = 47, // The Lich King
    SAY_LIGHT_OF_DAWN49               = 48, // The Lich King
    SAY_LIGHT_OF_DAWN50               = 49, // Lord Maxwell Tyrosus
    SAY_LIGHT_OF_DAWN51               = 50, // The Lich King
    SAY_LIGHT_OF_DAWN52               = 51, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN53               = 52, // Highlord Darion Mograine
    SAY_LIGHT_OF_DAWN54               = 53, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN55               = 54, // The Lich King
    SAY_LIGHT_OF_DAWN56               = 55, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN57               = 56, // The Lich King
    SAY_LIGHT_OF_DAWN58               = 57, // The Lich King
    SAY_LIGHT_OF_DAWN59               = 58, // The Lich King
    SAY_LIGHT_OF_DAWN60               = 59, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN61               = 60, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN62               = 61, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN63               = 62, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN64               = 63, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN65               = 64, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN66               = 65, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN67               = 66, // Highlord Tirion Fordring
    SAY_LIGHT_OF_DAWN68               = 67, // Highlord Darion Mograine

    EMOTE_LIGHT_OF_DAWN01             = 68,  // Emotes
    EMOTE_LIGHT_OF_DAWN02             = 69,
    EMOTE_LIGHT_OF_DAWN03             = 70,
    EMOTE_LIGHT_OF_DAWN04             = 71,
    EMOTE_LIGHT_OF_DAWN05             = 72,
    EMOTE_LIGHT_OF_DAWN06             = 73,
    EMOTE_LIGHT_OF_DAWN07             = 74,
    EMOTE_LIGHT_OF_DAWN08             = 75,
    EMOTE_LIGHT_OF_DAWN09             = 76,
    EMOTE_LIGHT_OF_DAWN10             = 77,
    EMOTE_LIGHT_OF_DAWN11             = 78,
    EMOTE_LIGHT_OF_DAWN12             = 79,
    EMOTE_LIGHT_OF_DAWN13             = 80,
    EMOTE_LIGHT_OF_DAWN14             = 81,
    EMOTE_LIGHT_OF_DAWN15             = 82,
    EMOTE_LIGHT_OF_DAWN16             = 83,
    EMOTE_LIGHT_OF_DAWN17             = 84,
    EMOTE_LIGHT_OF_DAWN18             = 85
};

enum LightOfDawnNPCs
{
    // Quest Credit NPC
    NPC_LIGHT_OF_DAWN_CREDIT            = 29245,
    // Defenders
    NPC_DEFENDER_OF_THE_LIGHT           = 29174,
    NPC_KORFAX_CHAMPION_OF_THE_LIGHT    = 29176,
    NPC_COMMANDER_ELIGOR_DAWNBRINGER    = 29177,
    NPC_LORD_MAXWELL_TYROSUS            = 29178,
    NPC_LEONID_BARTHALOMEW_THE_REVERED  = 29179,
    NPC_DUKE_NICHOLAS_ZVERENHOFF        = 29180,
    NPC_RAYNE                           = 29181,
    NPC_RIMBLAT_EARTHSHATTER            = 29182,

    // Scourge
    NPC_RAMPAGING_ABOMINATION           = 29186,
    NPC_ACHERUS_GHOUL                   = 29219,
    NPC_WARRIOR_OF_THE_FROZEN_WASTES    = 29206,
    NPC_FLESH_BEHEMOTH                  = 29190,

    NPC_HIGHLORD_DARION_MOGRAINE        = 29173,
    NPC_KOLTIRA_DEATHWEAVER             = 29199,
    NPC_ORBAZ_BLOODBANE                 = 29204,
    NPC_THASSARIAN                      = 29200,

    // Outro
    NPC_HIGHLORD_TIRION_FORDRING        = 29175,
    NPC_HIGHLORD_ALEXANDROS_MOGRAINE    = 29227, // ghost
    NPC_DARION_MOGRAINE                 = 29228, // ghost
    NPC_THE_LICH_KING                   = 29183,
};

enum LightOfDawnSpells
{
    // Intro Spells
    SPELL_CAMERA_SHAKE_INIT             = 36455,
    SPELL_CAMERA_SHAKE                  = 39983,
    SPELL_THE_MIGHT_OF_MOGRAINE         = 53642,

    // Mograine Fight
    SPELL_ANTI_MAGIC_ZONE1              = 52893,
    SPELL_DEATH_STRIKE                  = 53639,
    SPELL_DEATH_EMBRACE                 = 53635,
    SPELL_ICY_TOUCH1                    = 49723,
    SPELL_UNHOLY_BLIGHT                 = 53640,

    // Outro
    SPELL_THE_LIGHT_OF_DAWN             = 53658,
    SPELL_ALEXANDROS_MOGRAINE_SPAWN     = 53667,
    SPELL_ICEBOUND_VISAGE               = 53274,
    SPELL_SOUL_FEAST_ALEX               = 53677,
    SPELL_MOGRAINE_CHARGE               = 53679,
    SPELL_REBUKE                        = 53680,
    SPELL_SOUL_FEAST_TIRION             = 53685,
    SPELL_APOCALYPSE                    = 53210,
    SPELL_THROW_ASHBRINGER              = 53701,
    SPELL_REBIRTH_OF_THE_ASHBRINGER     = 53702,
    SPELL_TIRION_CHARGE                 = 53705,
    SPELL_EXIT_TELEPORT_VISUAL          = 61456,
    SPELL_LAY_ON_HANDS                  = 53778,
    SPELL_THE_LIGHT_OF_DAWN_Q           = 53606
};

struct PlayersData
{
    bool isDoingQuest;
    bool isInArea;
};
typedef std::unordered_map<ObjectGuid, PlayersData> PlayersDataContainer;

enum TheLightOfDawnPhase
{
    PHASE_IDLE  = 1,
    PHASE_INTRO = 2,
    PHASE_FIGHT = 3,
    PHASE_OUTRO = 4,
};

enum TheLightOfDawnEncounters
{
    // Light
    ENCOUNTER_DEFENDER_NUMBER         = 30, // how many of defender
    // Scourge
    ENCOUNTER_ABOMINATION_NUMBER      = 10, // how many of abomination
    ENCOUNTER_BEHEMOTH_NUMBER         = 3,  // how many of behemoth
    ENCOUNTER_GHOUL_NUMBER            = 60, // how many of ghoul
    ENCOUNTER_WARRIOR_NUMBER          = 25, // how many of warrior
};

class TC_GAME_API TheLightOfDawnEvent : public SpecialEvent
{
public:
    TheLightOfDawnEvent();
    ~TheLightOfDawnEvent();

    // phase
    TheLightOfDawnPhase GetPhase() { return phaseStage; }
    void SetPhase(TheLightOfDawnPhase phase) { phaseStage = phase; }

    void UpdateWorldState(uint32 id, uint32 state);
    void UpdateAllWorldStates();
    void SetupInitialStates();

    void DoPlaySoundToAll(uint32 soundId);
    void BroadcastPacketToPlayersInArea(WorldPacket const* data) const;

    void StartTheLightOfDawnEvent();
    void StopTheLightOfDawnEvent();

    void SummonScorgeArmy();
    void SummonDefenseArmy();
    void ClearAllNPC();

    // Override Sector
    // SpecialEvent sector
    void Update(uint32 diff) override;
    void OnSpecialEventStart() override;
    void OnSpecialEventEnd(bool /*endByTimer*/) override;
    void AddPlayer(ObjectGuid playerGUID) override;
    void RemovePlayer(ObjectGuid playerGUID) override;
    bool IsMemberOfEvent(Player* player) override;
    uint32 GetCountPlayerInEvent() override { return m_playersDataStore.size(); }
    bool SetupSpecialEvent(bool enabled, bool active, bool repeatable, uint32 id, uint32 cooldownTimer, uint32 durationTimer, std::string comment) override;
    bool IsSpellAreaAllowed(uint32 /*spellId*/, Player const* /*player*/, uint32 /*newArea*/) const  override { return true; }
    void DoActionForMember(ObjectGuid playerGUID, uint32 param) override;
    // ZoneScript sector
    void HandlePlayerEnterZone(Player* player, uint32 zoneId) override;
    // Called when a player leaves the battlefield zone
    void HandlePlayerLeaveZone(Player* player, uint32 zoneId) override;

    void HandlePlayerEnterArea(Player* player, uint32 zoneId, uint32 areaId) override;
    void HandlePlayerLeaveArea(Player* player, uint32 zoneId, uint32 areaId) override;

    Creature* GetCreature(ObjectGuid guid);
    GameObject* GetGameObject(ObjectGuid guid);

    void OnCreatureCreate(Creature* creature) override;
    void OnUnitDeath(Unit* unit) override;

    void SummonAbomination();
    void SummonBehemoth();
    void SummonGhoul();
    void SummonWarriorOfFrozenWastes();

    void SummonDawnDefender();

private:
    TheLightOfDawnPhase phaseStage;
    PlayersDataContainer m_playersDataStore;

    Map* _map;
    uint32 _mapId;

    uint32 defendersRemaining;
    uint32 scourgeRemaining;
    uint32 countdownTimerRemaining;
    bool show_soldiers_count;
    bool show_timer;
    bool show_event_begin;
    EventMap events;

    // Dawn
    ObjectGuid TirionGUID;
    ObjectGuid AlexandrosGUID;
    ObjectGuid DarionGUID;
    ObjectGuid KorfaxGUID;
    ObjectGuid MaxwellGUID;
    ObjectGuid EligorGUID;
    ObjectGuid LeonidGUID;
    ObjectGuid RayneGUID;
    ObjectGuid ZverenhoffGUID;
    ObjectGuid RimblatGUID;
    GuidUnorderedSet DefenderGUID;    //ENCOUNTER_DEFENDER_NUMBER

    // Scourge
    ObjectGuid Darion_Mograine;
    ObjectGuid KoltiraGUID;
    ObjectGuid OrbazGUID;
    ObjectGuid ThassarianGUID;
    ObjectGuid LichKingGUID;

    GuidUnorderedSet AbominationGUID; //ENCOUNTER_ABOMINATION_NUMBER
    GuidUnorderedSet BehemothGUID;    //ENCOUNTER_BEHEMOTH_NUMBER
    GuidUnorderedSet GhoulGUID;       //ENCOUNTER_GHOUL_NUMBER
    GuidUnorderedSet WarriorGUID;     //ENCOUNTER_WARRIOR_NUMBER
};

#endif // _TLODEVENT_H
