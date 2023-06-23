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

#ifndef _INSTANCE_CHAR_H
#define _INSTANCE_CHAR_H

#include "Common.h"
#include "DatabaseEnvFwd.h"
#include "ObjectGuid.h"

class Player;

struct InstanceCharData
{
    ObjectGuid::LowType guid;
    uint32 m_mapID;
    uint8 m_mapDifficulty;
    std::string data;
};
typedef std::unordered_map<uint32, InstanceCharData> InstanceCharContainer;

enum InstanceCharTimers
{
    INSTANCE_CHAR_UPDATE_INTERVAL = 1000 // 1 sec
};

class TC_GAME_API InstanceCharMgr
{
private:
    InstanceCharMgr();
    ~InstanceCharMgr() { m_instanceCharStore.clear(); };

public:
    static InstanceCharMgr* instance();

    void Initialize();

    //void Update(uint32 diff);

    void HandlePlayerEnterInInstance(Player* player, uint32 mapID, uint8 difficulty);
    void HandleDoneEncounterInInstance(std::vector<ObjectGuid::LowType> const& plrlist, uint32 mapID, uint8 difficulty, uint8 encounterID, uint32 bossEntry);

protected:
    //void _DoUpdate();
    void _AddPlayer(ObjectGuid::LowType plrGUID, uint32 mapID, uint8 mapDifficulty);

    std::string GetUpdatedData(std::string const data, uint8 encounterID, uint32 bossEntry);
    void AddPlrForUpdate(std::string data, ObjectGuid::LowType plrGUID, CharacterDatabaseTransaction& trans, uint32 mapID, uint8 difficulty);

private:
    //uint32 m_updateTimer;
    //TimeTracker _timer;
    InstanceCharContainer m_instanceCharStore;
};

#define sInstanceCharMgr InstanceCharMgr::instance()

#endif // _INSTANCE_CHAR_H
