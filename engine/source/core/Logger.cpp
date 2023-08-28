#include "Logger.h"

bool Logger::Initialize()
{
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