/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#include "ScriptMgr.h"
#include "CombatAI.h"
#include "CreatureAIImpl.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

/*#####
# npc_commander_eligor_dawnbringer
#####*/

enum CommanderEligorDawnbringer
{
    MODEL_IMAGE_OF_KELTHUZAD           = 24787, // Image of Kel'Thuzad
    MODEL_IMAGE_OF_SAPPHIRON           = 24788, // Image of Sapphiron
    MODEL_IMAGE_OF_RAZUVIOUS           = 24799, // Image of Razuvious
    MODEL_IMAGE_OF_GOTHIK              = 24804, // Image of Gothik
    MODEL_IMAGE_OF_THANE               = 24802, // Image of Thane Korth'azz
    MODEL_IMAGE_OF_BLAUMEUX            = 24794, // Image of Lady Blaumeux
    MODEL_IMAGE_OF_ZELIEK              = 24800, // Image of Sir Zeliek
    MODEL_IMAGE_OF_PATCHWERK           = 24798, // Image of Patchwerk
    MODEL_IMAGE_OF_GROBBULUS           = 24792, // Image of Grobbulus
    MODEL_IMAGE_OF_THADDIUS            = 24801, // Image of Thaddius
    MODEL_IMAGE_OF_GLUTH               = 24803, // Image of Gluth
    MODEL_IMAGE_OF_ANUBREKHAN          = 24789, // Image of Anub'rekhan
    MODEL_IMAGE_OF_FAERLINA            = 24790, // Image of Faerlina
    MODEL_IMAGE_OF_MAEXXNA             = 24796, // Image of Maexxna
    MODEL_IMAGE_OF_NOTH                = 24797, // Image of Noth
    MODEL_IMAGE_OF_HEIGAN              = 24793, // Image of Heigan
    MODEL_IMAGE_OF_LOATHEB             = 24795, // Image of Loatheb

    NPC_IMAGE_OF_KELTHUZAD             = 27766, // Image of Kel'Thuzad
    NPC_IMAGE_OF_SAPPHIRON             = 27767, // Image of Sapphiron
    NPC_IMAGE_OF_RAZUVIOUS             = 27768, // Image of Razuvious
    NPC_IMAGE_OF_GOTHIK                = 27769, // Image of Gothik
    NPC_IMAGE_OF_THANE                 = 27770, // Image of Thane Korth'azz
    NPC_IMAGE_OF_BLAUMEUX              = 27771, // Image of Lady Blaumeux
    NPC_IMAGE_OF_ZELIEK                = 27772, // Image of Sir Zeliek
    NPC_IMAGE_OF_PATCHWERK             = 27773, // Image of Patchwerk
    NPC_IMAGE_OF_GROBBULUS             = 27774, // Image of Grobbulus
    NPC_IMAGE_OF_THADDIUS              = 27775, // Image of Thaddius
    NPC_IMAGE_OF_GLUTH                 = 27782, // Image of Gluth
    NPC_IMAGE_OF_ANUBREKHAN            = 27776, // Image of Anub'rekhan
    NPC_IMAGE_OF_FAERLINA              = 27777, // Image of Faerlina
    NPC_IMAGE_OF_MAEXXNA               = 27778, // Image of Maexxna
    NPC_IMAGE_OF_NOTH                  = 27779, // Image of Noth
    NPC_IMAGE_OF_HEIGAN                = 27780, // Image of Heigan
    NPC_IMAGE_OF_LOATHEB               = 27781, // Image of Loatheb

    NPC_INFANTRYMAN                    = 27160, // Add in case I randomize the spawning
    NPC_SENTINAL                       = 27162,
    NPC_BATTLE_MAGE                    = 27164,

    // Five platforms to choose from
    SAY_PINNACLE                       = 0,
    SAY_DEATH_KNIGHT_WING              = 1,
    SAY_ABOMINATION_WING               = 2,
    SAY_SPIDER_WING                    = 3,
    SAY_PLAGUE_WING                    = 4,
    // Used in all talks
    SAY_TALK_COMPLETE                  = 5,
    // Pinnacle of Naxxramas
    SAY_SAPPHIRON                      = 6,
    SAY_KELTHUZAD_1                    = 7,
    SAY_KELTHUZAD_2                    = 8,
    SAY_KELTHUZAD_3                    = 9,
    // Death knight wing of Naxxramas
    SAY_RAZUVIOUS                      = 10,
    SAY_GOTHIK                         = 11,
    SAY_DEATH_KNIGHTS_1                = 12,
    SAY_DEATH_KNIGHTS_2                = 13,
    SAY_DEATH_KNIGHTS_3                = 14,
    SAY_DEATH_KNIGHTS_4                = 15,
    // Blighted abomination wing of Naxxramas
    SAY_PATCHWERK                      = 16,
    SAY_GROBBULUS                      = 17,
    SAY_GLUTH                          = 18,
    SAY_THADDIUS                       = 19,
    // Accursed spider wing of Naxxramas
    SAY_ANUBREKHAN                     = 20,
    SAY_FAERLINA                       = 21,
    SAY_MAEXXNA                        = 22,
    // Dread plague wing of Naxxramas
    SAY_NOTH                           = 23,
    SAY_HEIGAN_1                       = 24,
    SAY_HEIGAN_2                       = 25,
    SAY_LOATHEB                        = 26,

    SPELL_HEROIC_IMAGE_CHANNEL         = 49519,

    EVENT_START_RANDOM                 = 1,
    EVENT_MOVE_TO_POINT                = 2,
    EVENT_TALK_COMPLETE                = 3,
    EVENT_GET_TARGETS                  = 4,
    EVENT_KELTHUZAD_2                  = 5,
    EVENT_KELTHUZAD_3                  = 6,
    EVENT_DEATH_KNIGHTS_2              = 7,
    EVENT_DEATH_KNIGHTS_3              = 8,
    EVENT_DEATH_KNIGHTS_4              = 9,
    EVENT_HEIGAN_2                     = 10
};

uint32 const AudienceMobs[3] = { NPC_INFANTRYMAN, NPC_SENTINAL, NPC_BATTLE_MAGE };

Position const PosTalkLocations[6] =
{
    { 3805.453f, -682.9075f, 222.2917f, 2.793398f }, // Pinnacle of Naxxramas
    { 3807.508f, -691.0882f, 221.9688f, 2.094395f }, // Death knight wing of Naxxramas
    { 3797.228f, -690.3555f, 222.5019f, 1.134464f }, // Blighted abomination wing of Naxxramas
    { 3804.038f, -672.3098f, 222.5019f, 4.578917f }, // Accursed spider wing of Naxxramas
    { 3815.097f, -680.2596f, 221.9777f, 2.86234f  }, // Dread plague wing of Naxxramas
    { 3798.05f,  -680.611f,  222.9825f, 6.038839f }, // Home
};

struct npc_commander_eligor_dawnbringer : public ScriptedAI
{
    npc_commander_eligor_dawnbringer(Creature* creature) : ScriptedAI(creature)
    {
        talkWing = 0;
    }

    void Reset() override
    {
        talkWing = 0;
        for (ObjectGuid& guid : audienceList)
            guid.Clear();

        for (ObjectGuid& guid : imageList)
            guid.Clear();

        _events.ScheduleEvent(EVENT_GET_TARGETS, 5s);
        _events.ScheduleEvent(EVENT_START_RANDOM, 20s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 1)
            {
                me->SetFacingTo(PosTalkLocations[talkWing].GetOrientation());
                TurnAudience();

                switch (talkWing)
                {
                case 0: // Pinnacle of Naxxramas
                    {
                        switch (urand (0, 1))
                        {
                            case 0: ChangeImage(NPC_IMAGE_OF_KELTHUZAD, MODEL_IMAGE_OF_KELTHUZAD, SAY_KELTHUZAD_1);
                                    _events.ScheduleEvent(EVENT_KELTHUZAD_2, 8s); break;
                            case 1: ChangeImage(NPC_IMAGE_OF_SAPPHIRON, MODEL_IMAGE_OF_SAPPHIRON, SAY_SAPPHIRON); break;
                        }
                    }
                    break;
                case 1: // Death knight wing of Naxxramas
                    {
                        switch (urand (0, 2))
                        {
                            case 0: ChangeImage(NPC_IMAGE_OF_RAZUVIOUS, MODEL_IMAGE_OF_RAZUVIOUS, SAY_RAZUVIOUS); break;
                            case 1: ChangeImage(NPC_IMAGE_OF_GOTHIK, MODEL_IMAGE_OF_GOTHIK, SAY_GOTHIK); break;
                            case 2: ChangeImage(NPC_IMAGE_OF_THANE, MODEL_IMAGE_OF_THANE, SAY_DEATH_KNIGHTS_1);
                                    _events.ScheduleEvent(EVENT_DEATH_KNIGHTS_2, 10s); break;
                        }
                    }
                    break;
                case 2: // Blighted abomination wing of Naxxramas
                    {
                        switch (urand (0, 3))
                        {
                            case 0: ChangeImage(NPC_IMAGE_OF_PATCHWERK, MODEL_IMAGE_OF_PATCHWERK, SAY_PATCHWERK); break;
                            case 1: ChangeImage(NPC_IMAGE_OF_GROBBULUS, MODEL_IMAGE_OF_GROBBULUS, SAY_GROBBULUS); break;
                            case 2: ChangeImage(NPC_IMAGE_OF_THADDIUS, MODEL_IMAGE_OF_THADDIUS, SAY_THADDIUS); break;
                            case 3: ChangeImage(NPC_IMAGE_OF_GLUTH, MODEL_IMAGE_OF_GLUTH, SAY_GLUTH); break;
                        }
                    }
                    break;
                case 3: // Accursed spider wing of Naxxramas
                    {
                        switch (urand (0, 2))
                        {
                            case 0: ChangeImage(NPC_IMAGE_OF_ANUBREKHAN, MODEL_IMAGE_OF_ANUBREKHAN, SAY_ANUBREKHAN); break;
                            case 1: ChangeImage(NPC_IMAGE_OF_FAERLINA, MODEL_IMAGE_OF_FAERLINA, SAY_FAERLINA); break;
                            case 2: ChangeImage(NPC_IMAGE_OF_MAEXXNA, MODEL_IMAGE_OF_MAEXXNA, SAY_MAEXXNA); break;
                        }
                    }
                    break;
                case 4: // Dread plague wing of Naxxramas
                    {
                        switch (urand (0, 2))
                        {
                            case 0: ChangeImage(NPC_IMAGE_OF_NOTH, MODEL_IMAGE_OF_NOTH, SAY_NOTH); break;
                            case 1: ChangeImage(NPC_IMAGE_OF_HEIGAN, MODEL_IMAGE_OF_HEIGAN, SAY_HEIGAN_1);
                                    _events.ScheduleEvent(EVENT_HEIGAN_2, 8s); break;
                            case 2: ChangeImage(NPC_IMAGE_OF_LOATHEB, MODEL_IMAGE_OF_LOATHEB, SAY_LOATHEB); break;
                        }
                    }
                    break;
                case 5: // Home
                    _events.ScheduleEvent(EVENT_START_RANDOM, 30s);
                    break;
                }
            }
        }
    }

    void StoreTargets()
    {
        uint8 creaturecount;

        creaturecount = 0;

        for (uint8 ii = 0; ii < 3; ++ii)
        {
            std::vector<Creature*> creatureList;
            GetCreatureListWithEntryInGrid(creatureList, me, AudienceMobs[ii], 15.0f);
            for (Creature* creature : creatureList)
            {
                audienceList[creaturecount] = creature->GetGUID();
                ++creaturecount;
            }
        }

        if (Creature* creature = me->FindNearestCreature(NPC_IMAGE_OF_KELTHUZAD, 20.0f, true))
            imageList[0] = creature->GetGUID();
        if (Creature* creature = me->FindNearestCreature(NPC_IMAGE_OF_RAZUVIOUS, 20.0f, true))
            imageList[1] = creature->GetGUID();
        if (Creature* creature = me->FindNearestCreature(NPC_IMAGE_OF_PATCHWERK, 20.0f, true))
            imageList[2] = creature->GetGUID();
        if (Creature* creature = me->FindNearestCreature(NPC_IMAGE_OF_ANUBREKHAN, 20.0f, true))
            imageList[3] = creature->GetGUID();
        if (Creature* creature = me->FindNearestCreature(NPC_IMAGE_OF_NOTH, 20.0f, true))
            imageList[4] = creature->GetGUID();
    }

    void ChangeImage(uint32 entry, uint32 model, uint8 text)
    {
        if (Creature* creature = ObjectAccessor::GetCreature(*me, imageList[talkWing]))
        {
            Talk(text);
            creature->SetEntry(entry);
            creature->SetDisplayId(model);
            creature->CastSpell(creature, SPELL_HEROIC_IMAGE_CHANNEL);
            _events.ScheduleEvent(EVENT_TALK_COMPLETE, 40s);
        }
    }

    void TurnAudience()
    {
        for (uint8 i = 0; i < 10; ++i)
        {
            if (Creature* creature = ObjectAccessor::GetCreature(*me, audienceList[i]))
                creature->SetFacingToObject(me);
        }
    }

    void UpdateAI(uint32 diff) override
    {
       _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_START_RANDOM:
                    talkWing = urand (0, 4);
                    Talk(talkWing);
                    _events.ScheduleEvent(EVENT_MOVE_TO_POINT, 8s);
                    break;
                case EVENT_MOVE_TO_POINT:
                    me->SetWalk(true);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(1, PosTalkLocations[talkWing].m_positionX, PosTalkLocations[talkWing].m_positionY, PosTalkLocations[talkWing].m_positionZ);
                    break;
                case EVENT_TALK_COMPLETE:
                    talkWing = 5;
                    Talk(talkWing);
                    _events.ScheduleEvent(EVENT_MOVE_TO_POINT, 5s);
                    break;
                case EVENT_GET_TARGETS:
                    StoreTargets();
                    break;
                case EVENT_KELTHUZAD_2:
                    Talk(SAY_KELTHUZAD_2);
                    _events.ScheduleEvent(EVENT_KELTHUZAD_3, 8s);
                    break;
                case EVENT_KELTHUZAD_3:
                    Talk(SAY_KELTHUZAD_3);
                    break;
                case EVENT_DEATH_KNIGHTS_2:
                    Talk(SAY_DEATH_KNIGHTS_2);
                    if (Creature* creature = ObjectAccessor::GetCreature(*me, imageList[talkWing]))
                    {
                        creature->SetEntry(NPC_IMAGE_OF_BLAUMEUX);
                        creature->SetDisplayId(MODEL_IMAGE_OF_BLAUMEUX);
                    }
                    _events.ScheduleEvent(EVENT_DEATH_KNIGHTS_3, 10s);
                    break;
                case EVENT_DEATH_KNIGHTS_3:
                    Talk(SAY_DEATH_KNIGHTS_3);
                    if (Creature* creature = ObjectAccessor::GetCreature(*me, imageList[talkWing]))
                    {
                        creature->SetEntry(NPC_IMAGE_OF_ZELIEK);
                        creature->SetDisplayId(MODEL_IMAGE_OF_ZELIEK);
                    }
                    _events.ScheduleEvent(EVENT_DEATH_KNIGHTS_4, 10s);
                    break;
                case EVENT_DEATH_KNIGHTS_4:
                    Talk(SAY_DEATH_KNIGHTS_4);
                    break;
                case EVENT_HEIGAN_2:
                    Talk(SAY_HEIGAN_2);
                    break;
                default:
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
    private:
        EventMap _events;
        ObjectGuid audienceList[10];
        ObjectGuid imageList[5];
        uint8    talkWing;
};

/*######
## Quest Strengthen the Ancients (12096|12092)
######*/

enum StrengthenAncientsMisc
{
    SAY_WALKER_FRIENDLY         = 0,
    SAY_WALKER_ENEMY            = 1,
    SAY_LOTHALOR                = 0,

    SPELL_CREATE_ITEM_BARK      = 47550,
    SPELL_CONFUSED              = 47044,

    NPC_LOTHALOR                = 26321
};

// 47575 - Strengthen the Ancients: On Interact Dummy to Woodlands Walker
class spell_q12096_q12092_dummy : public SpellScript
{
    PrepareSpellScript(spell_q12096_q12092_dummy);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        uint32 roll = rand32() % 2;

        Creature* tree = GetHitCreature();
        Player* player = GetCaster()->ToPlayer();

        if (!tree || !player)
            return;

        tree->RemoveNpcFlag(UNIT_NPC_FLAG_SPELLCLICK);

        if (roll == 1) // friendly version
        {
            tree->CastSpell(player, SPELL_CREATE_ITEM_BARK);
            tree->AI()->Talk(SAY_WALKER_FRIENDLY, player);
            tree->DespawnOrUnsummon(1s);
        }
        else // enemy version
        {
            tree->AI()->Talk(SAY_WALKER_ENEMY, player);
            tree->SetFaction(FACTION_MONSTER);
            tree->Attack(player, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_q12096_q12092_dummy::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 47530 - Bark of the Walkers
class spell_q12096_q12092_bark : public SpellScript
{
    PrepareSpellScript(spell_q12096_q12092_bark);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Creature* lothalor = GetHitCreature();
        if (!lothalor || lothalor->GetEntry() != NPC_LOTHALOR)
            return;

        lothalor->AI()->Talk(SAY_LOTHALOR);
        lothalor->RemoveAura(SPELL_CONFUSED);
        lothalor->DespawnOrUnsummon(4s);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_q12096_q12092_bark::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/*######
## Quest: Defending Wyrmrest Temple ID: 12372
######*/

enum WyrmDefenderEnum
{
    // Quest data
    QUEST_DEFENDING_WYRMREST_TEMPLE          = 12372,
    GOSSIP_OPTION_ID                         = 0,
    MENU_ID                                  = 9568,

    // Spells data
    SPELL_CHARACTER_SCRIPT                   = 49213,
    SPELL_DEFENDER_ON_LOW_HEALTH_EMOTE       = 52421, // ID - 52421 Wyrmrest Defender: On Low Health Boss Emote to Controller - Random /self/
    SPELL_RENEW                              = 49263, // cast to heal drakes
    SPELL_WYRMREST_DEFENDER_MOUNT            = 49256,

    // Texts data
    WHISPER_MOUNTED                        = 0,
    BOSS_EMOTE_ON_LOW_HEALTH               = 2
};

struct npc_wyrmrest_defender : public VehicleAI
{
    npc_wyrmrest_defender(Creature* creature) : VehicleAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        hpWarningReady = true;
        renewRecoveryCanCheck = false;

        RenewRecoveryChecker = 0;
    }

    bool hpWarningReady;
    bool renewRecoveryCanCheck;

    uint32 RenewRecoveryChecker;

    void Reset() override
    {
        Initialize();
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        // Check system for Health Warning should happen first time whenever get under 30%,
        // after it should be able to happen only after recovery of last renew is fully done (20 sec),
        // next one used won't interfere
        if (hpWarningReady && me->GetHealthPct() <= 30.0f)
        {
            me->CastSpell(me, SPELL_DEFENDER_ON_LOW_HEALTH_EMOTE);
            hpWarningReady = false;
        }

        if (renewRecoveryCanCheck)
        {
            if (RenewRecoveryChecker <= diff)
            {
                renewRecoveryCanCheck = false;
                hpWarningReady = true;
            }
            else
                RenewRecoveryChecker -= diff;
        }
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            case SPELL_WYRMREST_DEFENDER_MOUNT:
                Talk(WHISPER_MOUNTED, me->GetCharmerOrOwner());
                me->SetImmuneToAll(false);
                me->SetUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED);
                break;
            // Both below are for checking low hp warning
            case SPELL_DEFENDER_ON_LOW_HEALTH_EMOTE:
                Talk(BOSS_EMOTE_ON_LOW_HEALTH, me->GetCharmerOrOwner());
                break;
            case SPELL_RENEW:
                if (!hpWarningReady && RenewRecoveryChecker <= 100)
                    RenewRecoveryChecker = 20000;

                renewRecoveryCanCheck = true;
                break;
        }
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId == MENU_ID && gossipListId == GOSSIP_OPTION_ID)
        {
            // Makes player cast trigger spell for 49207 on self
            player->CastSpell(player, SPELL_CHARACTER_SCRIPT, true);
            CloseGossipMenuFor(player);
        }
        return true;
    }

    void OnCharmed(bool /*apply*/) override
    {
        me->RemoveNpcFlag(UNIT_NPC_FLAG_GOSSIP);
    }
};

/*######
## Quest 12053: The Might of the Horde
######*/

enum WarsongBattleStandard
{
    TEXT_TAUNT_1    = 25888,
    TEXT_TAUNT_2    = 25889,
    TEXT_TAUNT_3    = 25890,
    TEXT_TAUNT_4    = 25891,
    TEXT_TAUNT_5    = 25892,
    TEXT_TAUNT_6    = 25893,
    TEXT_TAUNT_7    = 25894
};

// 47304 - Warsong Battle Standard
class spell_dragonblight_warsong_battle_standard : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_warsong_battle_standard);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return sObjectMgr->GetBroadcastText(TEXT_TAUNT_1) &&
            sObjectMgr->GetBroadcastText(TEXT_TAUNT_2) &&
            sObjectMgr->GetBroadcastText(TEXT_TAUNT_3) &&
            sObjectMgr->GetBroadcastText(TEXT_TAUNT_4) &&
            sObjectMgr->GetBroadcastText(TEXT_TAUNT_5) &&
            sObjectMgr->GetBroadcastText(TEXT_TAUNT_6) &&
            sObjectMgr->GetBroadcastText(TEXT_TAUNT_7);
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        caster->Unit::Say(RAND(TEXT_TAUNT_1, TEXT_TAUNT_2, TEXT_TAUNT_3, TEXT_TAUNT_4, TEXT_TAUNT_5, TEXT_TAUNT_6, TEXT_TAUNT_7), caster);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_warsong_battle_standard::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12470 & 13343: Mystery of the Infinite & Mystery of the Infinite, Redux
######*/

enum MysteryOfTheInfinite
{
    SPELL_MIRROR_IMAGE_AURA         = 49889
};

// 49686 - Mystery of the Infinite: Script Effect Player Cast Mirror Image
class spell_dragonblight_moti_mirror_image_script_effect : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_moti_mirror_image_script_effect);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MIRROR_IMAGE_AURA });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_MIRROR_IMAGE_AURA);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dragonblight_moti_mirror_image_script_effect::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 50020 - Mystery of the Infinite: Hourglass cast See Invis on Master
class spell_dragonblight_moti_hourglass_cast_see_invis_on_master : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_moti_hourglass_cast_see_invis_on_master);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ uint32(spellInfo->GetEffect(EFFECT_0).CalcValue()) });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (TempSummon* casterSummon = GetCaster()->ToTempSummon())
            if (Unit* summoner = casterSummon->GetSummonerUnit())
                summoner->CastSpell(summoner, uint32(GetEffectValue()));
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_moti_hourglass_cast_see_invis_on_master::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12457: The Chain Gun And You
######*/

enum TheChainGunAndYou
{
    TEXT_CALL_OUT_1    = 27083,
    TEXT_CALL_OUT_2    = 27084
};

// BasePoints of the dummy effect is ID of npc_text used to group texts, it's not implemented so texts are grouped manually. Same with 49556 but looks like it's not used
// 49550 - Call Out Injured Soldier
class spell_dragonblight_call_out_injured_soldier : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_call_out_injured_soldier);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return sObjectMgr->GetBroadcastText(TEXT_CALL_OUT_1) && sObjectMgr->GetBroadcastText(TEXT_CALL_OUT_2);
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Vehicle* vehicle = GetCaster()->GetVehicleKit())
            if (Unit* passenger = vehicle->GetPassenger(0))
                passenger->Unit::Say(RAND(TEXT_CALL_OUT_1, TEXT_CALL_OUT_2), passenger);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_call_out_injured_soldier::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12252: Torture the Torturer
######*/

enum TortureTheTorturer
{
    WHISPER_TORTURE_1             = 1,
    WHISPER_TORTURE_2             = 2,
    WHISPER_TORTURE_3             = 3,
    WHISPER_TORTURE_4             = 4,
    WHISPER_TORTURE_5             = 5,
    WHISPER_TORTURE_RANDOM        = 6,

    SPELL_TORTURER_KILL_CREDIT    = 48607,
    SPELL_BRANDING_IRON_IMPACT    = 48614
};

// 48603 - High Executor's Branding Iron
class spell_dragonblight_high_executor_branding_iron : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_high_executor_branding_iron);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TORTURER_KILL_CREDIT, SPELL_BRANDING_IRON_IMPACT });
    }

    void HandleWhisper()
    {
        Player* caster = GetCaster()->ToPlayer();
        Creature* target = GetHitCreature();
        if (!caster || !target)
            return;

        target->CastSpell(target, SPELL_BRANDING_IRON_IMPACT);

        if (Aura* aura = caster->GetAura(GetSpellInfo()->Id))
        {
            switch (aura->GetStackAmount())
            {
                case 1:
                    target->AI()->Talk(WHISPER_TORTURE_1, caster);
                    break;
                case 2:
                    target->AI()->Talk(WHISPER_TORTURE_2, caster);
                    break;
                case 3:
                    target->AI()->Talk(WHISPER_TORTURE_3, caster);
                    break;
                case 4:
                    target->AI()->Talk(WHISPER_TORTURE_4, caster);
                    break;
                case 5:
                    target->AI()->Talk(WHISPER_TORTURE_5, caster);
                    target->CastSpell(caster, SPELL_TORTURER_KILL_CREDIT);
                    break;
                case 6:
                    target->AI()->Talk(WHISPER_TORTURE_RANDOM, caster);
                    break;
                default:
                    return;
            }
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_dragonblight_high_executor_branding_iron::HandleWhisper);
    }
};

Position const SkyCaptainCryOfLight1Point = { 3720.85f, -755.389f, 239.45f, 0.0f };
Position const SkyCaptainCryOfLight2Point = { 3713.73f, -1289.81f, 155.37f, 0.0f };
Position const SkyCaptainCryOfLight3Point = { 3828.08f, -1401.45f, 183.12f, 0.0f };

enum SkyCaptainCryOfLight
{
    EVENT_FLY_AT_NPC              = 1,
    EVENT_FLY_AT_2                = 2,
    EVENT_FLY_AT_3                = 3,
    EVENT_DROPPLAYER              = 4,
    EVENT_RETURN_RESPAWN          = 5,

    POINT_1              = 1,
    POINT_2              = 2,
    POINT_3              = 3
};

class npc_sky_captain_cryoflight : public CreatureScript
{
public: npc_sky_captain_cryoflight() : CreatureScript("npc_sky_captain_cryoflight") {}

        struct npc_sky_captain_cryoflightAI : public ScriptedAI
        {
            npc_sky_captain_cryoflightAI(Creature* creature) : ScriptedAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                _events.Reset();
                _player = nullptr;
            }

            void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
            {
                if (Player* player = passenger->ToPlayer())
                {
                    if (me->GetVehicleKit())
                    {
                        Unit* npcpassenger = me->GetVehicleKit()->GetPassenger(1);
                        if (apply)
                        {
                            if (npcpassenger)
                                if (Creature* creature = npcpassenger->ToCreature())
                                    creature->AI()->Talk(0, player);
                            //Talk(27272); // Hang on to your hat, $n! To Scalawag we go!
                            _events.ScheduleEvent(EVENT_FLY_AT_NPC, 1s);
                            _player = player;
                        }
                        else
                        {
                            if (npcpassenger)
                                if (Creature* creature = npcpassenger->ToCreature())
                                    creature->AI()->Talk(1, player);
                            //Talk(26924); // This is your stop, pal. Safe landing!
                            _events.ScheduleEvent(EVENT_RETURN_RESPAWN, 5s);
                        }
                    }
                }
            }

            void MovementInform(uint32 movementType, uint32 pointId) override
            {
                if (movementType == POINT_MOTION_TYPE)
                {
                    if (pointId == POINT_3)
                        _events.ScheduleEvent(EVENT_DROPPLAYER, 100ms);
                    else if (pointId == POINT_1)
                        _events.ScheduleEvent(EVENT_FLY_AT_2, 100ms);
                    else if (pointId == POINT_2)
                        _events.ScheduleEvent(EVENT_FLY_AT_3, 100ms);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!me || !me->IsAlive())
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_DROPPLAYER:
                    {
                        if (_player)
                        {
                            _player->ExitVehicle();
                            me->CastSpell(_player, 45472, true); //parachute
                        }

                        _events.ScheduleEvent(EVENT_FLY_AT_2, 100ms);
                        break;
                    }
                    case EVENT_FLY_AT_NPC:
                    {
                        std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                        {
                            init.MoveTo(SkyCaptainCryOfLight1Point.GetPositionX(), SkyCaptainCryOfLight1Point.GetPositionY(), SkyCaptainCryOfLight1Point.GetPositionZ(), false);
                            init.SetFly();
                        };
                        me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_1, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                        break;
                    }
                    case EVENT_FLY_AT_2:
                    {
                        std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                        {
                            init.MoveTo(SkyCaptainCryOfLight2Point.GetPositionX(), SkyCaptainCryOfLight2Point.GetPositionY(), SkyCaptainCryOfLight2Point.GetPositionZ(), false);
                            init.SetFly();
                        };
                        me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_2, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                        break;
                    }
                    case EVENT_FLY_AT_3:
                    {
                        std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                        {
                            init.MoveTo(SkyCaptainCryOfLight3Point.GetPositionX(), SkyCaptainCryOfLight3Point.GetPositionY(), SkyCaptainCryOfLight3Point.GetPositionZ(), false);
                            init.SetFly();
                        };
                        me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_3, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                        break;
                    }
                    case EVENT_RETURN_RESPAWN:
                    {
                        me->SetVisible(false);
                        if (Unit* npcpassenger = me->GetVehicleKit()->GetPassenger(0))
                            npcpassenger->SetVisible(false);
                        me->Respawn(true);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        private:
            EventMap _events;
            Player* _player;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_sky_captain_cryoflightAI(creature);
        }
};

/*#####
# npc_destructive_ward_kill, quest No Place to Run 12261
#####*/

enum DestructiveWard
{
    SPELL_SUMMON_SKELETON             = 48715,
    SPELL_SUMMON_SMOLDERING_CONSTRUCT = 48718,
    SPELL_DESTRUCTIVE_PULSE           = 48733,
    SPELL_DESTRUCTIVE_BARRAGE         = 48734,
    SPELL_POWERUP                     = 48735,

    SPELL_KILL_REWARD                 = 52409,

    EVENT_SUMMON_SKELETON               = 1,
    EVENT_SUMMON_SKELETON_2             = 2,
    EVENT_POWERUP_1                     = 3,
    EVENT_POWERUP_2                     = 4,
    EVENT_SUMMON_SMOLDERING_CONSTRUCT   = 5,
    EVENT_SUMMON_SMOLDERING_CONSTRUCT_2 = 6,
    EVENT_SUMMON_DOUBLE_SPAWN           = 7,
    EVENT_SUMMON_LAST_SKELET            = 8,
    EVENT_FINAL_POWERUP                 = 9,
    EVENT_DESTRUCTIVE_BARRAGE           = 10,
    EVENT_DESTRUCTIVE_BARRAGE_2         = 11,
    EVENT_FDESPAWN                      = 12
};

class npc_destructive_ward_kill : public CreatureScript
{
public: npc_destructive_ward_kill() : CreatureScript("npc_destructive_ward_kill") {}

    struct npc_destructive_ward_killAI : public ScriptedAI
    {
        npc_destructive_ward_killAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            _events.Reset();
            _events.ScheduleEvent(EVENT_SUMMON_SKELETON, 2s, 3s);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SUMMON_SKELETON:
                    DoCastSelf(SPELL_SUMMON_SKELETON);
                    _events.ScheduleEvent(EVENT_SUMMON_SKELETON_2, 15s);
                    break;
                case EVENT_SUMMON_SKELETON_2:
                    DoCastSelf(SPELL_SUMMON_SKELETON);
                    _events.ScheduleEvent(EVENT_POWERUP_1, 1s);
                    break;
                case EVENT_POWERUP_1:
                    DoCastSelf(SPELL_POWERUP);
                    Talk(0);
                    DoCastSelf(SPELL_DESTRUCTIVE_PULSE);
                    _events.ScheduleEvent(EVENT_SUMMON_SMOLDERING_CONSTRUCT, 15s);
                    break;
                case EVENT_SUMMON_SMOLDERING_CONSTRUCT:
                    DoCastSelf(48718);
                    _events.ScheduleEvent(EVENT_SUMMON_SMOLDERING_CONSTRUCT_2, 15s);
                    break;
                case EVENT_SUMMON_SMOLDERING_CONSTRUCT_2:
                    DoCastSelf(48718);
                    _events.ScheduleEvent(EVENT_POWERUP_2, 1s);
                    break;
                case EVENT_POWERUP_2:
                    DoCastSelf(SPELL_POWERUP);
                    Talk(0);
                    DoCastSelf(SPELL_DESTRUCTIVE_PULSE);
                    _events.ScheduleEvent(EVENT_SUMMON_DOUBLE_SPAWN, 1s);
                    break;
                case EVENT_SUMMON_DOUBLE_SPAWN:
                    DoCastSelf(SPELL_SUMMON_SKELETON);
                    DoCastSelf(SPELL_SUMMON_SMOLDERING_CONSTRUCT);
                    _events.ScheduleEvent(EVENT_SUMMON_LAST_SKELET, 3s);
                    break;
                case EVENT_SUMMON_LAST_SKELET:
                    DoCastSelf(SPELL_SUMMON_SKELETON);
                    _events.ScheduleEvent(EVENT_FINAL_POWERUP, 2s);
                    break;
                case EVENT_FINAL_POWERUP:
                    DoCastSelf(SPELL_POWERUP);
                    Talk(1);
                    DoCastSelf(SPELL_DESTRUCTIVE_BARRAGE);
                    _events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE, 1s);
                    break;
                case EVENT_DESTRUCTIVE_BARRAGE:
                    DoCastSelf(SPELL_DESTRUCTIVE_BARRAGE);
                    _events.ScheduleEvent(EVENT_DESTRUCTIVE_BARRAGE_2, 1s);
                    break;
                case EVENT_DESTRUCTIVE_BARRAGE_2:
                    DoCastSelf(SPELL_DESTRUCTIVE_BARRAGE);
                    if (Unit* summoner = me->GetCharmerOrOwner())
                        me->CastSpell(summoner, SPELL_KILL_REWARD);
                    _events.ScheduleEvent(EVENT_FDESPAWN, 1s);
                    break;
                case EVENT_FDESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
                }
            }
        }
    private:
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_destructive_ward_killAI(creature);
    }
};

/*#####
# npc_body_magehunter, quest Rifle the Bodies 12000
#####*/

enum magehunterbody
{
    SPELL_RIFLE_THE_BODIES           = 61832,
    SPELL_RIFLE_THE_BODIES_LOOT      = 47096,
    ITEM_MAIL_MOONREST_GARDENS_PLANS = 35783
};

class npc_body_magehunter : public CreatureScript
{
public: npc_body_magehunter() : CreatureScript("npc_body_magehunter") {}

    struct npc_body_magehunterAI : public ScriptedAI
    {
        npc_body_magehunterAI(Creature* creature) : ScriptedAI(creature) {}

        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
            if (!caster->ToPlayer())
                return;

            Player* player = caster->ToPlayer();

            if (spellInfo->Id == SPELL_RIFLE_THE_BODIES)
            {
                player->CastSpell(player, SPELL_RIFLE_THE_BODIES_LOOT);
                /*uint32 rand = urand(0, 10);
                if (rand == 7)
                    player->AddItem(ITEM_MAIL_MOONREST_GARDENS_PLANS, 1);*/
            }

            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_body_magehunterAI(creature);
    }
};

/*######
## Quest 12260: The Perfect Dissemblance
######*/

enum ThePerfectDissemblance
{
    SPELL_BANSHEES_MAGIC_MIRROR     = 48648
};

// 48692 - The Perfect Dissemblance: Quest Completion Script
class spell_dragonblight_cancel_banshees_magic_mirror : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_cancel_banshees_magic_mirror);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_BANSHEES_MAGIC_MIRROR });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->RemoveAurasDueToSpell(SPELL_BANSHEES_MAGIC_MIRROR);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_cancel_banshees_magic_mirror::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12274: A Fall From Grace
######*/

enum AFallFromGrace
{
    SPELL_PRIEST_IMAGE_FEMALE     = 48761,
    SPELL_PRIEST_IMAGE_MALE       = 48763
};

// 48762 - A Fall from Grace: Scarlet Raven Priest Image - Master
class spell_dragonblight_scarlet_raven_priest_image_master : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_scarlet_raven_priest_image_master);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_IMAGE_FEMALE, SPELL_PRIEST_IMAGE_MALE });
    }

    void HandleAfterHit()
    {
        if (Player* target = GetHitUnit()->ToPlayer())
            target->CastSpell(target, target->GetNativeGender() == GENDER_FEMALE ? SPELL_PRIEST_IMAGE_FEMALE : SPELL_PRIEST_IMAGE_MALE);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_dragonblight_scarlet_raven_priest_image_master::HandleAfterHit);
    }
};

// 48769 - A Fall from Grace: Quest Completion Script
class spell_dragonblight_cancel_scarlet_raven_priest_image : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_cancel_scarlet_raven_priest_image);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_IMAGE_FEMALE, SPELL_PRIEST_IMAGE_MALE });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->RemoveAurasDueToSpell(SPELL_PRIEST_IMAGE_FEMALE);
        GetCaster()->RemoveAurasDueToSpell(SPELL_PRIEST_IMAGE_MALE);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_cancel_scarlet_raven_priest_image::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12232: Bombard the Ballistae
######*/

enum BombardTheBallistae
{
    SPELL_BALLISTA_BOW               = 48351,
    SPELL_BALLISTA_FRAME             = 48352,
    SPELL_BALLISTA_MISSILE           = 48353,
    SPELL_BALLISTA_WHEEL             = 48354,
    SPELL_BALLISTA_KNOCKBACK         = 52687
};

// 48347 - Bombard the Ballistae: FX Master
class spell_dragonblight_bombard_the_ballistae_fx_master : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_bombard_the_ballistae_fx_master);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_BALLISTA_BOW,
            SPELL_BALLISTA_FRAME,
            SPELL_BALLISTA_MISSILE,
            SPELL_BALLISTA_WHEEL,
            SPELL_BALLISTA_KNOCKBACK
        });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        caster->CastSpell(caster, SPELL_BALLISTA_BOW);
        caster->CastSpell(caster, SPELL_BALLISTA_FRAME);
        caster->CastSpell(caster, SPELL_BALLISTA_MISSILE);
        caster->CastSpell(caster, SPELL_BALLISTA_WHEEL);
        caster->CastSpell(caster, SPELL_BALLISTA_WHEEL);
        caster->CastSpell(caster, SPELL_BALLISTA_WHEEL);
        caster->CastSpell(caster, SPELL_BALLISTA_WHEEL);
        caster->CastSpell(caster, SPELL_BALLISTA_KNOCKBACK);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_bombard_the_ballistae_fx_master::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12060, 12061: Projections and Plans
######*/

enum ProjectionsAndPlans
{
    SPELL_TELE_MOONREST_GARDENS    = 47324,
    SPELL_TELE_SURGE_NEEDLE        = 47325,

    AREA_SURGE_NEEDLE              = 4156,
    AREA_MOONREST_GARDENS          = 4157
};

// 47097 - Surge Needle Teleporter
class spell_dragonblight_surge_needle_teleporter : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_surge_needle_teleporter);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TELE_MOONREST_GARDENS, SPELL_TELE_SURGE_NEEDLE });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        switch (caster->GetAreaId())
        {
            case AREA_SURGE_NEEDLE:
                caster->CastSpell(caster, SPELL_TELE_MOONREST_GARDENS);
                break;
            case AREA_MOONREST_GARDENS:
                caster->CastSpell(caster, SPELL_TELE_SURGE_NEEDLE);
                break;
            default:
                break;
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_surge_needle_teleporter::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

/*######
## Quest 12125, 12126, 12127: In Service of Blood & In Service of the Unholy & In Service of Frost
######*/

// 47703 - Unholy Union
// 47724 - Frost Draw
// 50252 - Blood Draw
class spell_dragonblight_fill_blood_unholy_frost_gem : public SpellScript
{
    PrepareSpellScript(spell_dragonblight_fill_blood_unholy_frost_gem);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ uint32(spellInfo->GetEffect(EFFECT_0).CalcValue()) });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), uint32(GetEffectValue()));
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dragonblight_fill_blood_unholy_frost_gem::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 47447 - Corrosive Spit
class spell_dragonblight_corrosive_spit : public AuraScript
{
    PrepareAuraScript(spell_dragonblight_corrosive_spit);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ uint32(spellInfo->GetEffect(EFFECT_0).CalcValue()) });
    }

    void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget()->HasAura(aurEff->GetSpellInfo()->GetEffect(EFFECT_0).CalcValue()))
            GetAura()->Remove();
    }

    void PeriodicTick(AuraEffect const* aurEff)
    {
        if (GetTarget()->HasAura(aurEff->GetSpellInfo()->GetEffect(EFFECT_0).CalcValue()))
        {
            PreventDefaultAction();
            GetAura()->Remove();
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_dragonblight_corrosive_spit::AfterApply, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dragonblight_corrosive_spit::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

void AddSC_dragonblight()
{
    RegisterCreatureAI(npc_commander_eligor_dawnbringer);
    RegisterSpellScript(spell_q12096_q12092_dummy);
    RegisterSpellScript(spell_q12096_q12092_bark);
    RegisterCreatureAI(npc_wyrmrest_defender);
    RegisterSpellScript(spell_dragonblight_warsong_battle_standard);
    RegisterSpellScript(spell_dragonblight_moti_mirror_image_script_effect);
    RegisterSpellScript(spell_dragonblight_moti_hourglass_cast_see_invis_on_master);
    RegisterSpellScript(spell_dragonblight_call_out_injured_soldier);
    RegisterSpellScript(spell_dragonblight_high_executor_branding_iron);

    new npc_sky_captain_cryoflight();
    new npc_destructive_ward_kill();
    new npc_body_magehunter();

    RegisterSpellScript(spell_dragonblight_cancel_banshees_magic_mirror);
    RegisterSpellScript(spell_dragonblight_scarlet_raven_priest_image_master);
    RegisterSpellScript(spell_dragonblight_cancel_scarlet_raven_priest_image);
    RegisterSpellScript(spell_dragonblight_bombard_the_ballistae_fx_master);
    RegisterSpellScript(spell_dragonblight_surge_needle_teleporter);
    RegisterSpellScript(spell_dragonblight_fill_blood_unholy_frost_gem);
    RegisterSpellScript(spell_dragonblight_corrosive_spit);
}
