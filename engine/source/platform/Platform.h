#pragma once

#include "Defines.h"

#include <string>

#ifdef MPLATFORM_LINUX
    #include <xcb/xcb.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
    #include <X11/Xlib.h>
    #include <X11/Xlib-xcb.h>
    #include <sys/time.h>
#endif

class PlatformBase
{
    public:
        virtual bool Startup(const std::string& ApplicationName,
                             int32_t X, int32_t Y, 
                             int32_t Width, int32_t Height) = 0;

        virtual void Shutdown() = 0;
};

#ifdef MPLATFORM_WINDOWS
    #include <windows.h>
    #include <windowsx.h>

class PlatformWin32 : public PlatformBase
{
    public:
        virtual bool Startup(const std::string& ApplicationName,
                             int32_t X, int32_t Y, 
                             int32_t Width, int32_t Height) override;

        virtual void Shutdown() override;

    private:
        HINSTANCE hInstance;
        HWND hWnd;

        static LRESULT CALLBACK Win32ProcessMessage(HWND hWnd, uint32_t Message, WPARAM wParam, LPARAM lParam);
};

#endif // MPLATFORM_WINDOWS

#ifdef MPLATFORM_LINUX
    #include <xcb/xcb.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
    #include <X11/Xlib.h>
    #include <X11/Xlib-xcb.h>
    #include <sys/time.h>

class PlatformLinux: public PlatformBase
{
    public:
        virtual bool Startup(const std::string& ApplicationName,
                             int32_t X, int32_t Y, 
                             int32_t Width, int32_t Height) override;

        virtual void Shutdown() override;

    private:
        Display* pDisplay;
        xcb_connection_t* pConnection;
        xcb_window_t Window;
        xcb_screen_t* pScreen;
        xcb_atom_t wmProtocols;
        xcb_atom_t wmDeleteWin;
}

#endif // MPLATFORM_LINUX