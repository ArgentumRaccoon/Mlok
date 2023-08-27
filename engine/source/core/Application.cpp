#include "Application.h"
#include "platform/Platform.h"

// TODO: move all platform dependency to platform state object, make platform-agnostic
#ifdef MPLATFORM_WINDOWS
    PlatformWin32 Platform;
#else
#ifdef MPLATFORM_LINUX
    PlatformLinux Platform;
#endif // MPLATFORM_LINUX
#endif // MPLATFORM_WINDOWS

bool Application::Create(const ApplicationConfig& Config)
{
    bIsRunning = false;
    bIsSuspended = false;

    if (!Platform.Startup(Config.Name, Config.StartPosX, Config.StartPosY, Config.StartWidth, Config.StartHeight))
    {
        return false;
    }

    return true;
}

bool Application::Run()
{
    bIsRunning = true;

    while (bIsRunning)
    {
        if (!bIsSuspended)
        {
            // TODO: update everything
        }
    }
    
    bIsRunning = false;

    Platform.Shutdown();

    return true;
}