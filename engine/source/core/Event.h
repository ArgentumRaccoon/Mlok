#pragma once

#include "Defines.h"

#include <variant>

#define MAX_MESSAGE_CODES 16384 // seems way more than enough

typedef struct EventContext
{
    std::variant<int64_t, uint64_t, double,
                 int32_t, uint32_t, float,
                 int16_t, uint16_t,
                 int8_t, uint8_t,
                 char>                      Data;
} EventContext;

typedef bool (*PFN_OnEvent)(uint16_t Code, void* Sender, void* ListenerInst, EventContext Data);

typedef struct RegisteredEvent
{
    void* Listener;
    PFN_OnEvent Callback;
} RegisteredEvent;

typedef struct EventCodeEntry
{
    std::vector<RegisteredEvent> Events; // TODO: replace with custom light-weight container
} EventCodeEntry;

enum class SystemEventCode
{
    // Shuts the application down on the next frame.
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // Keyboard key pressed.
    /* Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_PRESSED = 0x02,

    // Keyboard key released.
    /* Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_RELEASED = 0x03,

    // Mouse button pressed.
    /* Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_MOUSE_BUTTON_PRESSED = 0x04,

    // Mouse button released.
    /* Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_MOUSE_BUTTON_RELEASED = 0x05,

    // Mouse moved.
    /* Context usage:
     * u16 x = data.data.u16[0];
     * u16 y = data.data.u16[1];
     */
    EVENT_CODE_MOUSE_MOVED = 0x06,

    // Mouse moved.
    /* Context usage:
     * u8 z_delta = data.data.u8[0];
     */
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    // Resized/resolution changed from the OS.
    /* Context usage:
     * u16 width = data.data.u16[0];
     * u16 height = data.data.u16[1];
     */
    EVENT_CODE_RESIZED = 0x08,

    MAX_EVENT_CODE = 0xFF
};

class MAPI EventSystem
{
    public:

        static EventSystem* Get()
        {
            static EventSystem EventSystemInst;
            return &EventSystemInst;
        }

        void Initialize();
        void Shutdown();

        bool RegisterEvent(uint16_t Code, void* Listener, PFN_OnEvent OnEvent);
        bool UnregisterEvent(uint16_t Code, void* Listener, PFN_OnEvent OnEvent);
        bool FireEvent(uint16_t Code, void* Sender, EventContext Context);

    private:
        EventCodeEntry RegisteredEvents[MAX_MESSAGE_CODES];
};