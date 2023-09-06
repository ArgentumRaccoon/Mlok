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
    MlokFatal("test fatal {}!", 3.14);
    MlokError("test error {}!", 3.14);
    MlokWarning("test warning {}!", 3.14);
    MlokInfo("test info {}!", 3.14);
    MlokDebug("test debug {}!", 3.14);
    MlokVerbose("test verbose {}!", 3.14);

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