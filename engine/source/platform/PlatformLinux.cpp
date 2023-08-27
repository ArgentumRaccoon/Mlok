#include "Platform.h"

#ifdef MPLATFORM_LINUX

bool PlatformLinux::Startup(const char* ApplicationName,
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
                        strlen(ApplicationName),
                        ApplicationName);

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

#endif // MPLATFORM_LINUX