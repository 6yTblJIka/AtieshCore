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
 * Scriptnames of files in this file should be prefixed with "npc_pet_mag_".
 */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CombatAI.h"
#include "GridNotifiersImpl.h"
#include "MotionMaster.h"
#include "Pet.h"
#include "PetAI.h"
#include "ScriptedCreature.h"
#include "SpellHistory.h"
#include "Timer.h"

enum MageSpells
{
    SPELL_MAGE_CLONE_ME                 = 45204,
    SPELL_MAGE_MASTERS_THREAT_LIST      = 58838,
    SPELL_MAGE_FROST_BOLT               = 59638,
    SPELL_MAGE_FIRE_BLAST               = 59637,

    SPELL_FROSTSHIELD                   = 43008,
    SPELL_FIRESHIELD                    = 43046,
    SPELL_ICEBLOCK                      = 65802,
    SPELL_NOVA                          = 42917
};

enum MirrorImageTimers
{
    TIMER_MIRROR_IMAGE_FIRE_BLAST       = 6500
};

struct npc_pet_mage_mirror_image : ScriptedAI
{
    const float CHASE_DISTANCE = 35.0f;

    npc_pet_mage_mirror_image(Creature* creature) : ScriptedAI(creature), _fireBlastTimer(0) { }

    void InitializeAI() override
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        // here mirror image casts on summoner spell (not present in client dbc) 49866
        // here should be auras (not present in client dbc): 35657, 35658, 35659, 35660 selfcast by mirror images (stats related?)
        // Clone Me!
        owner->CastSpell(me, SPELL_MAGE_CLONE_ME, true);
    }

    void UpdateAI(uint32 diff) override
    {
        Unit* owner = me->GetOwner();
        if (!owner)
        {
            me->DespawnOrUnsummon();
            return;
        }

        if (!_fireBlastTimer.Passed())
            _fireBlastTimer.Update(diff);

        if (!UpdateImageVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (_fireBlastTimer.Passed())
        {
            DoCastVictim(SPELL_MAGE_FIRE_BLAST);
            _fireBlastTimer.Reset(TIMER_MIRROR_IMAGE_FIRE_BLAST);
        }
        else
            DoCastVictim(SPELL_MAGE_FROST_BOLT);
    }

    bool CanAIAttack(Unit const* who) const override
    {
        Unit* owner = me->GetOwner();
        return owner && who->IsAlive() && me->IsValidAttackTarget(who) && !who->HasBreakableByDamageCrowdControlAura() && who->IsInCombatWith(owner) && ScriptedAI::CanAIAttack(who);
    }

    // Do not reload Creature templates on evade mode enter - prevent visual lost
    void EnterEvadeMode(EvadeReason /*why*/) override
    {
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
    bool UpdateImageVictim()
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return false;

        if (!me->HasUnitState(UNIT_STATE_CASTING) && !me->IsEngaged() && !owner->IsInCombat())
            return false;

        Unit* currentTarget = me->GetVictim();
        if (currentTarget && !CanAIAttack(currentTarget))
        {
            me->InterruptNonMeleeSpells(true); // do not finish casting on invalid targets
            me->AttackStop();
            currentTarget = nullptr;
        }

        // don't reselect if we're currently casting anyway
        if (currentTarget && me->HasUnitState(UNIT_STATE_CASTING))
            return true;

        Unit* selectedTarget = nullptr;
        CombatManager const& mgr = owner->GetCombatManager();
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
            AttackStartCaster(selectedTarget, CHASE_DISTANCE);
        return true;
    }

    TimeTracker _fireBlastTimer;
};

void AddSC_mage_pet_scripts()
{
    RegisterCreatureAI(npc_pet_mage_mirror_image);
}
