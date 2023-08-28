#include "Platform.h"

#ifdef MPLATFORM_WINDOWS

Platform* Platform::Get()
{
    static PlatformWin32 PlatformHandle;
    return &PlatformHandle;
}

bool PlatformWin32::Startup(const std::string& ApplicationName,
                            int32_t X, int32_t Y, 
                            int32_t Width, int32_t Height)
{
    hInstance = GetModuleHandleA(0);
    LPCSTR WindowClass = "MlokWindowClass";

    HICON hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = Win32ProcessMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = WindowClass;

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    uint32_t ClientX = X;
    uint32_t ClientY = Y;
    uint32_t ClientWidth = Width;
    uint32_t ClientHeight = Height;

    uint32_t WindowX = ClientX;
    uint32_t WindowY = ClientY;
    uint32_t WindowWidth = ClientWidth;
    uint32_t WindowHeight = ClientHeight;

    uint32_t WindowStyle = WS_OVERLAPPEDWINDOW;
    uint32_t WindowExStyle = WS_EX_APPWINDOW;

    RECT BorderRect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&BorderRect, WindowStyle, 0, WindowExStyle);

    WindowX += BorderRect.left;
    WindowY += BorderRect.top;

    WindowWidth += BorderRect.right - BorderRect.left;
    WindowHeight += BorderRect.bottom - BorderRect.top;

    hWnd = CreateWindowExA(WindowExStyle, WindowClass, ApplicationName.c_str(),
                           WindowStyle, WindowX, WindowY, WindowWidth, WindowHeight,
                           0, 0, hInstance, 0);

    if (hWnd == 0)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    bool bShouldActivate = true;
    int32_t ShowWindowCommandFlags = bShouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(hWnd, ShowWindowCommandFlags);

    return true;
}

void PlatformWin32::Shutdown()
{
    if (hWnd != 0)
    {
        DestroyWindow(hWnd);
        hWnd = 0;
    }
}

LRESULT PlatformWin32::Win32ProcessMessage(HWND hWnd, uint32_t Message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcA(hWnd, Message, wParam, lParam);
}

#endif // MPLATFORM_WINDOWS

#ifdef MPLATFORM_LINUX
#endif // MPLATFORM_LINUX