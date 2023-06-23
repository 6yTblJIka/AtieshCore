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

/*
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "npc_pet_dk_".
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "CombatAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ItemTemplate.h"

enum DeathKnightSpells
{
    // Ebon Gargoyle
    SPELL_DK_SUMMON_GARGOYLE_1      = 49206,
    SPELL_DK_SUMMON_GARGOYLE_2      = 50514,
    SPELL_DK_DISMISS_GARGOYLE       = 50515,
    SPELL_DK_SANCTUARY              = 54661,

    // Dancing Rune Weapon
    SPELL_DK_DANCING_RUNE_WEAPON        = 49028,
    SPELL_COPY_WEAPON                   = 63416,
    SPELL_DK_RUNE_WEAPON_MARK           = 50474,
    SPELL_DK_DANCING_RUNE_WEAPON_VISUAL = 53160,
    SPELL_FAKE_AGGRO_RADIUS_8_YARD      = 49812,
    SPELL_DK_RUNE_WEAPON_SCALING_01     = 51905,
    SPELL_DK_RUNE_WEAPON_SCALING        = 51906,
    SPELL_PET_SCALING__MASTER_SPELL_06__SPELL_HIT_EXPERTISE_SPELL_PENETRATION = 67561,
    SPELL_DK_PET_SCALING_03             = 61697,
    SPELL_AGGRO_8_YD_PBAE               = 49813,
    SPELL_DISMISS_RUNEBLADE             = 50707, // Right now despawn is done by its duration

    // Main Spells
    SPELL_BLOOD_STRIKE                  = 49926,
    SPELL_PLAGUE_STRIKE                 = 49917,

    // Gargoyle attack
    SPELL_ATTACK_GARGOYLE               = 51963
};

enum GargoyleState
{
    EVENT_MOVE_AT_TARGET    = 1,
    EVENT_START_COMBAT      = 2,
    EVENT_COMBAT_OUT        = 3
};

class npc_pet_dk_ebon_gargoyle : public CreatureScript
{
public:
    npc_pet_dk_ebon_gargoyle() : CreatureScript("npc_pet_dk_ebon_gargoyle") { }

    struct npc_pet_dk_ebon_gargoyleAI : ScriptedAI
    {
        npc_pet_dk_ebon_gargoyleAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            ownerGuid = me->GetOwnerGUID();
            if (!ownerGuid)
                return;

            me->SetReactState(REACT_PASSIVE);
            me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);

            _events.Reset();
        }

        void IsSummonedBy(WorldObject* /*summoner*/) override
        {
            _events.ScheduleEvent(EVENT_MOVE_AT_TARGET, 100ms);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            _events.Update(diff);

            while (uint32 _eventId = _events.ExecuteEvent())
            {
                switch (_eventId)
                {
                    case EVENT_MOVE_AT_TARGET:
                    {
                        // Find victim of Summon Gargoyle spell
                        std::list<Unit*> targets;
                        Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 30.0f);
                        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                        Cell::VisitAllObjects(me, searcher, 30.0f);
                        if (!targets.empty())
                        {
                            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                                if ((*iter)->HasAura(SPELL_DK_SUMMON_GARGOYLE_1, ownerGuid))
                                {
                                    victim = (*iter)->GetGUID();
                                    continue;
                                }
                        }
                        else
                        {
                            _events.ScheduleEvent(EVENT_MOVE_AT_TARGET, 100ms);
                            break;
                        }

                        Unit* target = nullptr;
                        if (victim)
                            target = ObjectAccessor::GetUnit(*me, victim);

                        if (!target)
                        {
                            _events.ScheduleEvent(EVENT_MOVE_AT_TARGET, 400ms);
                            break;
                        }

                        float o = target->GetOrientation();
                        float x = target->GetPositionX() + (7 * cos(o)) + target->GetCombatReach();
                        float y = target->GetPositionY() + (7 * sin(o)) + target->GetCombatReach();
                        float z = target->GetPositionZ() + 4.5f;

                        me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, false);

                        me->SetSpeedRate(MOVE_FLIGHT, 1.0f);
                        me->SetSpeedRate(MOVE_RUN, 1.0f);

                        // Fly at target
                        Movement::MoveSplineInit init(me);
                        init.MoveTo(x, y, z);
                        init.SetFly();
                        int32 traveltime = init.Launch();
                        _events.ScheduleEvent(EVENT_START_COMBAT, Milliseconds(traveltime));
                        break;
                    }
                    case EVENT_START_COMBAT:
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        // Start combat
                        if (Unit* target = ObjectAccessor::GetUnit(*me, victim))
                        {
                            me->Attack(target, true);
                            me->GetMotionMaster()->MoveChase(target);
                            DoCast(target, SPELL_ATTACK_GARGOYLE);
                        }
                        break;
                    }
                    case EVENT_COMBAT_OUT:
                    {
                        bool mutualPVP = false;
                        if (Unit* target = ObjectAccessor::GetUnit(*me, victim))
                            if (target->ToPlayer())
                                mutualPVP = true;
                        me->CombatStop(true, mutualPVP);
                        // Stop Fighting
                        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);

                        // Sanctuary
                        me->CastSpell(me, SPELL_DK_SANCTUARY, true);
                        me->SetReactState(REACT_PASSIVE);

                        // Fly Away
                        me->SetSpeedRate(MOVE_FLIGHT, 0.75f);
                        me->SetSpeedRate(MOVE_RUN, 0.75f);
                        float newx = me->GetPositionX() + 20 * std::cos(me->GetOrientation());
                        float newy = me->GetPositionY() + 20 * std::sin(me->GetOrientation());
                        float newz = me->GetPositionZ() + 40.0f;
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(0, newx, newy, newz);

                        // Despawn as soon as possible
                        me->DespawnOrUnsummon(Seconds(4));
                        _events.Reset();
                        break;
                    }
                    default:
                        break;
                }
            }

            if (UpdateVictim())
                DoSpellAttackIfReady(SPELL_ATTACK_GARGOYLE);

            if (UpdateVictim() && !me->HasUnitState(UNIT_STATE_CASTING))
                DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            _events.Reset();
            // Stop Feeding Gargoyle when it dies
            if (Unit* owner = me->GetOwner())
                owner->RemoveAurasDueToSpell(SPELL_DK_SUMMON_GARGOYLE_2);
        }

        // Fly away when dismissed
        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
                if (spellInfo->Id != SPELL_DK_DISMISS_GARGOYLE || !me->IsAlive())
                    return;

            Unit* owner = me->GetOwner();
            if (!owner || owner != caster)
                return;
            _events.ScheduleEvent(EVENT_COMBAT_OUT, 0s);
        }

    private:
        ObjectGuid victim;
        ObjectGuid ownerGuid;
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pet_dk_ebon_gargoyleAI(creature);
    }
};

struct npc_pet_dk_guardian : public AggressorAI
{
    npc_pet_dk_guardian(Creature* creature) : AggressorAI(creature) { }

    bool CanAIAttack(Unit const* target) const override
    {
        if (!target)
            return false;
        Unit* owner = me->GetOwner();
        if (owner && !target->IsInCombatWith(owner))
            return false;
        return AggressorAI::CanAIAttack(target);
    }
};

enum DancingRuneWeaponMisc
{
    TASK_GROUP_COMBAT = 1,
    DATA_INITIAL_TARGET_GUID = 1,
};

struct npc_pet_dk_rune_weapon : ScriptedAI
{
    npc_pet_dk_rune_weapon(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        me->SetReactState(REACT_PASSIVE);

        if (summoner->GetTypeId() != TYPEID_UNIT)
            return;

        Unit* unitSummoner = summoner->ToUnit();

        DoCast(unitSummoner, SPELL_COPY_WEAPON, true);
        DoCast(unitSummoner, SPELL_DK_RUNE_WEAPON_MARK, true);
        DoCastSelf(SPELL_DK_DANCING_RUNE_WEAPON_VISUAL, true);
        DoCastSelf(SPELL_FAKE_AGGRO_RADIUS_8_YARD, true);
        DoCastSelf(SPELL_DK_RUNE_WEAPON_SCALING_01, true);
        DoCastSelf(SPELL_DK_RUNE_WEAPON_SCALING, true);
        DoCastSelf(SPELL_PET_SCALING__MASTER_SPELL_06__SPELL_HIT_EXPERTISE_SPELL_PENETRATION, true);
        DoCastSelf(SPELL_DK_PET_SCALING_03, true);

        _scheduler.Schedule(500ms, [this](TaskContext /*activate*/)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (!_targetGUID.IsEmpty())
                {
                    if (Unit* target = ObjectAccessor::GetUnit(*me, _targetGUID))
                        me->EngageWithTarget(target);
                }
            }).Schedule(6s, [this](TaskContext visual)
                {
                    // Cast every 6 seconds
                    DoCastSelf(SPELL_DK_DANCING_RUNE_WEAPON_VISUAL, true);
                    visual.Repeat();
                });
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id == DATA_INITIAL_TARGET_GUID)
            _targetGUID = guid;
    }

    void JustEnteredCombat(Unit* who) override
    {
        ScriptedAI::JustEnteredCombat(who);

        // Investigate further if these casts are done by any owned aura, eitherway SMSG_SPELL_GO is sent every X seconds.
        _scheduler.Schedule(1s, TASK_GROUP_COMBAT, [this](TaskContext aggro8YD)
            {
                // Cast every second
                if (Unit* victim = me->GetVictim())
                    DoCast(victim, SPELL_AGGRO_8_YD_PBAE, true);
                aggro8YD.Repeat();
            });
    }

    void UpdateAI(uint32 diff) override
    {
        Unit* owner = me->GetOwner();
        if (!owner)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _scheduler.Update(diff);

        if (!UpdateRuneWeaponVictim())
            return;

        DoMeleeAttackIfReady();
    }

    bool CanAIAttack(Unit const* who) const override
    {
        Unit* owner = me->GetOwner();
        return owner && who->IsAlive() && me->IsValidAttackTarget(who) && !who->HasBreakableByDamageCrowdControlAura() && who->IsInCombatWith(owner) && ScriptedAI::CanAIAttack(who);
    }

    // Do not reload Creature templates on evade mode enter - prevent visual lost
    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        _scheduler.CancelGroup(TASK_GROUP_COMBAT);

        if (!me->IsAlive())
        {
            EngagementOver();
            return;
        }

        Unit* owner = me->GetCharmerOrOwner();

        me->CombatStop(true);
        me->SetLootRecipient(nullptr);
        me->ResetPlayerDamageReq();
        me->SetLastDamagedTime(0);
        me->SetCannotReachTarget(false);
        me->DoNotReacquireSpellFocusTarget();
        me->SetTarget(ObjectGuid::Empty);
        EngagementOver();

        if (owner && !me->HasUnitState(UNIT_STATE_FOLLOW))
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
        }
    }

private:
    // custom UpdateVictim implementation to handle special target selection
    // we prioritize between things that are in combat with owner based on the owner's threat to them
    bool UpdateRuneWeaponVictim()
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return false;

        if (!me->IsEngaged() && !owner->IsInCombat())
            return false;

        Unit* currentTarget = me->GetVictim();
        if (currentTarget && !CanAIAttack(currentTarget))
        {
            me->InterruptNonMeleeSpells(true); // do not finish casting on invalid targets
            me->AttackStop();
            currentTarget = nullptr;
        }

        Unit* selectedTarget = nullptr;

        // first, try to get the initial target
        if (Unit* initialTarget = ObjectAccessor::GetUnit(*me, _targetGUID))
        {
            if (CanAIAttack(initialTarget))
                selectedTarget = initialTarget;
        }
        else if (!_targetGUID.IsEmpty())
            _targetGUID.Clear();

        CombatManager const& mgr = owner->GetCombatManager();
        if (!selectedTarget)
        {
            if (mgr.HasPvPCombat())
            {
                // select pvp target
                float minDistance = 0.f;
                for (auto const& pair : mgr.GetPvPCombatRefs())
                {
                    Unit* target = pair.second->GetOther(owner);
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        continue;
                    if (!CanAIAttack(target))
                        continue;

                    float dist = owner->GetDistance(target);
                    if (!selectedTarget || dist < minDistance)
                    {
                        selectedTarget = target;
                        minDistance = dist;
                    }
                }
            }
        }

        if (!selectedTarget)
        {
            // select pve target
            float maxThreat = 0.f;
            for (auto const& pair : mgr.GetPvECombatRefs())
            {
                Unit* target = pair.second->GetOther(owner);
                if (!CanAIAttack(target))
                    continue;

                float threat = target->GetThreatManager().GetThreat(owner);
                if (threat >= maxThreat)
                {
                    selectedTarget = target;
                    maxThreat = threat;
                }
            }
        }

        if (!selectedTarget)
        {
            EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
            return false;
        }

        if (selectedTarget != me->GetVictim())
            AttackStart(selectedTarget);
        return true;
    }

    TaskScheduler _scheduler;
    ObjectGuid _targetGUID;
};

void AddSC_deathknight_pet_scripts()
{
    new npc_pet_dk_ebon_gargoyle();
    RegisterCreatureAI(npc_pet_dk_guardian);
    RegisterCreatureAI(npc_pet_dk_rune_weapon);
}
