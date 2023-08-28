#pragma once

#include "Defines.h"

#include "MlokUtils.h"

#include <string>
#include <iostream>

#define LOG_WARNING_ENABLED
#define LOG_INFO_ENABLED

#ifndef MRELEASE
    #define LOG_DEBUG_ENABLED
    #define LOG_VERBOSE_ENABLED
#endif // MRELEASE

enum class LogLevel
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_VERBOSE = 5
};

class MAPI Logger
{
    public:
        static Logger* Get()
        {
            static Logger LoggerHandle;
            return &LoggerHandle;
        }

        bool Initialize();
        void Shutdown();

        template<typename... TArgs>
        void LogOutput(const LogLevel& Level, const std::string& Message, TArgs&&... Args);

        template<typename... TArgs>
        constexpr void MFatal(const std::string& Message, TArgs&&... Args);
        template<typename... TArgs>
        constexpr void MError(const std::string& Message, TArgs&&... Args);
        template<typename... TArgs>
        constexpr void MWarning(const std::string& Message, TArgs&&... Args);
        template<typename... TArgs>
        constexpr void MInfo(const std::string& Message, TArgs&&... Args);
        template<typename... TArgs>
        constexpr void MDebug(const std::string& Message, TArgs&&... Args);
        template<typename... TArgs>
        constexpr void MVerbose(const std::string& Message, TArgs&&... Args);

        friend std::ostream& operator<<(std::ostream& os, const LogLevel& Level);

    private:
        // TODO: add file handle

        static constexpr char Colors[6][6] = {
            "\033[41m",
            "\033[31m",
            "\033[33m",
            "\033[34m",
            "\033[32m",
            "\033[37m"
        };
};

std::ostream& operator<<(std::ostream& os, const LogLevel& Level);

#include "Logger.tcc"