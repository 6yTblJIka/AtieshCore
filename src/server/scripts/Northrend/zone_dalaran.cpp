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

/* Script Data Start
SDName: Dalaran
SDAuthor: WarHead, MaXiMiUS
SD%Complete: 99%
SDComment: For what is 63990+63991? Same function but don't work correct...
SDCategory: Dalaran
Script Data End */

#include "CellImpl.h"
#include "GameObject.h"
#include "GridNotifiersImpl.h"
#include "DatabaseEnv.h"
#include "MailMgr.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "ScriptMgr.h"
#include "Vehicle.h"
#include "WorldSession.h"
#include <G3D/Quat.h>

/*******************************************************
 * npc_mageguard_dalaran
 *******************************************************/

enum Spells
{
    SPELL_TRESPASSER_A                     = 54028,
    SPELL_TRESPASSER_H                     = 54029,

    SPELL_SUNREAVER_DISGUISE_FEMALE        = 70973,
    SPELL_SUNREAVER_DISGUISE_MALE          = 70974,
    SPELL_SILVER_COVENANT_DISGUISE_FEMALE  = 70971,
    SPELL_SILVER_COVENANT_DISGUISE_MALE    = 70972,
};

enum NPCs // All outdoor guards are within 35.0f of these NPCs
{
    NPC_APPLEBOUGH_A                       = 29547,
    NPC_SWEETBERRY_H                       = 29715,
    NPC_SILVER_COVENANT_GUARDIAN_MAGE      = 29254,
    NPC_SUNREAVER_GUARDIAN_MAGE            = 29255,
};

Position const AllianceEnterPoint = { 5757.f, 716.0f };
Position const AllianceTavernPoint = { 5732.f, 677.f };
Position const HordeEnterPoint = { 5861.f, 591.f };

struct npc_mageguard_dalaran : public ScriptedAI
{
    npc_mageguard_dalaran(Creature* creature) : ScriptedAI(creature)
    {
        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_NORMAL, true);
        creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);
        checkdist = 0.f;
        faction = me->GetEntry() == NPC_SILVER_COVENANT_GUARDIAN_MAGE ? TEAM_ALLIANCE : TEAM_HORDE;
        spellid = faction == TEAM_ALLIANCE ? SPELL_TRESPASSER_A : SPELL_TRESPASSER_H;
        if (me->IsWithinDist2d(AllianceEnterPoint.GetPositionX(), AllianceEnterPoint.GetPositionY(), 15.f))
        {
            checkdist = 7.0f;
            point = AllianceEnterPoint;
        }
        else if (me->IsWithinDist2d(AllianceTavernPoint.GetPositionX(), AllianceTavernPoint.GetPositionY(), 15.f))
        {
            checkdist = 5.0f;
            point = AllianceTavernPoint;
        }
        else if (me->IsWithinDist2d(HordeEnterPoint.GetPositionX(), HordeEnterPoint.GetPositionY(), 25.f))
        {
            checkdist = 10.0f;
            point = HordeEnterPoint;
        }

        if (checkdist)
            _events.ScheduleEvent(1, 1s);
    }

    void Reset() override { }

    void JustEngagedWith(Unit* /*who*/) override { }

    void AttackStart(Unit* /*who*/) override { }

    void UpdateAI(uint32 diff) override
    {
        if (!me || !me->IsAlive())
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case 1:
                {
                    std::vector<Player*> _players;
                    Trinity::AnyPlayerInObjectRangeCheck checker(me, checkdist);
                    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, _players, checker);
                    Cell::VisitWorldObjects(me, searcher, checkdist);
                    for (auto const& pointer : _players)
                    {
                        if (faction != pointer->GetTeamId())
                        {
                            if (pointer->IsAlive() &&
                                !pointer->IsGameMaster() &&
                                (pointer->GetPositionZ() < 670))
                            {
                                if (pointer->IsWithinDist2d(point.GetPositionX(), point.GetPositionY(), checkdist))
                                {
                                    if (// If player has Disguise aura for quest A Meeting With The Magister or An Audience With The Arcanist, do not teleport it away but let it pass
                                        pointer->HasAura(SPELL_SUNREAVER_DISGUISE_FEMALE) ||
                                        pointer->HasAura(SPELL_SUNREAVER_DISGUISE_MALE) ||
                                        pointer->HasAura(SPELL_SILVER_COVENANT_DISGUISE_FEMALE) ||
                                        pointer->HasAura(SPELL_SILVER_COVENANT_DISGUISE_MALE))
                                        continue;

                                    DoCast(pointer, spellid, TRIGGERED_FULL_MASK);
                                    me->CombatStop();
                                }
                            }
                        }
                    }
                    _players.clear();
                    _events.ScheduleEvent(1, 1s);
                    break;
                }
            }
        }
    }

private:
    TeamId faction;
    Position point;
    uint32 spellid;
    float checkdist;
    EventMap _events;
};

enum MinigobData
{
    ZONE_DALARAN            = 4395,

    SPELL_MANABONKED        = 61839,
    SPELL_TELEPORT_VISUAL   = 51347,
    SPELL_IMPROVED_BLINK    = 61995,

    EVENT_SELECT_TARGET     = 1,
    EVENT_LAUGH_1           = 2,
    EVENT_WANDER            = 3,
    EVENT_PAUSE             = 4,
    EVENT_CAST              = 5,
    EVENT_LAUGH_2           = 6,
    EVENT_BLINK             = 7,
    EVENT_DESPAWN           = 8,

    MAIL_MINIGOB_ENTRY      = 264,
    MAIL_DELIVER_DELAY_MIN  = 5*MINUTE,
    MAIL_DELIVER_DELAY_MAX  = 15*MINUTE
};

struct npc_minigob_manabonk : public ScriptedAI
{
    npc_minigob_manabonk(Creature* creature) : ScriptedAI(creature)
    {
        me->setActive(true);
    }

    void Reset() override
    {
        playerGuid = ObjectGuid();
        me->SetVisible(false);
        events.ScheduleEvent(EVENT_SELECT_TARGET, 1s);
    }

    void GetPlayersInDalaran(std::vector<Player*>& playerList) const
    {
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            if (Player* player = itr->GetSource()->ToPlayer())
                if (player->GetZoneId() == ZONE_DALARAN && !player->IsFlying() && !player->IsMounted() && !player->IsGameMaster())
                    playerList.push_back(player);
    }

    static Player* SelectTargetInDalaran(std::vector<Player*>& PlayerInDalaranList)
    {
        if (PlayerInDalaranList.empty())
            return nullptr;

        return Trinity::Containers::SelectRandomContainerElement(PlayerInDalaranList);
    }

    void SendMailToPlayer(Player* player) const
    {
        int16 deliverDelay = irand(MAIL_DELIVER_DELAY_MIN, MAIL_DELIVER_DELAY_MAX);
        sMailMgr->SendMailWithTemplateByGUID(me->GetEntry(), player->GetGUID().GetCounter(), MAIL_CREATURE, MAIL_MINIGOB_ENTRY, MAIL_CHECK_MASK_NONE, deliverDelay);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SELECT_TARGET:
                {
                    std::vector<Player*> PlayerInDalaranList;
                    GetPlayersInDalaran(PlayerInDalaranList);

                    // Increases chance of event based on player count in Dalaran (100 players or more = 100% else player count%)
                    if (PlayerInDalaranList.empty() || urand(1, 100) > PlayerInDalaranList.size())
                        me->AddObjectToRemoveList();

                    me->SetVisible(true);
                    DoCastSelf(SPELL_TELEPORT_VISUAL);
                    if (Player* player = SelectTargetInDalaran(PlayerInDalaranList))
                    {
                        playerGuid = player->GetGUID();
                        Position pos = player->GetPosition();
                        float dist = frand(10.0f, 30.0f);
                        float angle = frand(0.0f, 1.0f) * M_PI * 2.0f;
                        player->MovePositionToFirstCollision(pos, dist, angle);
                        me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
                    }
                    events.ScheduleEvent(EVENT_LAUGH_1, Seconds(2));
                    break;
                }
                case EVENT_LAUGH_1:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH_NO_SHEATHE);
                    events.ScheduleEvent(EVENT_WANDER, 3s);
                    break;
                case EVENT_WANDER:
                    me->GetMotionMaster()->MoveRandom(8);
                    events.ScheduleEvent(EVENT_PAUSE, 1min);
                    break;
                case EVENT_PAUSE:
                    me->GetMotionMaster()->MoveIdle();
                    events.ScheduleEvent(EVENT_CAST, 2s);
                    break;
                case EVENT_CAST:
                    if (Player* player = me->GetMap()->GetPlayer(playerGuid))
                    {
                        DoCast(player, SPELL_MANABONKED);
                        SendMailToPlayer(player);
                    }
                    else
                        me->AddObjectToRemoveList();

                    events.ScheduleEvent(EVENT_LAUGH_2, Seconds(8));
                    break;
                case EVENT_LAUGH_2:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH_NO_SHEATHE);
                    events.ScheduleEvent(EVENT_BLINK, 3s);
                    break;
                case EVENT_BLINK:
                    DoCastSelf(SPELL_IMPROVED_BLINK);
                    events.ScheduleEvent(EVENT_DESPAWN, 4s);
                    break;
                case EVENT_DESPAWN:
                    me->AddObjectToRemoveList();
                    break;
                default:
                    break;
            }
        }
    }

private:
    ObjectGuid playerGuid;
    EventMap events;
};

//28192
enum flyingmachine
{
    SPELL_SUMMON_MACHINE = 50860,
    SPELL_PASSENGER_BOARDED = 46598,
    SPELL_TELEPORT = 50987,
    SPELL_GO_FLY_TRIGGER = 51076,
    SPELL_SUMMON_PATROL = 51036,

    EVENT_GO_FLY = 1,
    EVENT_CAST_TELEPORT = 2,
    EVENT_GO_FLY_POINT_1 = 3,
    EVENT_GO_FLY_POINT_2 = 4,
    EVENT_GO_FLY_POINT_3 = 5,
    EVENT_DESPAWN_ME = 6,

    POINT_TELEPORT = 1,
    POINT_PATH_1 = 2,
    POINT_PATH_2 = 3,
    POINT_PATH_3 = 4
};

Position const flyAtPortal = { 5832.7f, 436.761f, 669.141f, 0.612513f };
Position const flyAt1point = { 5647.75f, 5229.6f, -72.69f, 0 };
Position const flyAt2point = { 5593.69f, 5187.79f, -72.69f, 0 };
Position const flyAt3point = { 5478.14f, 4971.84f, -22.4317f, 0 };
//X: 5183.344727 Y: 4788.114258 Z: 11.416546
// air 28229
class npc_archimage_flying_machine : public CreatureScript
{
public:
    npc_archimage_flying_machine() : CreatureScript("npc_archimage_flying_machine") {}

    struct npc_archimage_flying_machineAI : public ScriptedAI
    {
        npc_archimage_flying_machineAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            playerGuid = ObjectGuid();
            me->SetCanFly(true);
            events.Reset();
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_GO_FLY:
                {
                    DoCastSelf(SPELL_GO_FLY_TRIGGER);
                    std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                    {
                        init.MoveTo(flyAtPortal.GetPositionX(), flyAtPortal.GetPositionY(), flyAtPortal.GetPositionZ(), false);
                        init.SetFly();
                    };
                    me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_TELEPORT, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                    break;
                }
                case EVENT_CAST_TELEPORT:
                {
                    DoCast(SPELL_TELEPORT);
                    events.ScheduleEvent(EVENT_GO_FLY_POINT_1, 100ms);
                    break;
                }
                case EVENT_GO_FLY_POINT_1:
                {
                    for (uint8 i = 0; i < MAX_MOVE_TYPE; ++i)
                        me->SetSpeedRate(UnitMoveType(i), (me->GetSpeedRate(UnitMoveType(i)) * 2));

                    std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                    {
                        init.MoveTo(flyAt1point.GetPositionX(), flyAt1point.GetPositionY(), flyAt1point.GetPositionZ(), false);
                        init.SetFly();
                        init.SetWalk(false);
                    };
                    me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_PATH_1, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                    break;
                }
                case EVENT_GO_FLY_POINT_2:
                {
                    std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                    {
                        init.MoveTo(flyAt2point.GetPositionX(), flyAt2point.GetPositionY(), flyAt2point.GetPositionZ(), false);
                        init.SetFly();
                        init.SetWalk(false);
                    };
                    me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_PATH_2, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                    break;
                }
                case EVENT_GO_FLY_POINT_3:
                {
                    std::function<void(Movement::MoveSplineInit&)> initializer = [=](Movement::MoveSplineInit& init)
                    {
                        init.MoveTo(flyAt3point.GetPositionX(), flyAt3point.GetPositionY(), flyAt3point.GetPositionZ(), false);
                        init.SetFly();
                        init.SetWalk(false);
                    };
                    me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_PATH_3, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                    break;
                }
                case EVENT_DESPAWN_ME:
                {
                    me->SetVisible(false);
                    me->DespawnOrUnsummon();
                    break;
                }
                default:
                    break;
                }
            }
        }

        void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
        {
            if (!caster->ToPlayer())
                return;

            Player* player = caster->ToPlayer();

            if (spellInfo->Id == SPELL_PASSENGER_BOARDED)
                events.ScheduleEvent(EVENT_GO_FLY, 100ms);

            playerGuid = player->GetGUID();
        }

        void MovementInform(uint32 movementType, uint32 pointId) override
        {
            if (movementType == POINT_MOTION_TYPE)
            {
                switch (pointId)
                {
                case POINT_TELEPORT:
                {
                    events.ScheduleEvent(EVENT_CAST_TELEPORT, 100ms);
                    break;
                }
                case POINT_PATH_1:
                {
                    DoCast(SPELL_SUMMON_PATROL);
                    events.ScheduleEvent(EVENT_GO_FLY_POINT_2, 100ms);
                    break;
                }
                case POINT_PATH_2:
                {
                    if (Player* player = me->GetMap()->GetPlayer(playerGuid))
                        player->_ExitVehicle();
                    events.ScheduleEvent(EVENT_GO_FLY_POINT_3, 100ms);
                    break;
                }
                case POINT_PATH_3:
                {
                    events.ScheduleEvent(EVENT_DESPAWN_ME, 3s);
                    break;
                }
                default:
                    break;
                }
            }
        }

    private:
        EventMap events;
        ObjectGuid playerGuid;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_archimage_flying_machineAI(creature);
    }
};

// 28160
class npc_archimage_Pentarus : public CreatureScript
{
public:
    npc_archimage_Pentarus() : CreatureScript("npc_archimage_Pentarus") {}

    struct npc_archimage_PentarusAI : public ScriptedAI
    {
        npc_archimage_PentarusAI(Creature* creature) : ScriptedAI(creature) {}

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
        {
            CloseGossipMenuFor(player);
            player->CastSpell(player, 50860);
            Talk(0);
            QuaternionData rot = QuaternionData::fromEulerAnglesZYX(1.61251f, 0.f, 0.f);
            WorldObject* obj = nullptr;
            if (me)
                obj = me;
            obj->SummonGameObject(190488, Position(5832.7f, 436.761f, 669.141f, 1.61251f), rot, Minutes(2), GOSummonType(GO_SUMMON_TIMED_OR_CORPSE_DESPAWN));
            return true;
        }

    private:
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_archimage_PentarusAI(creature);
    }
};

void AddSC_dalaran()
{
    RegisterCreatureAI(npc_mageguard_dalaran);
    RegisterCreatureAI(npc_minigob_manabonk);
    new npc_archimage_flying_machine();
    new npc_archimage_Pentarus();
}
