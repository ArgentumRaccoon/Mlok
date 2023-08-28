#include "Application.h"

#include "platform/Platform.h"
#include "core/Logger.h"

bool Application::Create(const ApplicationConfig& Config)
{
    State.bIsRunning = false;
    State.bIsSuspended = false;

    if (!Platform::Get()->Startup(Config.Name, Config.StartPosX, Config.StartPosY, Config.StartWidth, Config.StartHeight))
    {
        return false;
    }

    return true;
}

bool Application::Run()
{
    State.bIsRunning = true;

    // TODO: remove after adding unit tests
    Logger::Get()->MFatal("test fatal {}!", 3.14);
    Logger::Get()->MError("test error {}!", 3.14);
    Logger::Get()->MWarning("test warning {}!", 3.14);
    Logger::Get()->MInfo("test info {}!", 3.14);
    Logger::Get()->MDebug("test debug {}!", 3.14);
    Logger::Get()->MVerbose("test verbose {}!", 3.14);

    while (State.bIsRunning)
    {
        if (!State.bIsSuspended)
        {
            if (!Platform::Get()->PumpMessages())
            {
                State.bIsRunning = false;
            }
            
            // TODO: update everything
        }
    }
    
    State.bIsRunning = false;

    Platform::Get()->Shutdown();

    return true;
}