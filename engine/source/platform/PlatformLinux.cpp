#include "Platform.h"

#ifdef MPLATFORM_LINUX

#include <cstdlib>

Platform* Platform::Get()
{
    static PlatformLinux PlatformHandle;
    return &PlatformHandle;
}

bool PlatformLinux::Startup(const std::string& ApplicationName,
                            int32_t X, int32_t Y, 
                            int32_t Width, int32_t Height)
{
    pDisplay = XOpenDisplay(nullptr);

    XAutoRepeatOff(pDisplay);

    pConection = XGetXCBConnection(pDisplay);

    if (xcb_connection_has_error(pConnection))
    {
        return false;
    }

    const struct xcb_setup_t* Setup = xcb_get_setup(pConnection);

    xcb_screen_iterator_t it = xcb_setup_roots_iterator(Setup);
    int ScreenP = 0;
    for (int32_t s = ScreenP; s > 0; s--)
    {
        xcb_screen_next(&it);
    }

    pScreen = it.data;

    Window = xcb_generate_id(pConnection);

    uint32_t EventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    uint32_t EventValues = XCB_EVENT_MASK_BUTTON_PRESSED    | XCB_EVENT_MASK_BUTTON_RELEASED   |
                           XCB_EVENT_MASK_KEY_PRESS         | XCB_EVENT_MASK_KEY_RELEASE       |
                           XCB_EVENT_MASK_EXPOSURE          | XCB_EVENT_MASK_POINTER_MOTION    |
                           XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    uint32_t ValueList[] = { pScreen->black_pixel, EventValues };

    xcb_void_cookie_t Cookie = xcb_create_window(pConnection,
                                                 XCB_COPY_FROM_PARENT,
                                                 Window,
                                                 pScreen->root,
                                                 X, Y,
                                                 Width, Height,
                                                 0,
                                                 XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                                 pScreen->root_visual,
                                                 EventMask,
                                                 ValueList);

    xcb_change_property(pConnection,
                        XCB_PROP_MODE_REPLACE,
                        Window,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        ApplicationName.length(),
                        ApplicationName.c_str());

    xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(pConnection,
                                                              0,
                                                              strlen("WM_DELETE_WINDOW"),
                                                              "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(pConnection,
                                                                 0,
                                                                 strlen("WM_PROTOCOLS"),
                                                                 "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wmDeleteReply = xcb_intern_atom_reply(pConnection,
                                                                   wmDeleteCookie,
                                                                   nullptr);
    xcb_intern_atom_reply_t* wmProtocolsReply = xcb_intern_atom_reply(pConnection,
                                                                      wmProtocolsCookie,
                                                                      nullptr);
    wmDeleteWin = wmDeleteReply->atom;
    wmProtocols = wmProtocolsReply->atom;

    xcb_change_property(pConnection,
                        XCB_PROP_MODE_REPLACE,
                        Window,
                        wmProtocolsReply->atom,
                        4,
                        32,
                        1,
                        &wmDeleteReply->atom);

    xcb_map_window(pConnection, Window);

    int32_t StreamResult = xcb_flush(pConnection);
    if (StreamResult <= 0)
    {
        return false;
    }

    return true;
}

void PlatformLinux::Shutdown()
{
    XAuthoRepeatOn(pDisplay);

    xcb_destroy_window(pConnection, Window);
}

bool PlatformLinux::PumpMessages()
{
    xcb_generic_event_t* Event;
    xcb_client_message_event_t* ClientMessage;

    bool bQuitFlagged = false;

    while((Event = xcb_poll_for_event(pConnection)))
    {
        switch (Event->response_type & ~0x80)
        {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE:
                {
                    xcb_key_press_event_t* kbEvent = (xcb_key_press_event_t*)Event;
                    bool bPressed = Event->response_type = XCB_KEY_PRESS;
                    xcb_keycode_t Code = kbEvent->detail;
                    KeySym Sym = XkbKeycodeToKeysym(InternalState->Display, (KeyCode)Code, 0, Code & ShiftMask ? 1 : 0);
                    KeyboardKey Key = TranslateKeycode(Sym);

                    InputProcessKey(Key, bPressed);
                }
                break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE:
                {
                    xcb_button_press_event_t* MouseEvent = (xcb_button_press_event_t*)Event;
                    bool bPressed = Event->response_type = XCB_BUTTON_PRESS;
                    MouseButton Button = MOUSE_BUTTON_MAX;
                    switch (MouseEvent->detail)
                    {
                    case XCB_BUTTON_INDEX_1:
                        Button = MOUSE_BUTTON_LEFT;
                        break;
                    case XCB_BUTTON_INDEX_2:
                        Button = MOUSE_BUTTON_MIDDLE;
                        break;
                    case XCB_BUTTON_INDEX_3:
                        Button = MOUSE_BUTTON_RIGHT;
                        break;
                    default:
                        break;
                    }

                    InputProcessMouseButton(Button, bPressed);
                }
                break;
            case XCB_MOTION_NOTIFY:
                {
                    xcb_motion_notify_event_t* MoveEvent = (xcb_motion_notify_event_t*)Event;

                    InputProcessMouseMove(MoveEvent->event_x, MoveEvent->event_y);
                }
                break;
            case XCB_CONFIGURE_NOTIFY:
                {
                    xcb_notify_event_t* ConfigureEvent = (xcb_configure_notity_event_t*)Event;

                    EventContext Context;
                    Context.Data.u16[0] = ConfigureEvent->width;
                    Context.Data.u16[1] = ConfigureEvent->height;
                    EventFire(EVENT_CODE_RESIZED, 0, Context);
                }
                break;
            case XCB_CLIENT_MESSAGE:
                {
                    ClientMessage = static_cast<xcb_client_message_event_t*>(Event);

                    if (ClientMessage->data.data32[0] == InternalState->wmDeleteWin)
                    {
                        bQuitFlagged = true;
                    }
                }
                break;
            default:
                break;
        }

        free(Event);
    }

    return !bQuitFlagged;
}

void* PlatformLinux::PlatformAllocate(size_t Size, bool bAligned)
{
    return malloc(Size);
}

void  PlatformLinux::PlatformFree(void* Block, bool bAligned)
{
    free(Block);
}

void* PlatformLinux::PlatformZeroMemory(void* Block, size_t Size)
{
    return memset(Block, 0, Size);
}

void* PlatformLinux::PlatformCopyMemory(void* Dst, const void* Src, size_t Size)
{
    return memcpy(Dst, Src, Size);
}

void* PlatformLinux::PlatformSetMemory(void* Dst, int32_t Value, size_t Size)
{
    return memset(Dst, Value, Size);
}

void PlatformLinux::ConsoleWrite(const char* Message, uint8_t Color)
{
    const cher* ColorCodes[] = { "0;41", "1;31", "1;33", "1;34", "1;32", "1;30" }; // FATAL, ERROR, WARNING, INFO, DEBUG, VERBOSE
    
    printf("\033[%sm%s\033[0m", ColorCodes[Color], Message);
}

void PlatformLinux::ConsoleWriteError(const char* Message, uint8_t Color)
{
    const cher* ColorCodes[] = { "0;41", "1;31", "1;33", "1;34", "1;32", "1;30" }; // FATAL, ERROR, WARNING, INFO, DEBUG, VERBOSE
    
    printf("\033[%sm%s\033[0m", ColorCodes[Color], Message);
}

double PlatformLinux::GetAbsoluteTime()
{
    struct timespec Now;
    clock_gettime(CLOCK_MONOTONIC, &Now);
    return Now.tv_sec + now.tv_nsec * 0.000000001f;
}

void PlatformLinux::PlatformSleep(uint64_t ms)
{
    #if _POSIX_C_SOURCE >= 199309L
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000 * 1000;
        nanosleep(&ts, 0);
    #else
        if (ms >= 1000)
        {
            sleep (ms / 1000);
        }
        usleep((ms % 1000) * 1000);
    #endif
}

#endif // MPLATFORM_LINUX