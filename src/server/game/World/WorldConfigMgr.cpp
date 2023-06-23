/*
 * Copyright (C) since 2016 AtieshCore <https://at-wow.org/>
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

#include "WorldConfigMgr.h"
#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "StringConvert.h"

WorldConfig* WorldConfig::instance()
{
    static WorldConfig instance;
    return &instance;
}

void WorldConfig::AddOption(WorldConfigType type, uint32 IDinTypeGroup, std::string const& defaultValue, std::string const& value)
{
    switch (type)
    {
        case WorldConfigType::GAME_CONFIG_TYPE_BOOL:
        {
            Optional<bool> boolValDefault = Trinity::StringTo<bool>(defaultValue);
            Optional<bool> boolVal = Trinity::StringTo<bool>(value);
            AddBoolOption(IDinTypeGroup, value.empty() ? *boolValDefault : *boolVal);
            break;
        }
        case WorldConfigType::GAME_CONFIG_TYPE_INT:
            AddIntOption(IDinTypeGroup, value.empty() ? (uint32)std::stoi(defaultValue) : (uint32)std::stoi(value));
            break;
        case WorldConfigType::GAME_CONFIG_TYPE_FLOAT:
            AddFloatOption(IDinTypeGroup, value.empty() ? std::stof(defaultValue) : std::stof(value));
            break;
        case WorldConfigType::GAME_CONFIG_TYPE_RATES:
            AddRateOption(IDinTypeGroup, value.empty() ? std::stof(defaultValue) : std::stof(value));
            break;
        default:
            ABORT();
            break;
    }
}

void WorldConfig::AddCustomOption(WorldConfigType type, uint32 IDinTypeGroup, std::string const& defaultValue, std::string const& value)
{
    switch (type)
    {
        case WorldConfigType::GAME_CONFIG_TYPE_BOOL:
        {
            Optional<bool> boolValDefault = Trinity::StringTo<bool>(defaultValue);
            Optional<bool> boolVal = Trinity::StringTo<bool>(value);
            CustomAddBoolOption(IDinTypeGroup, value.empty() ? *boolValDefault : *boolVal);
            break;
        }
        case WorldConfigType::GAME_CONFIG_TYPE_INT:
            CustomAddIntOption(IDinTypeGroup, value.empty() ? (uint32)std::stoi(defaultValue) : (uint32)std::stoi(value));
            break;
        case WorldConfigType::GAME_CONFIG_TYPE_FLOAT:
            CustomAddFloatOption(IDinTypeGroup, value.empty() ? std::stof(defaultValue) : std::stof(value));
            break;
        case WorldConfigType::GAME_CONFIG_TYPE_RATES:
            CustomAddRateOption(IDinTypeGroup, value.empty() ? std::stof(defaultValue) : std::stof(value));
            break;
        default:
            ABORT();
            break;
    }
}

void WorldConfig::Load()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.PQuery("SELECT `OptionName`, `Type`, `IDInTypeGroup`, `DefaultValue`, `CustomValue` FROM world_config");
    if (!result)
    {
        FMT_LOG_INFO("server.loading", ">> Loaded 0 world config options. DB table `world_config` is empty.");
        return;
    }

    uint32 count = 0;

    auto GetTypeString = [](std::string optionType) -> WorldConfigType
    {
        if (optionType == "bool")
            return WorldConfigType::GAME_CONFIG_TYPE_BOOL;
        else if (optionType == "int")
            return WorldConfigType::GAME_CONFIG_TYPE_INT;
        else if (optionType == "float")
            return WorldConfigType::GAME_CONFIG_TYPE_FLOAT;
        else if (optionType == "rate")
            return WorldConfigType::GAME_CONFIG_TYPE_RATES;
        else
            return WorldConfigType::GAME_CONFIG_TYPE_UNKNOWN;
    };

    do
    {
        Field* fields = result->Fetch();

        std::string const& optionName   = fields[0].GetString();
        std::string const& optionType   = fields[1].GetString();
        uint32 const& IDinTypeGroup     = fields[2].GetUInt32();
        std::string const& defaultValue = fields[3].GetString();
        std::string const& customValue  = fields[4].GetString();

        auto _type = GetTypeString(optionType);

        if (_type == WorldConfigType::GAME_CONFIG_TYPE_UNKNOWN)
        {
            FMT_LOG_ERROR("config", "> Don't support type ({}) for option ({})", optionType, optionName);
            continue;
        }

        AddOption(_type, IDinTypeGroup, defaultValue, customValue);

        count++;

    } while (result->NextRow());

    FMT_LOG_INFO("server.loading", ">> Loaded {} WorldConfigs in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WorldConfig::LoadCustom()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.PQuery("SELECT `OptionName`, `Type`, `IDInTypeGroup`, `DefaultValue`, `CustomValue` FROM world_custom_config");
    if (!result)
    {
        FMT_LOG_INFO("server.loading", ">> Loaded 0 world custom config options. DB table `world_custom_config` is empty.");
        return;
    }

    uint32 count = 0;

    auto GetTypeString = [](std::string optionType) -> WorldConfigType
    {
        if (optionType == "bool")
            return WorldConfigType::GAME_CONFIG_TYPE_BOOL;
        else if (optionType == "int")
            return WorldConfigType::GAME_CONFIG_TYPE_INT;
        else if (optionType == "float")
            return WorldConfigType::GAME_CONFIG_TYPE_FLOAT;
        else if (optionType == "rate")
            return WorldConfigType::GAME_CONFIG_TYPE_RATES;
        else
            return WorldConfigType::GAME_CONFIG_TYPE_UNKNOWN;
    };

    do
    {
        Field* fields = result->Fetch();

        std::string const& optionName = fields[0].GetString();
        std::string const& optionType = fields[1].GetString();
        uint32 const& IDinTypeGroup = fields[2].GetUInt32();
        std::string const& defaultValue = fields[3].GetString();
        std::string const& customValue = fields[4].GetString();

        auto _type = GetTypeString(optionType);

        if (_type == WorldConfigType::GAME_CONFIG_TYPE_UNKNOWN)
        {
            FMT_LOG_ERROR("config", "> Don't support type ({}) for option ({})", optionType, optionName);
            continue;
        }

        AddCustomOption(_type, IDinTypeGroup, defaultValue, customValue);

        count++;

    } while (result->NextRow());

    FMT_LOG_INFO("server.loading", ">> Loaded {} World Custom Configs in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WorldConfig::RecheckAndFixDependancy()
{
    uint32 oldMSTime = getMSTime();
    // for float
    for (uint32 i = 0; i < WorldFloatConfigs::FLOAT_CONFIG_VALUE_COUNT; i++)
    {
        float value = sWorld->getFloatConfig(WorldFloatConfigs(i));
        // TODO: this 2 case should be moved in Rate
        switch (WorldFloatConfigs(i))
        {
            case WorldFloatConfigs::CONFIG_RESPAWN_DYNAMICRATE_CREATURE:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Respawn.DynamicRateCreature ({}) must be positive. Set to 10.", value);
                    sWorld->setFloatConfig(WorldFloatConfigs(i), 10.0f);
                }
                break;
            }
            case WorldFloatConfigs::CONFIG_RESPAWN_DYNAMICRATE_GAMEOBJECT:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Respawn.DynamicRateGameObject ({}) must be positive. Set to 10.", value);
                    sWorld->setFloatConfig(WorldFloatConfigs(i), 10.0f);
                }
                break;
            }
            default:
                break;
        }
    }

    // for rate
    for (uint32 i = 0; i < Rates::MAX_RATES; i++)
    {
        float value = sWorld->getRate(Rates(i));
        switch (Rates(i))
        {
            case Rates::RATE_HEALTH:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.Health ({}) must be > 0. Using 1 instead.", value);
                    sWorld->setRate(Rates(i), 1.0f);
                }
                break;
            }
            case Rates::RATE_POWER_MANA:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.Mana ({}) must be > 0. Using 1 instead.", value);
                    sWorld->setRate(Rates(i), 1.0f);
                }
                break;
            }
            case Rates::RATE_POWER_RAGE_LOSS:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.Rage.Loss ({}) must be > 0. Using 1 instead.", value);
                    sWorld->setRate(Rates(i), 1.0f);
                }
                break;
            }
            case Rates::RATE_POWER_RUNICPOWER_LOSS:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.RunicPower.Loss ({}) must be > 0. Using 1 instead.", value);
                    sWorld->setRate(Rates(i), 1.0f);
                }
                break;
            }
            case Rates::RATE_REPAIRCOST:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.RepairCost ({}) must be >=0. Using 0.0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            case Rates::RATE_TALENT:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.Talent ({}) must be > 0. Using 1 instead.", value);
                    sWorld->setRate(Rates(i), 1.0f);
                }
                break;
            }
            case Rates::RATE_DURABILITY_LOSS_ON_DEATH:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "DurabilityLoss.OnDeath ({}) must be >=0. Using 0.0 instead.", value);
                    value = 0.0f;
                }
                else if (value > 100.0f)
                {
                    FMT_LOG_ERROR("config", "DurabilityLoss.OnDeath ({}) must be <= 100. Using 100.0 instead.", value);
                    value = 100.0f;
                }
                value /= 100.0f;
                break;
            }
            case Rates::RATE_DURABILITY_LOSS_DAMAGE:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "DurabilityLossChance.Damage ({}) must be >=0. Using 0.0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            case Rates::RATE_DURABILITY_LOSS_PARRY:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "DurabilityLossChance.Parry ({}) must be >=0. Using 0.0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            case Rates::RATE_DURABILITY_LOSS_ABSORB:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "DurabilityLossChance.Absorb ({}) must be >=0. Using 0.0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            case Rates::RATE_DURABILITY_LOSS_BLOCK:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "DurabilityLossChance.Block ({}) must be >=0. Using 0.0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            case Rates::RATE_MOVESPEED:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.MoveSpeed ({}) must be > 0. Using 1 instead.", value);
                    sWorld->setRate(Rates(i), 1.0f);
                }
                break;
            }
            case Rates::RATE_MONEY_QUEST:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.Quest.Money.Reward ({}) must be >=0. Using 0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            case Rates::RATE_MONEY_MAX_LEVEL_QUEST:
            {
                if (value < 0.0f)
                {
                    FMT_LOG_ERROR("config", "Rate.Quest.Money.Max.Level.Reward ({}) must be >=0. Using 0 instead.", value);
                    sWorld->setRate(Rates(i), 0.0f);
                }
                break;
            }
            default:
                break;
        }
    }

    // for int
    for (uint32 i = 0; i < WorldIntConfigs::INT_CONFIG_VALUE_COUNT; i++)
    {
        uint32 value = sWorld->getIntConfig(WorldIntConfigs(i));
        switch (WorldIntConfigs(i))
        {
            case WorldIntConfigs::CONFIG_COMPRESSION:
            {
                if (value < 1 || value > 9)
                {
                    FMT_LOG_ERROR("config", "Compression level ({}) must be in range 1..9. Using default compression level (1).", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 1);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_INTERVAL_GRIDCLEAN:
            {
                if (value < MIN_GRID_DELAY)
                {
                    FMT_LOG_ERROR("config", "GridCleanUpDelay ({}) must be greater {}. Use this minimal value.", value, MIN_GRID_DELAY);
                    sWorld->setIntConfig(WorldIntConfigs(i), MIN_GRID_DELAY);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_INTERVAL_MAPUPDATE:
            {
                if (value < MIN_MAP_UPDATE_DELAY)
                {
                    FMT_LOG_ERROR("config", "MapUpdateInterval ({}) must be greater {}. Use this minimal value.", value, MIN_MAP_UPDATE_DELAY);
                    sWorld->setIntConfig(WorldIntConfigs(i), MIN_MAP_UPDATE_DELAY);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MIN_PLAYER_NAME:
            {
                if (value < 1 || value > MAX_PLAYER_NAME)
                {
                    FMT_LOG_ERROR("config", "MinPlayerName ({}) must be in range 1..{}. Set to 2.", value, MAX_PLAYER_NAME);
                    sWorld->setIntConfig(WorldIntConfigs(i), 2);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MIN_CHARTER_NAME:
            {
                if (value < 1 || value > MAX_CHARTER_NAME)
                {
                    FMT_LOG_ERROR("config", "MinCharterName ({}) must be in range 1..{}. Set to 2.", value, MAX_CHARTER_NAME);
                    sWorld->setIntConfig(WorldIntConfigs(i), 2);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MIN_PET_NAME:
            {
                if (value < 1 || value > MAX_PET_NAME)
                {
                    FMT_LOG_ERROR("config", "MinPetName ({}) must be in range 1..{}. Set to 2.", value, MAX_PET_NAME);
                    sWorld->setIntConfig(WorldIntConfigs(i), 2);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_CHARACTERS_PER_REALM:
            {
                if (value < 1 || value > 10)
                {
                    FMT_LOG_ERROR("config", "CharactersPerRealm ({}) must be in range 1..10. Set to 10.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 10);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_CHARACTERS_PER_ACCOUNT:
            {
                uint32 accPerAcc = sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM);
                if (value < accPerAcc)
                {
                    FMT_LOG_ERROR("config", "CharactersPerAccount ({}) can't be less than CharactersPerRealm ({}).", value, accPerAcc);
                    sWorld->setIntConfig(WorldIntConfigs(i), accPerAcc);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_DEATH_KNIGHTS_PER_REALM:
            {
                if (value > 10)
                {
                    FMT_LOG_ERROR("config", "DeathKnightsPerRealm ({}) must be in range 0..10. Set to 1.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 1);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_SKIP_CINEMATICS:
            {
                if (value > 2)
                {
                    FMT_LOG_ERROR("config", "SkipCinematics ({}) must be in range 0..2. Set to 0.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 0);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MAX_PLAYER_LEVEL:
            {
                if (value < 1 || value > STRONG_MAX_LEVEL)
                {
                    FMT_LOG_ERROR("config", "MaxPlayerLevel ({}) must be in range 1..255. Set to 1.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 1);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_START_PLAYER_LEVEL:
            {
                uint32 maxStartLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
                if (value < 1 || value > maxStartLevel)
                {
                    FMT_LOG_ERROR("config", "StartPlayerLevel ({}) must be in range 0..({}). Set to max.", value, maxStartLevel);
                    sWorld->setIntConfig(WorldIntConfigs(i), maxStartLevel);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_START_DEATH_KNIGHT_PLAYER_LEVEL:
            {
                uint32 minDKStartLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_DEATH_KNIGHT);
                uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
                if (value < minDKStartLevel)
                {
                    FMT_LOG_ERROR("config", "StartDeathKnightPlayerLevel ({}) must be in range ({})..({}). Set to min.", value, minDKStartLevel, maxLevel);
                    sWorld->setIntConfig(WorldIntConfigs(i), minDKStartLevel);
                }
                else if (value > maxLevel)
                {
                    FMT_LOG_ERROR("config", "StartDeathKnightPlayerLevel ({}) must be in range ({})..({}). Set to max.", value, minDKStartLevel, maxLevel);
                    sWorld->setIntConfig(WorldIntConfigs(i), maxLevel);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_START_PLAYER_MONEY:
            {
                if (value > MAX_MONEY_AMOUNT)
                {
                    FMT_LOG_ERROR("config", "StartPlayerMoney ({}) must be in range 0..{}. Set to max.", value, MAX_MONEY_AMOUNT);
                    sWorld->setIntConfig(WorldIntConfigs(i), MAX_MONEY_AMOUNT);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_START_HONOR_POINTS:
            {
                uint32 maxHonor = sWorld->getIntConfig(CONFIG_MAX_HONOR_POINTS);
                if (value > maxHonor)
                {
                    FMT_LOG_ERROR("config", "StartHonorPoints ({}) must be in range 0..MaxHonorPoints({}). Set to {}.", value, maxHonor, maxHonor);
                    sWorld->setIntConfig(WorldIntConfigs(i), maxHonor);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_START_ARENA_POINTS:
            {
                uint32 maxAP = sWorld->getIntConfig(CONFIG_MAX_ARENA_POINTS);
                if (value > maxAP)
                {
                    FMT_LOG_ERROR("config", "StartArenaPoints ({}) must be in range 0..MaxArenaPoints({}). Set to {}.", value, maxAP, maxAP);
                    sWorld->setIntConfig(WorldIntConfigs(i), maxAP);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL:
            {
                uint32 maxLVL = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
                if (value > maxLVL)
                {
                    FMT_LOG_ERROR("config", "RecruitAFriend.MaxLevel ({}) must be in range 0..MaxPlayerLevel({}). Set to {}.", value, maxLVL, maxLVL);
                    sWorld->setIntConfig(WorldIntConfigs(i), maxLVL);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_DAILY_QUEST_RESET_TIME_HOUR:
            {
                if (value > 23)
                {
                    FMT_LOG_ERROR("config", "Quests.DailyResetTime ({}) must be in range 0..23. Set to 3.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 3);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_WEEKLY_QUEST_RESET_TIME_WDAY:
            {
                if (value > 6)
                {
                    FMT_LOG_ERROR("config", "Quests.WeeklyResetDay ({}) must be in range 0..6. Set to 3 (Wednesday).", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 6);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MIN_PETITION_SIGNS:
            {
                if (value > 9)
                {
                    FMT_LOG_ERROR("config", "MinPetitionSigns ({}) must be in range 0..9. Set to 9.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 9);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MAX_OVERSPEED_PINGS:
            {
                if (value != 0 && value < 2)
                {
                    FMT_LOG_ERROR("config", "MaxOverspeedPings ({}) must be in range 2..infinity (or 0 to disable check). Set to 2.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 2);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_BATTLEGROUND_REPORT_AFK:
            {
                if (value < 1)
                {
                    FMT_LOG_ERROR("config", "Battleground.ReportAFK ({}) must be > 0, set to min 1.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 1);
                }
                else if (value > 9)
                {
                    FMT_LOG_ERROR("config", "Battleground.ReportAFK ({}) must be < 10. Set max (10).", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 10);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_GUILD_EVENT_LOG_COUNT:
            {
                if (value > GUILD_EVENTLOG_MAX_RECORDS)
                {
                    FMT_LOG_ERROR("config", "Guild.EventLogRecordsCount ({}) must be < 100, set to default 100.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 100);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_GUILD_BANK_EVENT_LOG_COUNT:
            {
                if (value > GUILD_BANKLOG_MAX_RECORDS)
                {
                    FMT_LOG_ERROR("config", "Guild.BankEventLogRecordsCount ({}) must be < 25, set to default 25.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 25);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_MIN_LEVEL_STAT_SAVE:
            {
                uint32 maxStartLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
                if (value > maxStartLevel)
                {
                    FMT_LOG_ERROR("config", "PlayerSave.Stats.MinLevel ({}) must be in range 0..({}). Using default, do not save character stats (0).", value, maxStartLevel);
                    sWorld->setIntConfig(WorldIntConfigs(i), 0);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_RANDOM_BG_RESET_HOUR:
            {
                if (value > 23)
                {
                    FMT_LOG_ERROR("config", "Battleground.Random.ResetHour ({}) can't be load. Set to 6.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 6);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_CALENDAR_DELETE_OLD_EVENTS_HOUR:
            {
                if (value > 23)
                {
                    FMT_LOG_ERROR("config", "Calendar.DeleteOldEventsHour ({}) can't be load. Set to 6.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 6);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_GUILD_RESET_HOUR:
            {
                if (value > 23)
                {
                    FMT_LOG_ERROR("config", "Guild.ResetHour ({}) can't be load. Set to 6.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 6);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_AUCTION_SEARCH_DELAY:
            {
                uint32 delay = sWorld->getIntConfig(CONFIG_AUCTION_GETALL_DELAY);
                if (value < 100 || value > delay)
                {
                    FMT_LOG_ERROR("config", "Auction.SearchDelay ({}) must be between 100 and 10000. Using default of 300ms", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 300);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_RESPAWN_DYNAMICMODE:
            {
                if (value > 1)
                {
                    FMT_LOG_ERROR("config", "Invalid value for Respawn.DynamicMode ({}). Set to 0.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 0);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_RESPAWN_GUIDWARNLEVEL:
            {
                if (value > 16777215)
                {
                    FMT_LOG_ERROR("config", "Respawn.GuidWarnLevel ({}) cannot be greater than maximum GUID (16777215). Set to 12000000.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 12000000);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_RESPAWN_GUIDALERTLEVEL:
            {
                if (value > 16777215)
                {
                    FMT_LOG_ERROR("config", "Respawn.GuidWarnLevel ({}) cannot be greater than maximum GUID (16777215). Set to 16000000.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 16000000);
                }
                break;
            }
            case WorldIntConfigs::CONFIG_RESPAWN_RESTARTQUIETTIME:
            {
                if (value > 23)
                {
                    FMT_LOG_ERROR("config", "Respawn.RestartQuietTime ({}) must be an hour, between 0 and 23. Set to 3.", value);
                    sWorld->setIntConfig(WorldIntConfigs(i), 3);
                }
                break;
            }
            default:
                break;
        }
    }

    // for bool
    for (uint32 i = 0; i < WorldBoolConfigs::BOOL_CONFIG_VALUE_COUNT; i++)
    {
        bool value = sWorld->getBoolConfig(WorldBoolConfigs(i));
        switch (WorldBoolConfigs(i))
        {
            case WorldBoolConfigs::CONFIG_BASEMAP_LOAD_GRIDS:
            {
                bool unloadgrid = sWorld->getBoolConfig(CONFIG_GRID_UNLOAD);
                if (value && unloadgrid)
                {
                    FMT_LOG_ERROR("config", "BaseMapLoadAllGrids enabled, but GridUnload also enabled. GridUnload must be disabled to enable base map pre-loading. Base map pre-loading disabled");
                    sWorld->setBoolConfig(WorldBoolConfigs(i), 0);
                }
                break;
            }
            case WorldBoolConfigs::CONFIG_INSTANCEMAP_LOAD_GRIDS:
            {
                bool unloadgrid = sWorld->getBoolConfig(CONFIG_GRID_UNLOAD);
                if (value && unloadgrid)
                {
                    FMT_LOG_ERROR("config", "InstanceMapLoadAllGrids enabled, but GridUnload also enabled. GridUnload must be disabled to enable instance map pre-loading. Instance map pre-loading disabled");
                    sWorld->setBoolConfig(WorldBoolConfigs(i), 0);
                }
                break;
            }
            case WorldBoolConfigs::CONFIG_CHECK_GOBJECT_LOS:
            {
                bool vmap = sWorld->getBoolConfig(CONFIG_DETECT_POS_COLLISION);
                if (value && !vmap)
                {
                    FMT_LOG_ERROR("config", "CheckGameObjectLoS enabled, but DetectPosCollision disabled. CheckGameObjectLoS disabled");
                    sWorld->setBoolConfig(WorldBoolConfigs(i), 0);
                }
                break;
            }
            default:
                break;
        }
    }

    FMT_LOG_INFO("server.loading", ">> RecheckAndFixDependancy handled in {} ms", GetMSTimeDiffToNow(oldMSTime));
}

void WorldConfig::AddBoolOption(uint32 IDinTypeGroup, bool const& value)
{
    if (!sWorld->setBoolConfig(WorldBoolConfigs(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Bool option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::AddIntOption(uint32 IDinTypeGroup, uint32 const& value)
{
    if (!sWorld->setIntConfig(WorldIntConfigs(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Int option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::AddFloatOption(uint32 IDinTypeGroup, float const& value)
{
    if (!sWorld->setFloatConfig(WorldFloatConfigs(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Float option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::AddRateOption(uint32 IDinTypeGroup, float const& value)
{
    if (!sWorld->setRate(Rates(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Rate option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::CustomAddBoolOption(uint32 IDinTypeGroup, bool const& value)
{
    if (!sWorld->customSetBoolConfig(CustomWorldBoolConfigs(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Custom Bool option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::CustomAddIntOption(uint32 IDinTypeGroup, uint32 const& value)
{
    if (!sWorld->customSetIntConfig(CustomWorldIntConfigs(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Custom Int option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::CustomAddFloatOption(uint32 IDinTypeGroup, float const& value)
{
    if (!sWorld->customSetFloatConfig(CustomWorldFloatConfigs(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Custom Float option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}

void WorldConfig::CustomAddRateOption(uint32 IDinTypeGroup, float const& value)
{
    if (!sWorld->customSetRate(CustomRates(IDinTypeGroup), value))
    {
        FMT_LOG_ERROR("config", "> Custom Rate option ({}) did not registered in core!", IDinTypeGroup);
        return;
    }
}
