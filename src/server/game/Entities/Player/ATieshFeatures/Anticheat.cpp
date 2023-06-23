/*
 * Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
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

#include "AccountMgr.h"
#include "DBCStoresMgr.h"
#include "GameTime.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
#include "Player.h"
#include "Realm.h"
#include "Vehicle.h"
#include "World.h"
#include "WorldSession.h"

// Anticheat System
void Player::SetUnderACKmount()
{
    m_mountTimer = 3000;
    m_ACKmounted = true;
}

void Player::SetRootACKUpd(uint32 delay)
{
    m_rootUpdTimer = 1500 + delay;
    m_rootUpd = true;
}

void Player::UpdateMovementInfo(MovementInfo const& movementInfo)
{
    SetLastMoveClientTimestamp(movementInfo.time);
    SetLastMoveServerTimestamp(GameTime::GetGameTimeMS());
}

void Player::StartWaitingLandOrSwimOpcode()
{
    m_antiNoFallDmgTimer = 3000;
    m_antiNoFallDmg = true;
}

bool Player::CheckOnFlyHack()
{
    if (sWorld->isAreaDisabledForAC(GetAreaId()))
        return true;

    if (IsCanFlybyServer())
        return true;

    if (ToUnit()->IsFalling() || IsFalling())
        return true;

    if (IsFlying() && !CanFly()) // kick flyhacks
    {
        FMT_LOG_INFO("anticheat", "Player::CheckMovementInfo :  FlyHack Detected for Account id : {}, Player {}", GetSession()->GetAccountId(), GetName());
        FMT_LOG_INFO("anticheat", "Player::========================================================");
        FMT_LOG_INFO("anticheat", "Player IsFlying but CanFly is false");

        sWorld->SendGMText(LANG_GM_ANNOUNCE_AFH_CANFLYWRONG, GetName().c_str());
        AccountMgr::RecordAntiCheatLog(GetSession()->GetAccountId(), GetName().c_str(), GetDescriptionACForLogs(1), GetPositionACForLogs(), int32(realm.Id.Realm));
        return false;
    }

    if (IsFlying() || IsGravityDisabled() || IsInFlight())
        return true;

    if (GetTransport() || GetVehicle() || GetVehicleKit())
        return true;

    if (HasAuraType(SPELL_AURA_CONTROL_VEHICLE))
        return true;

    if (HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
        return true;

    if (HasUnitState(UNIT_STATE_IGNORE_ANTISPEEDHACK))
        return true;

    if (GetAreaId() == 4859) // Area: 4859 (Frozen Throne)
        return true;

    if (UnderACKmount())
        return true;

    if (IsSkipOnePacketForASH())
        return true;

    Position npos = GetPosition();
    float pz = npos.GetPositionZ();
    if (!IsInWater() && HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING))
    {
        float waterlevel = GetMap()->GetWaterLevel(npos.GetPositionX(), npos.GetPositionY()); // water walking
        bool hovergaura = HasAuraType(SPELL_AURA_WATER_WALK) || HasAuraType(SPELL_AURA_HOVER);
        if (waterlevel && (pz - waterlevel) <= (hovergaura ? GetCollisionHeight() + 1.5f + GetHoverOffset() : GetCollisionHeight() + GetHoverOffset()))
            return true;

        FMT_LOG_INFO("anticheat", "Player::CheckOnFlyHack :  FlyHack Detected for Account id : {}, Player {}", GetSession()->GetAccountId(), GetName());
        FMT_LOG_INFO("anticheat", "Player::========================================================");
        FMT_LOG_INFO("anticheat", "Player::CheckOnFlyHack :  Player has a MOVEMENTFLAG_SWIMMING, but not in water");

        sWorld->SendGMText(LANG_GM_ANNOUNCE_AFK_SWIMMING, GetName().c_str());
        AccountMgr::RecordAntiCheatLog(GetSession()->GetAccountId(), GetName().c_str(), GetDescriptionACForLogs(2), GetPositionACForLogs(), int32(realm.Id.Realm));
        return false;
    }
    else
    {
        if (HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING))
            return true;

        float z = GetMap()->GetHeight(GetPhaseMask(), npos.GetPositionX(), npos.GetPositionY(), pz + GetCollisionHeight() + 0.5f, true, 50.0f); // smart flyhacks -> SimpleFly
        float diff = pz - z;
        if (diff > 6.8f)
            if (diff > 6.8f + GetHoverOffset()) // better calculate the second time for false situations, but not call GetHoverOffset everytime (economy resource)
            {
                float waterlevel = GetMap()->GetWaterLevel(npos.GetPositionX(), npos.GetPositionY()); // water walking
                if (waterlevel && waterlevel + GetCollisionHeight() + GetHoverOffset() > pz)
                    return true;

                float cx, cy, cz;
                GetTheClosestPoint(cx, cy, cz, 0.5, pz, 6.8f); // first check
                if (pz - cz > 6.8f)
                {
                    // check dynamic collision for transport (TODO navmesh for transport map)
                    GetMap()->getObjectHitPos(GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + GetCollisionHeight(), cx, cy, cz + GetCollisionHeight(), cx, cy, cz, -GetCollisionHeight());

                    if (pz - cz > 6.8f)
                    {
                        FMT_LOG_INFO("anticheat", "Player::CheckOnFlyHack :  FlyHack Detected for Account id : {}, Player {}", GetSession()->GetAccountId(), GetName());
                        FMT_LOG_INFO("anticheat", "Player::========================================================");
                        FMT_LOG_INFO("anticheat", "Player::CheckOnFlyHack :  playerZ = {}", pz);
                        FMT_LOG_INFO("anticheat", "Player::CheckOnFlyHack :  normalZ = {}", z);
                        FMT_LOG_INFO("anticheat", "Player::CheckOnFlyHack :  checkz = {}", cz);
                        sWorld->SendGMText(LANG_GM_ANNOUNCE_AFH, GetName().c_str());
                        AccountMgr::RecordAntiCheatLog(GetSession()->GetAccountId(), GetName().c_str(), GetDescriptionACForLogs(3, pz, z), GetPositionACForLogs(), int32(realm.Id.Realm));
                        return false;
                    }
                }
            }
    }

    return true;
}

bool Player::CheckMovementInfo(MovementInfo const& movementInfo, bool jump)
{
    if (!sWorld->customGetBoolConfig(CONFIG_ANTICHEAT_SPEEDHACK_ENABLED))
        return true;

    if (sWorld->isAreaDisabledForAC(GetAreaId()))
        return true;

    uint32 oldctime = GetLastMoveClientTimestamp();
    if (oldctime)
    {
        if (ToUnit()->IsFalling() || IsInFlight())
            return true;

        bool vehicle = false;
        if (GetVehicleKit() && GetVehicleKit()->GetBase())
            vehicle = true;

        if (GetVehicle())
            return true;

        if (!IsControlledByPlayer())
            return true;

        if (HasUnitState(UNIT_STATE_IGNORE_ANTISPEEDHACK))
            return true;

        if (IsSkipOnePacketForASH())
        {
            SetSkipOnePacketForASH(false);
            return true;
        }

        bool transportflag = movementInfo.GetMovementFlags() & MOVEMENTFLAG_ONTRANSPORT || HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        float x, y, z;
        Position npos;

        // Position coords for new point
        if (!transportflag)
            npos = movementInfo.pos;
        else
            npos = movementInfo.transport.pos;

        // Position coords for previous point (old)
        //         Just CheckMovementInfo are calling before player change UnitMovementFlag MOVEMENTFLAG_ONTRANSPORT
        if (transportflag)
        {
            if (GetTransOffsetX() == 0.f) // if it elevator or fist step - player can have zero this coord
                return true;

            x = GetTransOffsetX();
            y = GetTransOffsetY();
            z = GetTransOffsetZ();
        }
        else
            GetPosition(x, y, z);

        if (sWorld->customGetBoolConfig(CONFIG_ANTICHEAT_IGNORE_CONTROL_MOVEMENT_ENABLED))
        {
            if (HasUnitState(UNIT_STATE_ROOT) && !UnderACKRootUpd())
            {
                bool unrestricted = npos.GetPositionX() != x || npos.GetPositionY() != y;
                if (unrestricted)
                {
                    FMT_LOG_INFO("anticheat", "CheckMovementInfo :  Ignore controll Hack detected for Account id : {}, Player {}", GetSession()->GetAccountId(), GetName());
                    sWorld->SendGMText(LANG_GM_ANNOUNCE_MOVE_UNDER_CONTROL, GetSession()->GetAccountId(), GetName().c_str());
                    AccountMgr::RecordAntiCheatLog(GetSession()->GetAccountId(), GetName().c_str(), GetDescriptionACForLogs(4), GetPositionACForLogs(), int32(realm.Id.Realm));
                    return false;
                }
            }
        }

        float flyspeed = 0.f;
        float distance, runspeed, difftime, normaldistance, delay, diffPacketdelay;
        uint32 ptime;
        std::string mapname = GetMap()->GetMapName();

        // calculate distance - don't use func, because x,z can be offset transport coords
        distance = sqrt((npos.GetPositionY() - y) * (npos.GetPositionY() - y) + (npos.GetPositionX() - x) * (npos.GetPositionX() - x));

        if (!jump && !CanFly() && !isSwimming() && !transportflag)
        {
            float diffz = fabs(movementInfo.pos.GetPositionZ() - z);
            float tanangle = distance / diffz;

            if (movementInfo.pos.GetPositionZ() > z &&
                diffz > 1.87f &&
                tanangle < 0.57735026919f) // 30 degrees
            {
                FMT_LOG_INFO("anticheat", "Player::CheckMovementInfo :  Climb-Hack detected for Account id : {}, Player {}, diffZ = {}, distance = {}, angle = {}, Map = {}, mapId = {}, X = {}, Y = {}, Z = {}",
                    GetSession()->GetAccountId(), GetName(), diffz, distance, tanangle, mapname, GetMapId(), x, y, z);
                sWorld->SendGMText(LANG_GM_ANNOUNCE_WALLCLIMB, GetSession()->GetAccountId(), GetName().c_str(), diffz, distance, tanangle, mapname.c_str(), GetMapId(), x, y, z);
                AccountMgr::RecordAntiCheatLog(GetSession()->GetAccountId(), GetName().c_str(), GetDescriptionACForLogs(5, diffz, distance), GetPositionACForLogs(), int32(realm.Id.Realm));
                return false;
            }
        }

        uint32 oldstime = GetLastMoveServerTimestamp();
        uint32 stime = GameTime::GetGameTimeMS();
        uint32 ping;
        ptime = movementInfo.time;

        if (!vehicle)
            runspeed = GetSpeed(MOVE_RUN);
        else
            runspeed = GetVehicleKit()->GetBase()->GetSpeed(MOVE_RUN);

        if (isSwimming())
        {
            if (!vehicle)
                runspeed = GetSpeed(MOVE_SWIM);
            else
                runspeed = GetVehicleKit()->GetBase()->GetSpeed(MOVE_SWIM);
        }

        if (IsFlying() || CanFly())
        {
            if (!vehicle)
                flyspeed = GetSpeed(MOVE_FLIGHT);
            else
                flyspeed = GetVehicleKit()->GetBase()->GetSpeed(MOVE_FLIGHT);
        }

        if (flyspeed > runspeed)
            runspeed = flyspeed;

        delay = ptime - oldctime;
        diffPacketdelay = 10000000 - delay;

        if (oldctime > ptime)
        {
            FMT_LOG_INFO("anticheat", "oldctime > ptime");
            delay = 0;
        }
        diffPacketdelay = diffPacketdelay * 0.0000000001f;
        difftime = delay * 0.001f + diffPacketdelay;

        // if movetime faked and lower, difftime should be with "-"
        normaldistance = (runspeed * difftime) + 0.002f; // 0.002f a little safe temporary hack
        if (UnderACKmount())
            normaldistance += 20.0f;
        if (distance < normaldistance)
            return true;

        ping = uint32(diffPacketdelay * 10000.f);

        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  SpeedHack Detected for Account id : {}, Player {}", GetSession()->GetAccountId(), GetName());
        FMT_LOG_INFO("anticheat", "Unit::========================================================");
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldX = {}", x);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldY = {}", y);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  newX = {}", npos.GetPositionX());
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  newY = {}", npos.GetPositionY());
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  packetdistance = {}", distance);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  available distance = {}", normaldistance);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldStime = {}", oldstime);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldCtime = {}", oldctime);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  serverTime = {}", stime);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  packetTime = {}", ptime);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  diff delay between old ptk and current pkt = {}", diffPacketdelay);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  FullDelay = {}", delay / 1000.f);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  difftime = {}", difftime);
        FMT_LOG_INFO("anticheat", "Unit::CheckMovementInfo :  ping = {}", ping);

        sWorld->SendGMText(LANG_GM_ANNOUNCE_ASH, GetName().c_str(), normaldistance, distance);
        AccountMgr::RecordAntiCheatLog(GetSession()->GetAccountId(), GetName().c_str(), GetDescriptionACForLogs(0, distance, normaldistance), GetPositionACForLogs(), int32(realm.Id.Realm));
    }
    else
        return true;

    return false;
}

std::string Player::GetDescriptionACForLogs(uint8 type, float param1, float param2) const
{
    std::string str = "";
    switch (type)
    {        
        case 0: // ASH
        {
            str = fmt::format("AntiSpeedHack: distance from packet =  {}, available distance = {}", param1, param2);
            break;
        }
        case 1: // AFH - IsFlying but CanFly is false
        {
            str = "AntiFlyHack: Player IsFlying but CanFly is false";
            break;
        }
        case 2: // AFH - Player has a MOVEMENTFLAG_SWIMMING, but not in water
        {
            str = "AntiFlyHack: Player has a MOVEMENTFLAG_SWIMMING, but not in water";
            break;
        }
        case 3: // AFH - just z checks (smaughack)
        {
            str = fmt::format("AntiFlyHack: Player::CheckOnFlyHack : playerZ = {}, but normalZ = {}", param1, param2);
            break;
        }
        case 4: // Ignore control Hack
        {
            str = "Ignore controll Hack detected";
            break;
        }
        case 5: // Climb-Hack
        {
            str = fmt::format("Climb-Hack detected , diffZ =  {}, distance = {}", param1, param2);
            break;
        }
        case 6: // doublejumper
        {
            str = "Double-jump detected";
            break;
        }
        case 7: // fakejumper
        {
            str = "FakeJumper detected";
            break;
        }
        case 8: // fakeflying
        {
            str = "FakeFlying mode detected";
            break;
        }
        case 9: // NoFallingDmg
        {
            str = "NoFallingDamage mode detected";
            break;
        }
        default:
            break;
    }
    return str;
}

std::string Player::GetPositionACForLogs() const
{
    uint32 areaId = GetAreaId();
    std::string areaName = "Unknown";
    std::string zoneName = "Unknown";
    if (AreaTableDBC const* area = sDBCStoresMgr->GetAreaTableDBC(areaId))
    {
        int locale = GetSession()->GetSessionDbcLocale();
        areaName = area->AreaName[locale];
        if (AreaTableDBC const* zone = sDBCStoresMgr->GetAreaTableDBC(area->ParentAreaID))
            zoneName = zone->AreaName[locale];
    }

    return fmt::format("Map: {} ({}) Area: {} ({}) Zone: {} XYZ: {} {} {}", GetMapId(), FindMap() ? FindMap()->GetMapName() : "Unknown", areaId, areaName.c_str(), zoneName.c_str(), GetPositionX(), GetPositionY(), GetPositionZ());
}
