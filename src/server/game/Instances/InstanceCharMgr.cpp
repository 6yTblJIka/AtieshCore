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

#include "DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "InstanceCharMgr.h"
#include "Log.h"

InstanceCharMgr::InstanceCharMgr()
{
    //m_updateTimer = 0;
    //_timer.Reset(MINUTE * 1000);    // upd only 1 time in minute
}

InstanceCharMgr* InstanceCharMgr::instance()
{
    static InstanceCharMgr instance;
    return &instance;
}

void InstanceCharMgr::Initialize()
{
    uint32 oldMSTime = getMSTime();

    m_instanceCharStore.clear();                                  // for reload case

    //                                                0    1     2       3       4
    QueryResult result = CharacterDatabase.Query("SELECT id, guid, map, difficulty, data FROM character_instance_completed");

    if (!result)
    {
        FMT_LOG_INFO("server.loading", ">> Loaded 0 Character Instance History. DB table `character_instance_completed` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();

        InstanceCharData pd;
        pd.guid = fields[1].GetUInt32();
        pd.m_mapID = fields[2].GetUInt32();
        pd.m_mapDifficulty = fields[3].GetUInt8();
        pd.data = fields[4].GetString();

        m_instanceCharStore[id] = pd;

        ++count;
    } while (result->NextRow());

    FMT_LOG_INFO("server.loading", ">> Loaded {} InstanceCharDatas in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

/*
void InstanceCharMgr::Update(uint32 diff)
{
    m_updateTimer += diff;
    if (m_updateTimer >= INSTANCE_CHAR_UPDATE_INTERVAL) // upd only each 1 sec
    {
        _timer.Update(m_updateTimer / 1000);
        if (_timer.Passed())
        {
            //_DoUpdate();
            _timer.Reset(MINUTE);
        }

        m_updateTimer = 0;
    }
}


void InstanceCharMgr::_DoUpdate()
{
}*/

void InstanceCharMgr::_AddPlayer(ObjectGuid::LowType plrGUID, uint32 mapID, uint8 mapDifficulty)
{
    InstanceCharData pd;
    pd.guid = plrGUID;
    pd.m_mapID = mapID;
    pd.m_mapDifficulty = mapDifficulty;
    //         1 2 3 4 5 6 7 8 9101112131415 - 15 encounters
    pd.data = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";

    // find max id
    uint32 new_id = 0;
    for (InstanceCharContainer::const_iterator itr = m_instanceCharStore.begin(); itr != m_instanceCharStore.end(); ++itr)
        if (itr->first > new_id)
            new_id = itr->first;

    // use next
    ++new_id;
    // add in active player list of event
    m_instanceCharStore[new_id] = pd;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_INSTANCE_WEB_SAVE);
    stmt->setUInt32(0, pd.guid);
    stmt->setUInt16(1, pd.m_mapID);
    stmt->setUInt8(2, pd.m_mapDifficulty);
    stmt->setString(3, pd.data);
    CharacterDatabase.Execute(stmt);
}

void InstanceCharMgr::HandlePlayerEnterInInstance(Player* player, uint32 mapID, uint8 difficulty)
{
    ObjectGuid::LowType pg = player->GetGUID().GetCounter();

    // stage 1 - check if player already exist in character_instance_completed
    bool existCharInData = false;
    for (InstanceCharContainer::const_iterator itr = m_instanceCharStore.begin(); itr != m_instanceCharStore.end(); ++itr)
        if (itr->second.guid == pg &&
            itr->second.m_mapID == mapID &&
            itr->second.m_mapDifficulty == difficulty)
            existCharInData = true;

    // stage 2 - if stage 1 failed (insert)
    if (!existCharInData)
        _AddPlayer(pg, mapID, difficulty);
}

std::string InstanceCharMgr::GetUpdatedData(std::string const data, uint8 encounterID, uint32 bossEntry)
{
    std::ostringstream saveStream;

    uint8 index = 0;
    bool corrected = false;
    bool correctedSecondStage = false;
    for (uint8 i = 0; i < (uint8)data.length(); i++)
    {
        // stage 1 - encounter = 1
        if (encounterID == 0)
        {
            if (i == 0)
                saveStream << bossEntry;
            else
            {
                if (!corrected)
                {
                    if (data[i] != ' ')
                        continue;
                    else
                    {
                        saveStream << data[i];
                        corrected = true;
                    }
                }
                else
                    saveStream << data[i];
            }
        }

        // stage 2 - encounter after some spaces ' '
        if (encounterID != 0)
        {
            if (!corrected)
            {
                if (data[i] == ' ')
                {
                    index++;
                    saveStream << data[i];
                    continue;
                }

                if (index != encounterID)
                    saveStream << data[i];
                else
                {
                    saveStream << bossEntry;
                    corrected = true;
                    continue;
                }
            }

            if (corrected && !correctedSecondStage)
            {
                if (data[i] != ' ')
                    continue;
                else
                    correctedSecondStage = true;
            }

            if (correctedSecondStage)
                saveStream << data[i];
        }
    }

    return saveStream.str();
}

void InstanceCharMgr::HandleDoneEncounterInInstance(std::vector<ObjectGuid::LowType> const& plrlist, uint32 mapID, uint8 difficulty, uint8 encounterID, uint32 bossEntry)
{
    if (!plrlist.empty()) // it's unreal, but possible to sent message with empty list
    {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        std::string data = "";

        for (InstanceCharContainer::iterator itr = m_instanceCharStore.begin(); itr != m_instanceCharStore.end(); ++itr)
        {
            for (std::vector<ObjectGuid::LowType>::const_iterator itr2 = plrlist.begin(); itr2 != plrlist.end(); ++itr2)
            {
                if (itr->second.guid == (*itr2) &&
                    itr->second.m_mapID == mapID &&
                    itr->second.m_mapDifficulty == difficulty)
                {
                    data = GetUpdatedData(itr->second.data, encounterID, bossEntry);
                    itr->second.data = data;
                    AddPlrForUpdate(data, (*itr2), trans, mapID, difficulty);
                }
            }
        }
        CharacterDatabase.CommitTransaction(trans);
    }
}

void InstanceCharMgr::AddPlrForUpdate(std::string data, ObjectGuid::LowType plrGUID, CharacterDatabaseTransaction& trans, uint32 mapID, uint8 difficulty)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_INSTANCE_WEB_DATA);
    stmt->setString(0, data);
    stmt->setUInt32(1, plrGUID);
    stmt->setUInt32(2, mapID);
    stmt->setUInt32(3, difficulty);
    trans->Append(stmt);
}
