#pragma once

#include "Defines.h"

#define MAX_MESSAGE_CODES 16384 // seems way more than enough

typedef struct EventContext
{
    // Using union instead of std::variant since the last one doesn't support simple arrays
    union
    {
        int64_t     i64[2];
        uint64_t    u64[2];
        double      f64[2];

        int32_t     i32[4];
        uint32_t    u32[4];
        float       f32[4];

        int16_t     i16[8];
        uint16_t    u16[8];

        int8_t      i8[16];
        uint8_t     u8[16];

        char        c[16];
    } Data;
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

typedef enum SystemEventCode
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
} SystemEventCode;

class MAPI EventSystem
{
    public:
        static EventSystem* Get();

        static void Initialize(size_t* outMemReq, void* Ptr);
        static void Shutdown();

        bool RegisterEvent(uint16_t Code, void* Listener, PFN_OnEvent OnEvent);
        bool UnregisterEvent(uint16_t Code, void* Listener, PFN_OnEvent OnEvent);
        bool FireEvent(uint16_t Code, void* Sender, EventContext Context);
        
    private:
        EventCodeEntry RegisteredEvents[MAX_MESSAGE_CODES];

        static EventSystem* Instance;
};