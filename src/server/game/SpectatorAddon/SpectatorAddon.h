/*
* Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Unit.h"
#include "WorldSession.h"

#define SPECTATOR_ADDON_SPELL_INTERUPTED 99999 // specific addons
#define SPECTATOR_ADDON_SPELL_CANCELED 99998   // numbers

class SpectatorAddonMsg {
public:
    SpectatorAddonMsg();

    void SetPlayer(std::string _player)     { player = _player;  EnableFlag(SPECTATOR_PREFIX_PLAYER); }
    void SetStatus(bool _isAlive)           { isAlive = _isAlive; EnableFlag(SPECTATOR_PREFIX_STATUS); }
    void SetClass(uint8 _class)             { pClass = _class;   EnableFlag(SPECTATOR_PREFIX_CLASS); }
    void SetTarget(std::string _target)     { target = _target;  EnableFlag(SPECTATOR_PREFIX_TARGET); }
    void SetTeam(uint32 _team)              { team = _team;    EnableFlag(SPECTATOR_PREFIX_TEAM); }

    void SetMaxHP(uint16 hp)                { maxHP = hp; EnableFlag(SPECTATOR_PREFIX_MAXHP); }
    void SetCurrentHP(uint16 hp)            { currHP = hp; EnableFlag(SPECTATOR_PREFIX_CURHP); }
    void SetMaxPower(uint16 power)          { maxPower = power; EnableFlag(SPECTATOR_PREFIX_MAXPOWER); }
    void SetCurrentPower(uint16 power)      { currPower = power; EnableFlag(SPECTATOR_PREFIX_CURPOWER); }
    void SetPowerType(Powers power)         { powerType = power; EnableFlag(SPECTATOR_PREFIX_POWERTYPE); }

    void CastSpell(uint32 _spellId, uint32 _castTime) { spellId = _spellId; castTime = _castTime; EnableFlag(SPECTATOR_PREFIX_SPELL); }
    void CancelSpell(uint32 _spellId) { spellId = _spellId; EnableFlag(SPECTATOR_PREFIX_CANCEL_SPELL); }
    void CreateAura(ObjectGuid _caster, uint32 _spellId, bool _isDebuff, uint8 _type, int32 _duration, int32 _expire, uint16 _stack, bool _isRemove);
    void RemoveAura(ObjectGuid _caster, uint32 _spellId, bool _isDebuff, uint8 _type, int32 _duration, int32 _expire, uint16 _stack, bool _isRemove);

    static bool SendPacket(SpectatorAddonMsg msg, ObjectGuid receiver);
    bool SendPacket(ObjectGuid receiver);

    std::string GetMsgData();

    bool isFilledIn(uint8 prefix) { return prefixFlags[prefix]; }

    static bool CanSandAura(uint32 auraID);
private:

    void EnableFlag(uint8 prefix) { prefixFlags[prefix] = true; }
    std::string player;
    bool isAlive;
    std::string target;
    uint8 pClass;

    uint16 maxHP;
    uint16 maxPower;
    uint16 currHP;
    uint16 currPower;
    Powers powerType;

    uint32 spellId;
    uint32 castTime;

    uint32 team;

    // aura data
    ObjectGuid aCaster;
    uint32 aSpellId;
    bool aIsDebuff;
    uint8 aType;
    int32 aDuration;
    int32 aExpire;
    uint16 aStack;
    bool aRemove;
    uint32 aurastack;

    bool prefixFlags[SPECTATOR_PREFIX_COUNT];
};
