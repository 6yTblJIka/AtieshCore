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
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SpecialEvent.h"
#include "SpecialEventMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

class npc_highlord_darion_mograine : public CreatureScript
{
public:
    npc_highlord_darion_mograine() : CreatureScript("npc_highlord_darion_mograine") { }

    struct npc_highlord_darion_mograineAI : public EscortAI
    {
        npc_highlord_darion_mograineAI(Creature* creature) : EscortAI(creature)
        {
            Reset();
        }

        void Reset() override
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
            {
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->Mount(25279);
                me->SetWalk(false);
                me->SetSpeedRate(MOVE_RUN, 2.0f);
                me->SetVisible(true);
            }
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1:
                    CloseGossipMenuFor(player);
                    /*if (SpecialEvent* LoD = sSpecialEventMgr->GetEnabledSpecialEventByEventId(SPECIALEVENT_EVENTID_THELIGHTOFDAWN))
                        LoD->StartSpecialEvent();*/
                    player->CompleteQuest(12801);
                    break;
            }
            return true;
        }

        bool OnGossipHello(Player* player) override
        {
            if (me->IsQuestGiver())
                player->PrepareQuestMenu(me->GetGUID());

            /*if (SpecialEvent* LoD = sSpecialEventMgr->GetEnabledSpecialEventByEventId(SPECIALEVENT_EVENTID_THELIGHTOFDAWN))
            {
                if (!LoD->IsActiveTime())
                    if (player->GetQuestStatus(12801) == QUEST_STATUS_INCOMPLETE)
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I am ready, Highlord. Let the siege of the Last Hope Chapel begin!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            }*/

            //TODO:remove it and finish QUEST
            if (player->GetQuestStatus(12801) == QUEST_STATUS_INCOMPLETE)
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "(UNDER DEVELOPMENT) QUEST: skip quest!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_highlord_darion_mograineAI(creature);
    }
};

/*######
## npc the lich king in dawn of light
######*/
class npc_the_lich_king_tirion_dawn : public CreatureScript
{
public:
    npc_the_lich_king_tirion_dawn() : CreatureScript("npc_the_lich_king_tirion_dawn") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_the_lich_king_tirion_dawnAI(creature);
    }

    struct npc_the_lich_king_tirion_dawnAI : public ScriptedAI
    {
        npc_the_lich_king_tirion_dawnAI(Creature* creature) : ScriptedAI(creature) { Reset(); }
        void Reset() override { }
        void AttackStart(Unit* /*who*/) override { } // very sample, just don't make them aggreesive
        void UpdateAI(uint32 /*diff*/) override { }
        void JustDied(Unit* /*killer*/) override { }
    };

};

enum ScorgeSpells
{
    // ABOMINATION
    SPELL_ABOMINATION_CLEAVE = 53633,
    SPELL_ABOMINATION_HOOK   = 50335,

    // GHOUL
    SPELL_GHOUL_GHOULPLOSION = 53632,

    // Warrior of frozen wastes
    SPELL_WARRIOR_CLEAVE     = 53631,

    // Behemoth
    SPELL_BEHEMOTH_THUNDER   = 36706,
    SPELL_BEHEMOTH_STOMP     = 53634
};

uint32 GetScorgeTargetEntry()
{
    uint32 targetEntry[8] =
    {   29174,                      // NPC_DEFENDER_OF_THE_LIGHT
        29176,                      // NPC_KORFAX_CHAMPION_OF_THE_LIGHT
        29177,                      // NPC_COMMANDER_ELIGOR_DAWNBRINGER
        29178,                      // NPC_LORD_MAXWELL_TYROSUS
        29179,                      // NPC_LEONID_BARTHALOMEW_THE_REVERED
        29180,                      // NPC_DUKE_NICHOLAS_ZVERENHOFF
        29181,                      // NPC_RAYNE
        29182 };                    // NPC_RIMBLAT_EARTHSHATTER

    return targetEntry[urand(0, 7)];
}

Unit* GetVictimInDistance(Unit* owner, float mindistance, float maxdistance)
{
    Unit* victim = nullptr;

    std::vector<Unit*> targets;
    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(owner, owner, maxdistance);
    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(owner, targets, u_check);
    Cell::VisitAllObjects(owner, searcher, maxdistance);
    Position ownerpos = owner->GetPosition();
    for (auto const& pointer : targets)
    {
        if (pointer->GetDistance(ownerpos) >= mindistance &&
            pointer->GetDistance(ownerpos) <= maxdistance)
        {
            victim = pointer;
            break;
        }
    }
    targets.clear();

    return victim;
}

enum AbominationEvent
{
    EVENT_CLEAVE = 1,
    EVENT_HOOK   = 2
};

class npc_scorge_abomination : public CreatureScript
{
public:
    npc_scorge_abomination() : CreatureScript("npc_scorge_abomination") { }

    struct npc_scorge_abominationAI : public ScriptedAI
    {
        npc_scorge_abominationAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.Reset();
        }

        void Reset() override
        {
            events.Reset();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_HOOK, 0s);
            events.ScheduleEvent(EVENT_CLEAVE, 4s);
        }

        void EnterEvadeMode(EvadeReason /*why*/) override {}

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HOOK:
                        if (Unit* victim = GetVictimInDistance(me, 8.f, 40.f))
                            DoCast(victim, SPELL_ABOMINATION_HOOK);
                        events.ScheduleEvent(SPELL_ABOMINATION_HOOK, 10s);
                        break;
                    case EVENT_CLEAVE:
                        if (Unit* victim = GetVictimInDistance(me, 0.f, 5.f))
                            DoCast(victim, SPELL_ABOMINATION_CLEAVE);
                        events.ScheduleEvent(EVENT_CLEAVE, 10s);
                        break;
                }
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
            else
            {
                if (Creature* target = me->FindNearestCreature(GetScorgeTargetEntry(), 40.0f))
                    AttackStart(target);
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scorge_abominationAI(creature);
    }
};

enum GhoulEvent
{
    EVENT_KABOOM = 1
};

class npc_scorge_ghoul : public CreatureScript
{
public:
    npc_scorge_ghoul() : CreatureScript("npc_scorge_ghoul") { }

    struct npc_scorge_ghoulAI : public ScriptedAI
    {
        npc_scorge_ghoulAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.Reset();
        }

        void Reset() override
        {
            events.Reset();
        }

        void EnterEvadeMode(EvadeReason /*why*/) override {}

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_KABOOM, randtime(10s, 20s));
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_KABOOM:
                        if (Unit* victim = GetVictimInDistance(me, 0.f, 5.f))
                            DoCast(victim, SPELL_GHOUL_GHOULPLOSION);
                        events.ScheduleEvent(EVENT_KABOOM, 10s);
                        break;
                }
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
            else
            {
                if (Creature* target = me->FindNearestCreature(GetScorgeTargetEntry(), 15.0f))
                    AttackStart(target);
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scorge_ghoulAI(creature);
    }
};

enum WarriorEvent
{
    EVENT_WARRIOR_CLEAVE = 1
};

class npc_scorge_warrior_of_frozen_wastes : public CreatureScript
{
public:
    npc_scorge_warrior_of_frozen_wastes() : CreatureScript("npc_scorge_warrior_of_frozen_wastes") { }

    struct npc_scorge_warrior_of_frozen_wastesAI : public ScriptedAI
    {
        npc_scorge_warrior_of_frozen_wastesAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.Reset();
        }

        void Reset() override
        {
            events.Reset();
        }

        void EnterEvadeMode(EvadeReason /*why*/) override {}

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_WARRIOR_CLEAVE, 5s);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_WARRIOR_CLEAVE:
                        DoCastVictim(SPELL_WARRIOR_CLEAVE);
                        events.ScheduleEvent(EVENT_WARRIOR_CLEAVE, 10s);
                        break;
                }
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
            else
            {
                if (Creature* target = me->FindNearestCreature(GetScorgeTargetEntry(), 15.0f))
                    AttackStart(target);
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scorge_warrior_of_frozen_wastesAI(creature);
    }
};

enum BehemothEvent
{
    EVENT_BEHEMOTH_THUNDER = 1,
    EVENT_BEHEMOTH_STOMP   = 2
};

class npc_scorge_behemoth : public CreatureScript
{
public:
    npc_scorge_behemoth() : CreatureScript("npc_scorge_behemoth") { }

    struct npc_scorge_behemothAI : public ScriptedAI
    {
        npc_scorge_behemothAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.Reset();
        }

        void Reset() override
        {
            events.Reset();
        }

        void EnterEvadeMode(EvadeReason /*why*/) override {}

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_BEHEMOTH_THUNDER, 3s);
            events.ScheduleEvent(EVENT_BEHEMOTH_STOMP, 7s);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!me || !me->IsAlive())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BEHEMOTH_THUNDER:
                        DoCastVictim(SPELL_BEHEMOTH_THUNDER);
                        events.ScheduleEvent(EVENT_BEHEMOTH_THUNDER, 10s);
                        break;
                    case EVENT_BEHEMOTH_STOMP:
                        DoCastVictim(SPELL_BEHEMOTH_STOMP);
                        events.ScheduleEvent(EVENT_BEHEMOTH_STOMP, randtime(10s, 20s));
                        break;
                }
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
            else
            {
                if (Creature* target = me->FindNearestCreature(GetScorgeTargetEntry(), 10.0f))
                    AttackStart(target);
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scorge_behemothAI(creature);
    }
};

void AddSC_the_scarlet_enclave_c5()
{
    new npc_highlord_darion_mograine();
    new npc_the_lich_king_tirion_dawn();
    new npc_scorge_abomination();
    new npc_scorge_ghoul();
    new npc_scorge_warrior_of_frozen_wastes();
    new npc_scorge_behemoth();
}
