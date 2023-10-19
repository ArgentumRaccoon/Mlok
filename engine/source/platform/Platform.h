#pragma once

#include "Defines.h"

#ifdef MPLATFORM_LINUX
    #include <xcb/xcb.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
    #include <X11/Xlib.h>
    #include <X11/Xlib-xcb.h>
    #include <sys/time.h>
#endif

class Platform
{
    public:
        static Platform* Get();

        virtual bool Startup(const std::string& ApplicationName,
                             int32_t X, int32_t Y, 
                             int32_t Width, int32_t Height) = 0;

        virtual void Shutdown() = 0;

        virtual bool PumpMessages() = 0;

        // Memory
        virtual void* PlatformAllocate(size_t Size, bool bAligned) = 0;
        virtual void  PlatformFree(void* Block, bool bAligned) = 0;
        virtual void* PlatformZeroMemory(void* Block, size_t Size) = 0;
        virtual void* PlatformCopyMemory(void* Dst, const void* Src, size_t Size) = 0;
        virtual void* PlatformSetMemory(void* Dst, int32_t Value, size_t Size) = 0;

        virtual void ConsoleWrite(const std::string& Message, uint8_t Color) = 0;
        virtual void ConsoleWriteError(const std::string& Message, uint8_t Color) = 0;

        virtual double GetAbsoluteTime() = 0;

        virtual void PlatformSleep(uint64_t ms) = 0;

        // Renderer
        // Vulkan 
        // TODO: think on a more flexible and convenient way of declaring platform specific and renderer specific calls
        virtual void GetRequiredExtensionNames(std::vector<const char*>& OutExtensions) const = 0;
};

#ifdef MPLATFORM_WINDOWS
    #include <windows.h>
    #include <windowsx.h>

class PlatformWin32 : public Platform
{
    public:
        virtual bool Startup(const std::string& ApplicationName,
                             int32_t X, int32_t Y, 
                             int32_t Width, int32_t Height) override;

        virtual void Shutdown() override;

        virtual bool PumpMessages() override;

        // Memory
        virtual void* PlatformAllocate(size_t Size, bool bAligned) override;
        virtual void  PlatformFree(void* Block, bool bAligned) override;
        virtual void* PlatformZeroMemory(void* Block, size_t Size) override;
        virtual void* PlatformCopyMemory(void* Dst, const void* Src, size_t Size) override;
        virtual void* PlatformSetMemory(void* Dst, int32_t Value, size_t Size) override;

        virtual void ConsoleWrite(const std::string& Message, uint8_t Color) override;
        virtual void ConsoleWriteError(const std::string& Message, uint8_t Color) override;

        virtual double GetAbsoluteTime() override;

        virtual void PlatformSleep(uint64_t ms) override;

        // Renderer
        // Vulkan
        virtual void GetRequiredExtensionNames(std::vector<const char*>& OutExtensions) const override;

    private:
        HINSTANCE hInstance;
        HWND hWnd;

        double ClockFrequency;
        LARGE_INTEGER StartTime;

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

class PlatformLinux : public Platform
{
    public:
        virtual bool Startup(const std::string& ApplicationName,
                             int32_t X, int32_t Y, 
                             int32_t Width, int32_t Height) override;

        virtual void Shutdown() override;

        virtual bool PumpMessages() override;

        // Memory
        virtual void* PlatformAllocate(size_t Size, bool bAligned) override;
        virtual void  PlatformFree(void* Block, bool bAligned) override;
        virtual void* PlatformZeroMemory(void* Block, size_t Size) override;
        virtual void* PlatformCopyMemory(void* Dst, const void* Src, size_t Size) override;
        virtual void* PlatformSetMemory(void* Dst, int32_t Value, size_t Size) override;

        virtual void ConsoleWrite(const char* Message, uint8_t Color) override;
        virtual void ConsoleWriteError(const char* Message, uint8_t Color) override;

        virtual double GetAbsoluteTime() override;

        virtual void PlatformSleep(uint64_t ms) override;

        // Renderer
        // Vulkan
        virtual void GetRequiredExtensionNames(std::vector<const char*>& OutExtensions) const override;
        
    private:
        Display* pDisplay;
        xcb_connection_t* pConnection;
        xcb_window_t Window;
        xcb_screen_t* pScreen;
        xcb_atom_t wmProtocols;
        xcb_atom_t wmDeleteWin;
}

#endif // MPLATFORM_LINUX