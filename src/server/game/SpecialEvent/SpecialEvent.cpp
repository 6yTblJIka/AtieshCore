/*
 * Copyright (C) 2018+ AtieshCore <https://at-wow.org/>
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

#include "GameTime.h"
#include "SpecialEvent.h"
#include "SpecialEventMgr.h"
#include "Log.h"
#include "ObjectAccessor.h"

SpecialEvent::SpecialEvent() : _eventId(0), _timer(0), _active(false), _enabled(true), _Repeatable(true), _noEventTime(0), _comment(""), _gameTimeNextEvent(0) { }

SpecialEvent::~SpecialEvent() { }

void SpecialEvent::Update(uint32 diff)
{
    if (!_enabled)
        return;

    _timer.Update(diff);
    if (_timer.Passed())
    {
        if (IsActiveTime())     // Event ends on time
            EndSpecialEvent(true);
        else                    // time to start for new Event
        {
            if (_Repeatable)
                StartSpecialEvent();
            else                // For Non-Repeatable Events just reset time
                _timer.Reset(_noEventTime);
        }
    }
}

void SpecialEvent::RegisterEvent(uint32 eventId)
{
    sSpecialEventMgr->AddEvent(eventId, this);
}

void SpecialEvent::RegisterZoneIdForEvent(uint32 zoneId)
{
    sSpecialEventMgr->AddZone(zoneId, this);
}

bool SpecialEvent::SetupSpecialEvent(bool enabled, bool active, bool repeatable, uint32 id, uint32 cooldownTimer, uint32 durationTimer, std::string comment)
{
    if (!id || !cooldownTimer || !durationTimer)
        return false;

    _enabled = enabled;
    _active = active;
    _Repeatable = repeatable;
    _eventId = id;
    _noEventTime = cooldownTimer * MINUTE * IN_MILLISECONDS;
    _EventTime = durationTimer * MINUTE * IN_MILLISECONDS;
    _comment = comment;

    _timer.Reset(_noEventTime);
    if (_Repeatable)
        _gameTimeNextEvent = uint32(GameTime::GetGameTime() + cooldownTimer * MINUTE);
    RegisterEvent(_eventId);

    return true;
}

void SpecialEvent::StartSpecialEvent()
{
    if (_active)
        return;

    _timer.Reset(_EventTime);
    _active = true;

    OnSpecialEventStart();

    // If Event Repeatable
    if (_Repeatable)
        _gameTimeNextEvent = uint32(GameTime::GetGameTime() + (_EventTime + _noEventTime) / IN_MILLISECONDS);
}

void SpecialEvent::EndSpecialEvent(bool endByTimer)
{
    if (!_active)
        return;

    _timer.Reset(_noEventTime);
    _active = false;

    OnSpecialEventEnd(endByTimer);

    // If Event Repeatable
    if (_Repeatable)
        _gameTimeNextEvent = uint32(GameTime::GetGameTime() + _noEventTime / IN_MILLISECONDS);
}
