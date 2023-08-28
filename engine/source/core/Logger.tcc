#pragma once

std::ostream& operator<<(std::ostream& os, const LogLevel& Level);

template<typename... TArgs>
void Logger::LogOutput(const LogLevel& Level, const std::string& Message, TArgs&&... Args)
{
    bool bIsError = static_cast<int32_t>(Level) < static_cast<int32_t>(LogLevel::LOG_LEVEL_WARNING);
    const size_t LevelIdx = static_cast<size_t>(Level);
    
    if (bIsError)
    {
        std::cerr << Level << Logger::Colors[LevelIdx] << MlokUtils::StringFormat(Message, Args...) << "\033[m" << std::endl;
    }
    else
    {
        std::cout << Level << Logger::Colors[LevelIdx] << MlokUtils::StringFormat(Message, Args...) << "\033[m" << std::endl;
    }
}

template<typename... TArgs>
constexpr void Logger::MFatal(const std::string& Message, TArgs&&... Args)
{
    LogOutput(LogLevel::LOG_LEVEL_FATAL, Message, Args...);
}

template<typename... TArgs>
constexpr void Logger::MError(const std::string& Message, TArgs&&... Args)
{
    LogOutput(LogLevel::LOG_LEVEL_ERROR, Message, Args...);
}

template<typename... TArgs>
constexpr void Logger::MWarning(const std::string& Message, TArgs&&... Args)
{
#ifdef LOG_WARNING_ENABLED
    LogOutput(LogLevel::LOG_LEVEL_WARNING, Message, Args...);
#endif
}

template<typename... TArgs>
constexpr void Logger::MInfo(const std::string& Message, TArgs&&... Args)
{
#ifdef LOG_INFO_ENABLED
    LogOutput(LogLevel::LOG_LEVEL_INFO, Message, Args...);
#endif
}

template<typename... TArgs>
constexpr void Logger::MDebug(const std::string& Message, TArgs&&... Args)
{
#ifdef LOG_DEBUG_ENABLED
    LogOutput(LogLevel::LOG_LEVEL_DEBUG, Message, Args...);
#endif
}

template<typename... TArgs>
constexpr void Logger::MVerbose(const std::string& Message, TArgs&&... Args)
{
#ifdef LOG_VERBOSE_ENABLED
    LogOutput(LogLevel::LOG_LEVEL_VERBOSE, Message, Args...);
#endif
}