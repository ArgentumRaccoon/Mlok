#include "Logger.h"

Logger* Logger::Instance = nullptr;

Logger* Logger::Get()
{
    return Instance;
}

bool Logger::Initialize(size_t* outMemReq, void* Ptr)
{
    *outMemReq = sizeof(Logger);
    if (Ptr == nullptr)
    {
        return true;
    }

    Instance = static_cast<Logger*>(Ptr);
    // TODO: open File

    return true;
}

void Logger::Shutdown()
{
    // TODO: close file
}

std::ostream& operator<<(std::ostream& os, const LogLevel& Level)
{
    const char* LevelStrings[6] = { "FATAL: ", "ERROR: ", "WARN: ", "INFO: ", "DEBUG: ", "VERBOSE: " };
    const size_t LevelIdx = static_cast<size_t>(Level);

    os << Logger::Colors[LevelIdx] << LevelStrings[LevelIdx] << "\033[m";

    return os;
}