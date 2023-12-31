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

#ifndef TRINITYCORE_LOG_H
#define TRINITYCORE_LOG_H

#include "Define.h"
#include "AsioHacksFwd.h"
#include "LogCommon.h"
#include "StringFormat.h"

#include <memory>
#include <unordered_map>
#include <vector>

class Appender;
class Logger;
struct LogMessage;

namespace Trinity
{
    namespace Asio
    {
        class IoContext;
    }
}

#define LOGGER_ROOT "root"

typedef Appender*(*AppenderCreatorFn)(uint8 id, std::string const& name, LogLevel level, AppenderFlags flags, std::vector<std::string_view> const& extraArgs);

template <class AppenderImpl>
Appender* CreateAppender(uint8 id, std::string const& name, LogLevel level, AppenderFlags flags, std::vector<std::string_view> const& extraArgs)
{
    return new AppenderImpl(id, name, level, flags, extraArgs);
}

class TC_COMMON_API Log
{
    typedef std::unordered_map<std::string, Logger> LoggerMap;

    private:
        Log();
        ~Log();
        Log(Log const&) = delete;
        Log(Log&&) = delete;
        Log& operator=(Log const&) = delete;
        Log& operator=(Log&&) = delete;

    public:
        static Log* instance();

        void Initialize(Trinity::Asio::IoContext* ioContext);
        void SetSynchronous();  // Not threadsafe - should only be called from main() after all threads are joined
        void LoadFromConfig();
        void Close();
        bool ShouldLog(std::string const& type, LogLevel level) const;
        bool SetLogLevel(std::string const& name, int32 level, bool isLogger = true);

        template<typename Format, typename... Args>
        inline void outMessage(std::string const& filter, LogLevel const level, Format&& fmt, Args&&... args)
        {
            outMessage(filter, level, Trinity::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...));
        }

        template<typename Format, typename... Args>
        void outCommand(uint32 account, Format&& fmt, Args&&... args)
        {
            if (!ShouldLog("commands.gm", LOG_LEVEL_INFO))
                return;

            outCommand(Trinity::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...), std::to_string(account));
        }

        void outCharDump(char const* str, uint32 account_id, uint64 guid, char const* name);

        void SetRealmId(uint32 id);

        template<class AppenderImpl>
        void RegisterAppender()
        {
            RegisterAppender(AppenderImpl::type, &CreateAppender<AppenderImpl>);
        }

        std::string const& GetLogsDir() const { return m_logsDir; }
        std::string const& GetLogsTimestamp() const { return m_logsTimestamp; }

    private:
        static std::string GetTimestampStr();
        void write(std::unique_ptr<LogMessage>&& msg) const;

        Logger const* GetLoggerByType(std::string const& type) const;
        Appender* GetAppenderByName(std::string_view name);
        uint8 NextAppenderId();
        void CreateAppenderFromConfig(std::string const& name);
        void CreateLoggerFromConfig(std::string const& name);
        void ReadAppendersFromConfig();
        void ReadLoggersFromConfig();
        void RegisterAppender(uint8 index, AppenderCreatorFn appenderCreateFn);
        void outMessage(std::string const& filter, LogLevel level, std::string&& message);
        void outCommand(std::string&& message, std::string&& param1);

        std::unordered_map<uint8, AppenderCreatorFn> appenderFactory;
        std::unordered_map<uint8, std::unique_ptr<Appender>> appenders;
        std::unordered_map<std::string, std::unique_ptr<Logger>> loggers;
        uint8 AppenderId;
        LogLevel lowestLogLevel;

        std::string m_logsDir;
        std::string m_logsTimestamp;

        Trinity::Asio::IoContext* _ioContext;
        Trinity::Asio::Strand* _strand;
};

#define sLog Log::instance()

#define LOG_EXCEPTION_FREE(filterType__, level__, ...) \
    { \
        try \
        { \
            sLog->outMessage(filterType__, level__, __VA_ARGS__); \
        } \
        catch (std::exception& e) \
        { \
            sLog->outMessage("server", LOG_LEVEL_ERROR, "Wrong format occurred ({}) at {}:{}.", \
                e.what(), __FILE__, __LINE__); \
        } \
    }

// This will catch format errors on build time
#define FMT_LOG_MESSAGE_BODY(filterType__, level__, ...)                 \
        do {                                                            \
            if (sLog->ShouldLog(filterType__, level__))                 \
            {                                                           \
                LOG_EXCEPTION_FREE(filterType__, level__, __VA_ARGS__); \
            }                                                           \
        } while (0)

#define FMT_LOG_TRACE(filterType__, ...) \
    FMT_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_TRACE, __VA_ARGS__)

#define FMT_LOG_DEBUG(filterType__, ...) \
    FMT_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define FMT_LOG_INFO(filterType__, ...)  \
    FMT_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_INFO, __VA_ARGS__)

#define FMT_LOG_WARN(filterType__, ...)  \
    FMT_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_WARN, __VA_ARGS__)

#define FMT_LOG_ERROR(filterType__, ...) \
    FMT_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_ERROR, __VA_ARGS__)

#define FMT_LOG_FATAL(filterType__, ...) \
    FMT_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_FATAL, __VA_ARGS__)

#endif
