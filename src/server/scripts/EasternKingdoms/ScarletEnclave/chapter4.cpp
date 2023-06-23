/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedEscortAI.h"

 //Scarlet courier
enum Ballista
{
    NPC_FROSTBROODVANQUISHER = 28670,
    SPELL_BALLISTA_ASSAULT   = 53117
};

class npc_scarlet_ballista : public CreatureScript
{
public:
    npc_scarlet_ballista() : CreatureScript("npc_scarlet_ballista") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scarlet_ballistaAI(creature);
    }

    struct npc_scarlet_ballistaAI : public ScriptedAI
    {
        npc_scarlet_ballistaAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            uiStage_timer = 2000;
            me->SetIgnoreEvade(true);
            SetCombatMovement(false);
            inCombat = false;
        }

        uint32 uiStage_timer;

        void Reset() override
        {
            targetGUID.Clear();
            Initialize();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            if (uiStage_timer <= diff)
            {
                bool targetReady = false;
                if (targetGUID)
                {
                    if (Creature* target = ObjectAccessor::GetCreature(*me, targetGUID))
                        if (me->IsWithinDist(target, 150.0f))
                            targetReady = true;
                }

                if (!targetReady)
                {
                    if (Creature* target = me->FindNearestCreature(NPC_FROSTBROODVANQUISHER, 120.0f))
                    {
                        AttackStart(target);
                        targetGUID = target->GetGUID();
                        targetReady = true;
                        inCombat = true;
                    }
                }

                if (!targetReady && inCombat)
                {
                    EnterEvadeMode();
                    inCombat = false;
                }

                uiStage_timer = 2000;
            }
            else
                uiStage_timer -= diff;

            if (UpdateVictim())
                DoSpellAttackIfReady(SPELL_BALLISTA_ASSAULT);
        }

    private:
        ObjectGuid targetGUID;
        bool inCombat;
    };
};

void AddSC_the_scarlet_enclave_c4()
{
    new npc_scarlet_ballista();
}
