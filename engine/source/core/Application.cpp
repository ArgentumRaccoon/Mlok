#include "Application.h"

#include "platform/Platform.h"
#include "core/Event.h"
#include "core/Logger.h"
#include "core/Input.h"

bool ApplicationOnEvent(uint16_t Code, void* Sender, void* ListenerInst, EventContext Context);

bool Application::Create(const ApplicationConfig& Config)
{
    State.bIsRunning = false;
    State.bIsSuspended = false;

    EventSystem::Get()->Initialize();

    EventSystem::Get()->RegisterEvent(EVENT_CODE_APPLICATION_QUIT, this, ApplicationOnEvent);

    if (!Logger::Get()->Initialize())
    {
        MlokError("Failed to initialize Logger! Shutting down...");
        return false;
    }

    InputSystem::Get()->Initialize();

    if (!Platform::Get()->Startup(Config.Name, Config.StartPosX, Config.StartPosY, Config.StartWidth, Config.StartHeight))
    {
        MlokError("Failed to initialize Platform! Shutting down...");
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

    InputSystem::Get()->Shutdown();

    Logger::Get()->Shutdown();

    EventSystem::Get()->UnregisterEvent(EVENT_CODE_APPLICATION_QUIT, this, ApplicationOnEvent);

    EventSystem::Get()->Shutdown();

    return true;
}

void Application::Stop()
{
    State.bIsRunning = false;
}

bool ApplicationOnEvent(uint16_t Code, void* Sender, void* ListenerInst, EventContext Context)
{
    switch (Code)
    {
        case EVENT_CODE_APPLICATION_QUIT:
            MlokInfo("EVENT_CODE_APPLICATION_QUIT received, shutting down the Application...");
            static_cast<Application*>(ListenerInst)->Stop();
            return true;
    }

    return false;
}