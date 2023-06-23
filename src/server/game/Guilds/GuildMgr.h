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

#ifndef _GUILDMGR_H
#define _GUILDMGR_H

#include "Define.h"
#include "ObjectGuid.h"
#include <unordered_map>
#include <vector>

class Guild;

struct GuildWars
{
    uint32 attackerGuildId;
    uint32 defenderGuildId;
};
typedef std::unordered_map<uint32, GuildWars> GuildWarsContainer;

struct GuildWarsHistory
{
    std::string attackerGuild;
    uint32 attackerGuildId;
    std::string defenderGuild;
    uint32 defenderGuildId;
    time_t timeOfStartWar;
    time_t timeOfEndWar;
    std::string winnerGuild;
};
typedef std::unordered_map<uint32, GuildWarsHistory> GuildWarsHistoryContainer;

class TC_GAME_API GuildMgr
{
private:
    GuildMgr();
    ~GuildMgr();
    GuildMgr(GuildMgr const&) = delete;
    GuildMgr& operator=(GuildMgr const&) = delete;

public:
    static GuildMgr* instance();

    Guild* GetGuildByLeader(ObjectGuid guid) const;
    Guild* GetGuildById(ObjectGuid::LowType guildId) const;
    Guild* GetGuildByName(std::string_view guildName) const;
    std::string GetGuildNameById(ObjectGuid::LowType guildId) const;

    void LoadGuilds();
    void AddGuild(Guild* guild);
    void RemoveGuild(ObjectGuid::LowType guildId);

    // Atiesh features
    // GSystem
    std::string GetGuildNameByIdWithLvl(ObjectGuid::LowType guildId) const;
    std::string GetGuildNameWithGLvl(std::string const& guildName, uint32 level) const;
    std::string GetGuildEnemy(ObjectGuid::LowType guildId) const;

    time_t GetTimeOfLastWarStart(ObjectGuid::LowType guildId);
    time_t GetTimeOfLastWarEnd(ObjectGuid::LowType guildId);

    bool GuildHasWarState(ObjectGuid::LowType guildId);
    bool IsGuildsInWar(ObjectGuid::LowType firstguildId, ObjectGuid::LowType secondguildId);
    bool StartNewWar(GuildWars& data);

    void StopWarBetween(ObjectGuid::LowType firstguildId, ObjectGuid::LowType secondguildId, ObjectGuid::LowType winnerguildId);
    void StopAllGuildWarsFor(ObjectGuid::LowType guildId); // Stop all GW before Disband
    void UpdateWarFlagForAllEnemiesAndThis(ObjectGuid::LowType guildId);

    void LoadGuildWarData();
    void LoadGuildWarHistory();
    // Atiesh features end

    ObjectGuid::LowType GenerateGuildId();
    void SetNextGuildId(ObjectGuid::LowType Id) { NextGuildId = Id; }

    void ResetTimes();
protected:
    typedef std::unordered_map<ObjectGuid::LowType, Guild*> GuildContainer;
    ObjectGuid::LowType NextGuildId;
    GuildContainer GuildStore;

    // Atiesh features
    GuildWarsContainer _guildWarStore;
    GuildWarsHistoryContainer _guildWarHistoryStore;
    // Atiesh features end
};

#define sGuildMgr GuildMgr::instance()

#endif
