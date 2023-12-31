/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "Transmogrification.h"
#include "Bag.h"
#include "Common.h"
#include "Config.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "DBCStructure.h"
#include "Define.h"
#include "Field.h"
#include "GameEventMgr.h"
#include "GossipDef.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Language.h"
#include "Log.h"
#include "Player.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "QueryResult.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Transaction.h"
#include "WorldSession.h"
#include <sstream>
#include <string>

#define GTS session->GetTrinityString

class CS_Transmogrification : public CreatureScript
{
public:
    CS_Transmogrification() : CreatureScript("Creature_Transmogrify") { }

    class TransmogAI : public ScriptedAI
    {
    public:
        TransmogAI(Creature* creature) : ScriptedAI(creature) {}

        struct Timed : public BasicEvent
        {
            // This timed event tries to fix modify money breaking gossip
            // This event closes the gossip menu and on the second player update tries to open the next menu
            Timed(Player* player, Creature* creature, uint32 sender, uint32 action) : guid(creature->GetGUID()), player(player), sender(sender), action(action), triggered(false)
            {
                CloseGossipMenuFor(player);
                player->m_Events.AddEvent(this, player->m_Events.CalculateTime(1ms));
            }

            bool Execute(uint64, uint32) override
            {
                if (!triggered)
                {
                    triggered = true;
                    player->m_Events.AddEvent(this, player->m_Events.CalculateTime(1ms));
                    return false;
                }
                if (Creature* creature = player->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_GOSSIP))
                    StaticGossipSelect(player, creature, sender, action);
                return true;
            }

            ObjectGuid guid;
            Player* player;
            uint32 sender;
            uint32 action;
            bool triggered;
        };

        bool OnGossipHello(Player* player) override
        {
            return OnGossipHello(player, me);
        }

        static bool OnGossipHello(Player* player, Creature* creature)
        {
            WorldSession* session = player->GetSession();
            if (sTransmogrification->EnableTransmogInfo)
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_HOW_TRANSMOG_WORKS), EQUIPMENT_SLOT_END + 9, 0);
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
            {
                if (const char* slotName = sTransmogrification->GetSlotName(slot, session))
                {
                    Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                    uint32 entry = newItem ? sTransmogrification->GetFakeEntry(newItem) : 0;
                    std::string icon = entry ? sTransmogrification->GetItemIcon(entry, 30, 30, -18, 0) : sTransmogrification->GetSlotIcon(slot, 30, 30, -18, 0);
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, icon + std::string(slotName), EQUIPMENT_SLOT_END, slot);
                }
            }
#ifdef PRESETS
            if (sTransmogrification->EnableSets)
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_MANAGE_SETS), EQUIPMENT_SLOT_END + 4, 0);
#endif
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_REMOVE_ALL_TRANSMOG), EQUIPMENT_SLOT_END + 2, 0, GTS(LANG_REMOVE_ALL_TRANSMOG_CONFIRM), 0, false);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_UPDATE_MENU), EQUIPMENT_SLOT_END + 1, 0);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 gossipListId) override
        {
            uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            return StaticGossipSelect(player, me, sender, action);
        }

        static bool StaticGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
        {
            ClearGossipMenuFor(player);
            WorldSession* session = player->GetSession();
            switch (sender)
            {
                case EQUIPMENT_SLOT_END: // Show items you can use
                    ShowTransmogItems(player, creature, action);
                    break;
                case EQUIPMENT_SLOT_END + 1: // Main menu
                    OnGossipHello(player, creature);
                    break;
                case EQUIPMENT_SLOT_END + 2: // Remove Transmogrifications
                    {
                        bool removed = false;
                        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                        {
                            if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                            {
                                if (!sTransmogrification->GetFakeEntry(newItem))
                                    continue;
                                sTransmogrification->DeleteFakeEntry(player, newItem);
                                removed = true;
                            }
                        }
                        if (removed)
                            session->SendAreaTriggerMessage("%s", GTS(LANG_ERR_UNTRANSMOG_OK));
                        else
                            session->SendNotification(LANG_ERR_UNTRANSMOG_NO_TRANSMOGS);
                        OnGossipHello(player, creature);
                    } break;
                case EQUIPMENT_SLOT_END + 3: // Remove Transmogrification from single item
                    {
                        if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, action))
                        {
                            if (sTransmogrification->GetFakeEntry(newItem))
                            {
                                sTransmogrification->DeleteFakeEntry(player, newItem);
                                session->SendAreaTriggerMessage("%s", GTS(LANG_ERR_UNTRANSMOG_OK));
                            }
                            else
                                session->SendNotification(LANG_ERR_UNTRANSMOG_NO_TRANSMOGS);
                        }
                        StaticGossipSelect(player, creature, EQUIPMENT_SLOT_END, action);
                    } break;
#ifdef PRESETS
                case EQUIPMENT_SLOT_END + 4: // Presets menu
                    {
                        if (!sTransmogrification->EnableSets)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }
                        if (sTransmogrification->EnableSetInfo)
                            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_HOW_SETS_WORKS), EQUIPMENT_SLOT_END + 10, 0);

                        if (!player->presetMap.empty())
                        {
                            for (PresetMapType::const_iterator it = player->presetMap.begin(); it != player->presetMap.end(); ++it)
                                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Misc_Statue_02:30:30:-18:0|t" + it->second.name, EQUIPMENT_SLOT_END + 6, it->first);

                            if (player->presetMap.size() < sTransmogrification->MaxSets)
                                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_SAVE_SET), EQUIPMENT_SLOT_END + 8, 0);
                        }
                        else
                            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_SAVE_SET), EQUIPMENT_SLOT_END + 8, 0);
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_BACK), EQUIPMENT_SLOT_END + 1, 0);
                        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    } break;
                case EQUIPMENT_SLOT_END + 5: // Use preset
                    {
                        if (!sTransmogrification->EnableSets)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }
                        // action = presetID

                        PresetMapType::const_iterator it = player->presetMap.find(action);
                        if (it != player->presetMap.end())
                        {
                            for (PresetslotMapType::const_iterator it2 = it->second.slotMap.begin(); it2 != it->second.slotMap.end(); ++it2)
                                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, it2->first))
                                    sTransmogrification->PresetTransmog(player, item, it2->second, it2->first);
                        }
                        StaticGossipSelect(player, creature, EQUIPMENT_SLOT_END + 6, action);
                    } break;
                case EQUIPMENT_SLOT_END + 6: // view preset
                    {
                        if (!sTransmogrification->EnableSets)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }
                        // action = presetID

                        PresetMapType::const_iterator it = player->presetMap.find(action);
                        if (it == player->presetMap.end())
                        {
                            StaticGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
                            return true;
                        }

                        for (PresetslotMapType::const_iterator it2 = it->second.slotMap.begin(); it2 != it->second.slotMap.end(); ++it2)
                            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, sTransmogrification->GetItemIcon(it2->second, 30, 30, -18, 0) + sTransmogrification->GetItemLink(it2->second, session), sender, action);

                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_USE_SET), EQUIPMENT_SLOT_END + 5, action, GTS(LANG_USE_SET_CONFIRM) + it->second.name, 0, false);
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_DELETE_SET), EQUIPMENT_SLOT_END + 7, action, GTS(LANG_DELETE_SET_CONFIRM) + it->second.name + "?", 0, false);
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_BACK), EQUIPMENT_SLOT_END + 4, 0);
                        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    } break;
                case EQUIPMENT_SLOT_END + 7: // Delete preset
                    {
                        if (!sTransmogrification->EnableSets)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }
                        // action = presetID

                        player->presetMap.erase(action);

                        StaticGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
                    } break;
                case EQUIPMENT_SLOT_END + 8: // Save preset
                    {
                        if (!sTransmogrification->EnableSets)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }

                        if (player->presetMap.size() >= sTransmogrification->MaxSets)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }

                        uint32 cost = 0;
                        bool canSave = false;
                        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                        {
                            if (!sTransmogrification->GetSlotName(slot, session))
                                continue;
                            if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                            {
                                uint32 entry = sTransmogrification->GetFakeEntry(newItem);
                                if (!entry)
                                    continue;
                                const ItemTemplate* temp = sObjectMgr->GetItemTemplate(entry);
                                if (!temp)
                                    continue;
                                if (!sTransmogrification->SuitableForTransmogrification(player, temp)) // no need to check?
                                    continue;
                                cost += sTransmogrification->GetSpecialPrice(temp);
                                canSave = true;
                                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, sTransmogrification->GetItemIcon(entry, 30, 30, -18, 0) + sTransmogrification->GetItemLink(entry, session), EQUIPMENT_SLOT_END + 8, 0);
                            }
                        }
                        if (canSave)
                            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_SAVE_SET), 0, 0, GTS(LANG_SAVE_SET_NAME), cost*sTransmogrification->SetCostModifier + sTransmogrification->SetCopperCost, true);
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_UPDATE_MENU), sender, action);
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_BACK), EQUIPMENT_SLOT_END + 4, 0);
                        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    } break;
                case EQUIPMENT_SLOT_END + 10: // Set info
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_BACK), EQUIPMENT_SLOT_END + 4, 0);
                        SendGossipMenuFor(player, sTransmogrification->SetNpcText, creature->GetGUID());
                    } break;
#endif
                case EQUIPMENT_SLOT_END + 9: // Transmog info
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_BACK), EQUIPMENT_SLOT_END + 1, 0);
                        SendGossipMenuFor(player, sTransmogrification->TransmogNpcText, creature->GetGUID());
                    } break;
                default: // Transmogrify
                    {
                        if (!sender && !action)
                        {
                            OnGossipHello(player, creature);
                            return true;
                        }
                        // sender = slot, action = display
                        TransmogTrinityStrings res = sTransmogrification->Transmogrify(player, ObjectGuid(HighGuid::Item, 0, action), sender);
                        if (res == LANG_ERR_TRANSMOG_OK)
                            session->SendAreaTriggerMessage("%s", GTS(LANG_ERR_TRANSMOG_OK));
                        else
                            session->SendNotification(res);
                        // OnGossipSelect(player, EQUIPMENT_SLOT_END, sender);
                        new Timed(player, creature, EQUIPMENT_SLOT_END, sender);
                    } break;
            }
            return true;
        }

#ifdef PRESETS
        bool OnGossipSelectCode(Player* player, uint32 /*menu_id*/, uint32 gossipListId, const char* code) override
        {
            uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            return StaticGossipSelectCode(player, me, sender, action, code);
        }

        bool StaticGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
        {
            ClearGossipMenuFor(player);
            if (sender || action)
                return true; // should never happen
            if (!sTransmogrification->EnableSets)
            {
                OnGossipHello(player);
                return true;
            }

            // Allow only alnum
            std::string name = code;
            static const char* allowedcharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz _.,'1234567890";
            if (!name.length() || name.find_first_not_of(allowedcharacters) != std::string::npos)
            {
                player->GetSession()->SendNotification(LANG_PRESET_ERR_INVALID_NAME);
                StaticGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
                return true;
            }

            int32 cost = 0;
            PresetslotMapType items;
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
            {
                if (!sTransmogrification->GetSlotName(slot, player->GetSession()))
                    continue;
                if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    uint32 entry = sTransmogrification->GetFakeEntry(newItem);
                    if (!entry)
                        continue;
                    const ItemTemplate* temp = sObjectMgr->GetItemTemplate(entry);
                    if (!temp)
                        continue;
                    if (!sTransmogrification->SuitableForTransmogrification(player, temp))
                        continue;
                    cost += sTransmogrification->GetSpecialPrice(temp);
                    items[slot] = entry;
                }
            }

            if (!items.empty())
            {
                // transmogrified items were found to be saved
                cost *= sTransmogrification->SetCostModifier;
                cost += sTransmogrification->SetCopperCost;

                if (!player->HasEnoughMoney(cost))
                {
                    player->GetSession()->SendNotification(LANG_ERR_TRANSMOG_NOT_ENOUGH_MONEY);
                }
                else
                {
                    uint8 presetID = sTransmogrification->MaxSets;
                    if (player->presetMap.size() < sTransmogrification->MaxSets)
                    {
                        for (uint8 i = 0; i < sTransmogrification->MaxSets; ++i) // should never reach over max
                        {
                            if (player->presetMap.find(i) == player->presetMap.end())
                            {
                                presetID = i;
                                break;
                            }
                        }
                    }

                    if (presetID < sTransmogrification->MaxSets)
                    {
                        // Make sure code doesnt mess up SQL!
                        player->presetMap[presetID].name = name;
                        player->presetMap[presetID].slotMap = items;

                        if (cost)
                            player->ModifyMoney(-cost);
                    }
                }
            }

            // OnGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
            new Timed(player, creature, EQUIPMENT_SLOT_END + 4, 0);
            return true;
        }
#endif

        static void ShowTransmogItems(Player* player, Creature* creature, uint8 slot) // Only checks bags while can use an item from anywhere in inventory
        {
            WorldSession* session = player->GetSession();
            Item* oldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (oldItem)
            {
                uint32 limit = 0;
                uint32 price = sTransmogrification->GetSpecialPrice(oldItem->GetTemplate());
                price *= sTransmogrification->ScaledCostModifier;
                price += sTransmogrification->CopperCost;
                std::ostringstream ss;
                ss << std::endl;
                if (sTransmogrification->RequireToken)
                    ss << std::endl << std::endl << sTransmogrification->TokenAmount << " x " << sTransmogrification->GetItemLink(sTransmogrification->TokenEntry, session);

                for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
                {
                    if (limit >= MAX_OPTIONS)
                        break;
                    Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                    if (!newItem)
                        continue;
                    if (!sTransmogrification->CanTransmogrifyItemWithItem(player, oldItem->GetTemplate(), newItem->GetTemplate()))
                        continue;
                    if (sTransmogrification->GetFakeEntry(oldItem) == newItem->GetEntry())
                        continue;
                    ++limit;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, sTransmogrification->GetItemIcon(newItem->GetEntry(), 30, 30, -18, 0) + sTransmogrification->GetItemLink(newItem, session), slot, newItem->GetGUID().GetCounter(), GTS(LANG_BIND_ITEM) + sTransmogrification->GetItemIcon(newItem->GetEntry(), 40, 40, -15, -10) + sTransmogrification->GetItemLink(newItem, session) + ss.str(), price, false);
                }

                for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    Bag* bag = player->GetBagByPos(i);
                    if (!bag)
                        continue;
                    for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                    {
                        if (limit >= MAX_OPTIONS)
                            break;
                        Item* newItem = player->GetItemByPos(i, j);
                        if (!newItem)
                            continue;
                        if (!sTransmogrification->CanTransmogrifyItemWithItem(player, oldItem->GetTemplate(), newItem->GetTemplate()))
                            continue;
                        if (sTransmogrification->GetFakeEntry(oldItem) == newItem->GetEntry())
                            continue;
                        ++limit;
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, sTransmogrification->GetItemIcon(newItem->GetEntry(), 30, 30, -18, 0) + sTransmogrification->GetItemLink(newItem, session), slot, newItem->GetGUID().GetCounter(), GTS(LANG_BIND_ITEM) + sTransmogrification->GetItemIcon(newItem->GetEntry(), 40, 40, -15, -10) + sTransmogrification->GetItemLink(newItem, session) + ss.str(), price, false);
                    }
                }
            }

            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_REMOVE), EQUIPMENT_SLOT_END + 3, slot, GTS(LANG_TRANSMOG_REMOVE_CONFIRM), 0, false);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_UPDATE_MENU), EQUIPMENT_SLOT_END, slot);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GTS(LANG_TRANSMOG_BACK), EQUIPMENT_SLOT_END + 1, 0);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new TransmogAI(creature);
    }
};

class PS_Transmogrification : public PlayerScript
{
public:
    PS_Transmogrification() : PlayerScript("PS_Transmogrification") { }

    void OnSave(Player* player) override
    {
        uint32 lowguid = player->GetGUID().GetCounter();
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        trans->PAppend("DELETE FROM `custom_transmogrification` WHERE `Owner` = %u", lowguid);
#ifdef PRESETS
        trans->PAppend("DELETE FROM `custom_transmogrification_sets` WHERE `Owner` = %u", lowguid);
#endif

        if (!player->transmogMap.empty())
        {
            // Only save items that are in inventory / bank / etc
            std::vector<ObjectGuid> items = sTransmogrification->GetItemList(player);
            for (std::vector<ObjectGuid>::const_iterator it = items.begin(); it != items.end(); ++it)
            {
                TransmogMapType::const_iterator it2 = player->transmogMap.find(*it);
                if (it2 == player->transmogMap.end())
                    continue;

                trans->PAppend("REPLACE INTO custom_transmogrification (GUID, FakeEntry, Owner) VALUES (%u, %u, %u)", it2->first.GetCounter(), it2->second, lowguid);
            }
        }

#ifdef PRESETS
        if (!player->presetMap.empty())
        {
            for (PresetMapType::const_iterator it = player->presetMap.begin(); it != player->presetMap.end(); ++it)
            {
                std::ostringstream ss;
                for (PresetslotMapType::const_iterator it2 = it->second.slotMap.begin(); it2 != it->second.slotMap.end(); ++it2)
                    ss << uint32(it2->first) << ' ' << it2->second << ' ';
                trans->PAppend("REPLACE INTO `custom_transmogrification_sets` (`Owner`, `PresetID`, `SetName`, `SetData`) VALUES (%u, %u, \"%s\", \"%s\")", lowguid, uint32(it->first), it->second.name.c_str(), ss.str().c_str());
            }
        }
#endif

        if (trans->GetSize()) // basically never false
            CharacterDatabase.CommitTransaction(trans);
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT GUID, FakeEntry FROM custom_transmogrification WHERE Owner = %u", player->GetGUID().GetCounter());

        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                ObjectGuid itemGUID(HighGuid::Item, 0, field[0].GetUInt32());
                uint32 fakeEntry = field[1].GetUInt32();
                // Only load items that are in inventory / bank / etc
                if (sObjectMgr->GetItemTemplate(fakeEntry) && player->GetItemByGuid(itemGUID))
                {
                    player->transmogMap[itemGUID] = fakeEntry;
                }
                else
                {
                    // Ignore, will be erased on next save.
                    // Additionally this can happen if an item was deleted from DB but still exists for the player
                    // FMT_LOG_ERROR("custom.transmog", "Item entry (Entry: {}, itemGUID: {}, playerGUID: {}) does not exist, ignoring.", fakeEntry, GUID_LOPART(itemGUID), player->GetGUID().ToString());
                    // CharacterDatabase.PExecute("DELETE FROM custom_transmogrification WHERE FakeEntry = %u", fakeEntry);
                }
            } while (result->NextRow());

            if (!player->transmogMap.empty())
            {
                for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                    {
                        player->SetVisibleItemSlot(slot, item);
                        if (player->IsInWorld())
                            item->SendUpdateToPlayer(player);
                    }
                }
            }
        }

#ifdef PRESETS
        if (sTransmogrification->EnableSets)
            sTransmogrification->LoadPlayerSets(player);
#endif
    }
};

class WS_Transmogrification : public WorldScript
{
public:
    WS_Transmogrification() : WorldScript("WS_Transmogrification") { }

    void OnConfigLoad(bool reload) override
    {
        sTransmogrification->LoadConfig(reload);
    }

    void OnStartup() override
    {
        FMT_LOG_INFO("custom.transmog", "Deleting non-existing transmogrification entries...");
        CharacterDatabase.DirectExecute("DELETE FROM custom_transmogrification WHERE NOT EXISTS (SELECT 1 FROM item_instance WHERE item_instance.guid = custom_transmogrification.GUID)");

#ifdef PRESETS
        // Clean even if disabled
        // Dont delete even if player has more presets than should
        CharacterDatabase.DirectExecute("DELETE FROM `custom_transmogrification_sets` WHERE NOT EXISTS(SELECT 1 FROM characters WHERE characters.guid = custom_transmogrification_sets.Owner)");
#endif
        sTransmogrification->LoadConfig(false);
    }
};

void AddSC_Transmogrification()
{
    new WS_Transmogrification();
    new PS_Transmogrification();
    new CS_Transmogrification();
}
