#include "Event.h"
#include "Logger.h"

EventSystem* EventSystem::Instance = nullptr;

EventSystem* EventSystem::Get()
{
    return EventSystem::Instance;
}

void EventSystem::Initialize(size_t* outMemReq, void* Ptr)
{
    *outMemReq = sizeof(EventSystem);
    if (Ptr == nullptr)
    {
        return;
    }

    Instance = static_cast<EventSystem*>(Ptr);
}

void EventSystem::Shutdown()
{
    for (uint16_t i = 0; i < MAX_MESSAGE_CODES; ++i)
    {
        Instance->RegisteredEvents[i].Events.clear();
    }

    Instance = nullptr;
}

bool EventSystem::RegisterEvent(uint16_t Code, void* Listener, PFN_OnEvent OnEvent)
{
    size_t RegisteredCount = RegisteredEvents[Code].Events.size();
    for (auto& Event : RegisteredEvents[Code].Events)
    {
        if (Event.Listener == Listener)
        {
            MlokWarning("Trying to register event twice for the same listener");
            return false;
        }
    }

    RegisteredEvent Event;
    Event.Listener = Listener;
    Event.Callback = OnEvent;
    RegisteredEvents[Code].Events.push_back(Event);

    return true;
}

bool EventSystem::UnregisterEvent(uint16_t Code, void* Listener, PFN_OnEvent OnEvent)
{
    if (RegisteredEvents[Code].Events.size() == 0)
    {
        MlokWarning("Trying to unregister event that is not registered");
        return false;
    }
    
    size_t RegisteredCount = RegisteredEvents[Code].Events.size();
    for (auto it = RegisteredEvents[Code].Events.begin(); it != RegisteredEvents[Code].Events.end(); ++it)
    {
        if (it->Listener == Listener && it->Callback == OnEvent)
        {
            RegisteredEvents[Code].Events.erase(it);
            return true;
        }
    }

    return false;
}

bool EventSystem::FireEvent(uint16_t Code, void* Sender, EventContext Context)
{
    if (RegisteredEvents[Code].Events.size() == 0)
    {
        return false;
    }

    for (auto& Event : RegisteredEvents[Code].Events)
    {
        if (Event.Callback(Code, Sender, Event.Listener, Context))
        {
            return true;
        }
    }
    
    return false;
}