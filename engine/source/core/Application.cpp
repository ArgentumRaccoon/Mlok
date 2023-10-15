#include "Application.h"

#include "platform/Platform.h"
#include "core/Event.h"
#include "core/Logger.h"
#include "core/Input.h"

#include "renderer/RendererFrontend.h"

bool ApplicationOnEvent(uint16_t Code, void* Sender, void* ListenerInst, EventContext Context);
bool ApplicationOnKey(uint16_t Code, void* Sender, void* ListenerInst, EventContext Context);

bool Application::Create(const ApplicationConfig& Config)
{
    State.bIsRunning = false;
    State.bIsSuspended = false;

    AppClock = std::make_unique<MlokClock>();

    EventSystem::Get()->Initialize();

    EventSystem::Get()->RegisterEvent(EVENT_CODE_APPLICATION_QUIT, this, ApplicationOnEvent);
    EventSystem::Get()->RegisterEvent(EVENT_CODE_KEY_PRESSED, this, ApplicationOnKey);
    EventSystem::Get()->RegisterEvent(EVENT_CODE_KEY_RELEASED, this, ApplicationOnKey);

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

    if (!Renderer::Get()->Initialize(Config.Name))
    {
        Logger::Get()->MFatal("Failed to initialize Renderer. Shutting down...");
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

    AppClock->Start();
    AppClock->Update();
    State.LastTime = AppClock->GetElapsed();

    double TargetFrameSeconds = 1.f / 60; // TODO: move to config

    while (State.bIsRunning)
    {
        if (!State.bIsSuspended)
        {
            if (!Platform::Get()->PumpMessages())
            {
                State.bIsRunning = false;
            }
            
            AppClock->Update();
            double CurrentTime = AppClock->GetElapsed();
            double DeltaTime = CurrentTime - State.LastTime;
            double FrameStartTime = Platform::Get()->GetAbsoluteTime();

            RenderPacket Packet;
            Packet.DeltaTime = DeltaTime;
            Renderer::Get()->DrawFrame(&Packet);

            double FrameEndTime = Platform::Get()->GetAbsoluteTime();
            double FrameElapsedTime = FrameEndTime - FrameStartTime;
            double RemainingSeconds = TargetFrameSeconds - FrameElapsedTime;

            if (RemainingSeconds > 0.f)
            {
                uint64_t RemainingMs = RemainingSeconds * 1000;

                bool bLimitFrames = false; // TODO: move to config
                if (bLimitFrames && RemainingMs > 0.f)
                {
                    Platform::Get()->PlatformSleep(RemainingMs - 1);
                }
            }

            InputSystem::Get()->Update(DeltaTime);

            State.LastTime = CurrentTime;
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

bool ApplicationOnKey(uint16_t Code, void* Sender, void* ListenerInst, EventContext Context)
{
    if (Code == EVENT_CODE_KEY_PRESSED)
    {
        uint16_t KeyCode = Context.Data.u16[0];
        if (KeyCode == KEY_ESCAPE)
        {
            EventContext Data {};
            EventSystem::Get()->FireEvent(EVENT_CODE_APPLICATION_QUIT, nullptr, Data);

            return true;
        }
    }
    
    return false;
}