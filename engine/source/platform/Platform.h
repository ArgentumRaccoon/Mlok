#pragma once

#include "Defines.h"

#ifdef MPLATFORM_WINDOWS
    #include <windows.h>
    #include <windowsx.h>

    #include "renderer/vulkan/VulkanTypes.inl"
#endif

#ifdef MPLATFORM_LINUX
    #include <xcb/xcb.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
    #include <X11/Xlib.h>
    #include <X11/Xlib-xcb.h>
    #include <sys/time.h>
#endif

class VulkanContext;

class Platform
{
    public:
        static Platform* Get();

        static bool Startup(size_t* outMemReq, void* Ptr,
                            const std::string& ApplicationName,
                            int32_t X, int32_t Y, 
                            int32_t Width, int32_t Height);

        static void Shutdown();

        bool PumpMessages();

        // Memory
        static void* PlatformAllocate(size_t Size, bool bAligned);
        static void  PlatformFree(void* Block, bool bAligned);
        static void* PlatformZeroMemory(void* Block, size_t Size);
        static void* PlatformCopyMemory(void* Dst, const void* Src, size_t Size);
        static void* PlatformSetMemory(void* Dst, int32_t Value, size_t Size);

        void ConsoleWrite(const std::string& Message, uint8_t Color);
        void ConsoleWriteError(const std::string& Message, uint8_t Color);

        double GetAbsoluteTime();

        void PlatformSleep(uint64_t ms);

        // Renderer
        // Vulkan 
        // TODO: think on a more flexible and convenient way of declaring platform specific and renderer specific calls
        void GetRequiredExtensionNames(std::vector<const char*>& OutExtensions) const;
        bool CreateVulkanSurface(VulkanContext* Context);
    
    private:
    #ifdef MPLATFORM_WINDOWS
        HINSTANCE hInstance;
        HWND hWnd;

        double ClockFrequency;
        LARGE_INTEGER StartTime;

        static LRESULT CALLBACK Win32ProcessMessage(HWND hWnd, uint32_t Message, WPARAM wParam, LPARAM lParam);
    #endif // MPLATFORM_WINDOWS
    
    #ifdef MPLATFORM_LINUX
        Display* pDisplay;
        xcb_connection_t* pConnection;
        xcb_window_t Window;
        xcb_screen_t* pScreen;
        xcb_atom_t wmProtocols;
        xcb_atom_t wmDeleteWin;
    #endif // MPLATFORM_LINUX

        vk::SurfaceKHR Surface;

        static Platform* Instance;
};

// #ifdef MPLATFORM_WINDOWS
//     #include <windows.h>
//     #include <windowsx.h>

// class PlatformWin32 : public Platform
// {
//     public:
//         static PlatformWin32* Get();

//         static bool Startup(size_t* outMemReq, void* Ptr,
//                             const std::string& ApplicationName,
//                             int32_t X, int32_t Y, 
//                             int32_t Width, int32_t Height);

//         static void Shutdown();

//         virtual bool PumpMessages() override;

//         // Memory
//         virtual void* PlatformAllocate(size_t Size, bool bAligned) override;
//         virtual void  PlatformFree(void* Block, bool bAligned) override;
//         virtual void* PlatformZeroMemory(void* Block, size_t Size) override;
//         virtual void* PlatformCopyMemory(void* Dst, const void* Src, size_t Size) override;
//         virtual void* PlatformSetMemory(void* Dst, int32_t Value, size_t Size) override;

//         virtual void ConsoleWrite(const std::string& Message, uint8_t Color) override;
//         virtual void ConsoleWriteError(const std::string& Message, uint8_t Color) override;

//         virtual double GetAbsoluteTime() override;

//         virtual void PlatformSleep(uint64_t ms) override;

//         // Renderer
//         // Vulkan
//         virtual void GetRequiredExtensionNames(std::vector<const char*>& OutExtensions) const override;

//     private:
//         HINSTANCE hInstance;
//         HWND hWnd;

//         double ClockFrequency;
//         LARGE_INTEGER StartTime;

//         static LRESULT CALLBACK Win32ProcessMessage(HWND hWnd, uint32_t Message, WPARAM wParam, LPARAM lParam);

//         static PlatformWin32* Instance;
// };

// #endif // MPLATFORM_WINDOWS

// #ifdef MPLATFORM_LINUX
//     #include <xcb/xcb.h>
//     #include <X11/keysym.h>
//     #include <X11/XKBlib.h>
//     #include <X11/Xlib.h>
//     #include <X11/Xlib-xcb.h>
//     #include <sys/time.h>

// class PlatformLinux : public Platform
// {
//     public:
//         static PlatformLinux* Get();

//         static bool Startup(size_t* outMemReq, void* Ptr,
//                             const std::string& ApplicationName,
//                             int32_t X, int32_t Y, 
//                             int32_t Width, int32_t Height);

//         static void Shutdown();

//         virtual bool PumpMessages() override;

//         // Memory
//         virtual void* PlatformAllocate(size_t Size, bool bAligned) override;
//         virtual void  PlatformFree(void* Block, bool bAligned) override;
//         virtual void* PlatformZeroMemory(void* Block, size_t Size) override;
//         virtual void* PlatformCopyMemory(void* Dst, const void* Src, size_t Size) override;
//         virtual void* PlatformSetMemory(void* Dst, int32_t Value, size_t Size) override;

//         virtual void ConsoleWrite(const char* Message, uint8_t Color) override;
//         virtual void ConsoleWriteError(const char* Message, uint8_t Color) override;

//         virtual double GetAbsoluteTime() override;

//         virtual void PlatformSleep(uint64_t ms) override;

//         // Renderer
//         // Vulkan
//         virtual void GetRequiredExtensionNames(std::vector<const char*>& OutExtensions) const override;
        
//     private:
//         Display* pDisplay;
//         xcb_connection_t* pConnection;
//         xcb_window_t Window;
//         xcb_screen_t* pScreen;
//         xcb_atom_t wmProtocols;
//         xcb_atom_t wmDeleteWin;

//         static PlatformLinux* Instance;
// };

// #endif // MPLATFORM_LINUX