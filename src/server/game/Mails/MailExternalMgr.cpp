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
#include "ObjectMgr.h"
#include "Player.h"
#include "Mail.h"
#include "MailMgr.h"
#include "MailExternalMgr.h"
#include "Item.h"
#include "Log.h"

MailExternalMgr* MailExternalMgr::instance()
{
    static MailExternalMgr instance;
    return &instance;
}

void MailExternalMgr::Initialize()
{
    m_updateTimer = 1000;
}

void MailExternalMgr::Update(uint32 diff)
{
    m_updateTimer += diff;
    if (m_updateTimer >= MAIL_UPDATE_INTERVAL * MINUTE) // upd only 1 time in minute
    {
        _DoUpdate();
        m_updateTimer = 0;
    }
}

void MailExternalMgr::_DoUpdate()
{
    FMT_LOG_DEBUG("mailexternal", "External Mail> Sending mails in queue...");

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_GET_EXTERNAL_MAIL);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
        FMT_LOG_DEBUG("mailexternal", "External Mail> No mails in queue...");
        return;
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();
        ObjectGuid::LowType receiver_guid = fields[1].GetUInt32();
        std::string subject = fields[2].GetString();
        std::string body = fields[3].GetString();
        uint32 money = fields[4].GetUInt32();
        uint32 itemId = fields[5].GetUInt32();
        uint32 itemCount = fields[6].GetUInt32();

        std::vector<Item*> itemlist;
        if (itemId)
        {
            if (!sObjectMgr->GetItemTemplate(itemId))
                FMT_LOG_ERROR("mailexternal", "External Mail> Item entry {} from `mail_external` doesn't exist in DB, skipped.", itemId);
            else
            {
                if (Item* mailItem = Item::CreateItem(itemId, itemCount))
                {
                    itemlist.push_back(mailItem);
                    mailItem->SaveToDB(trans);
                    FMT_LOG_INFO("mailexternal", "External Mail> Adding {} of item with id {} for player_id {}", itemCount, itemId, receiver_guid);
                }
            }
        }

        sMailMgr->SendMailWithItemsByGUID(0, receiver_guid, MAIL_NORMAL, subject, body, money, itemlist);
        itemlist.clear();

        CharacterDatabasePreparedStatement* stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_DEL_EXTERNAL_MAIL);
        stmt2->setUInt32(0, id);
        trans->Append(stmt2);

        FMT_LOG_DEBUG("mailexternal", "External Mail> Mail sent");
    } while (result->NextRow());

    CharacterDatabase.CommitTransaction(trans);
    FMT_LOG_DEBUG("mailexternal", "External Mail> All Mails Sent...");
}
