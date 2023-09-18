#include "Platform.h"

#ifdef MPLATFORM_WINDOWS

#include "core/Logger.h"
#include "core/Event.h"
#include "core/Input.h"

#include <cstdlib>

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

    if (!hWnd)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    bool bShouldActivate = true;
    int32_t ShowWindowCommandFlags = bShouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(hWnd, ShowWindowCommandFlags);

    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    ClockFrequency = 1.f / (double)Frequency.QuadPart;
    QueryPerformanceCounter(&StartTime);

    return true;
}

void PlatformWin32::Shutdown()
{
    if (hWnd)
    {
        DestroyWindow(hWnd);
        hWnd = nullptr;
    }
}

bool PlatformWin32::PumpMessages()
{
    MSG Message;
    while(PeekMessageA(&Message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }

    return true;
}


void* PlatformWin32::PlatformAllocate(size_t Size, bool bAligned)
{
    return malloc(Size);
}

void  PlatformWin32::PlatformFree(void* Block, bool bAligned)
{
    free(Block);
}

void* PlatformWin32::PlatformZeroMemory(void* Block, size_t Size)
{
    return memset(Block, 0, Size);
}

void* PlatformWin32::PlatformCopyMemory(void* Dst, const void* Src, size_t Size)
{
    return memcpy(Dst, Src, Size);
}

void* PlatformWin32::PlatformSetMemory(void* Dst, int32_t Value, size_t Size)
{
    return memset(Dst, Value, Size);
}

void PlatformWin32::ConsoleWrite(const std::string& Message, uint8_t Color)
{
    HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    static uint8_t Levels[6] = { 64, 4, 6, 1, 2, 8 }; // FATAL, ERROR, WARNING, INFO, DEBUG, VERBOSE
    SetConsoleTextAttribute(ConsoleHandle, Levels[Color]);

    OutputDebugStringA(Message.c_str());
    LPDWORD NumberWritten = nullptr;
    WriteConsoleA(ConsoleHandle, Message.c_str(), static_cast<DWORD>(Message.length()), NumberWritten, nullptr);
}

void PlatformWin32::ConsoleWriteError(const std::string& Message, uint8_t Color)
{
    HANDLE ConsoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    static uint8_t Levels[6] = { 64, 4, 6, 1, 2, 8 }; // FATAL, ERROR, WARNING, INFO, DEBUG, VERBOSE
    SetConsoleTextAttribute(ConsoleHandle, Levels[Color]);

    OutputDebugStringA(Message.c_str());
    LPDWORD NumberWritten = nullptr;
    WriteConsoleA(ConsoleHandle, Message.c_str(), Message.length(), NumberWritten, nullptr);
}

double PlatformWin32::GetAbsoluteTime()
{
    LARGE_INTEGER NowTime;
    QueryPerformanceCounter(&NowTime);
    return (double)NowTime.QuadPart * ClockFrequency;
}

void PlatformWin32::PlatformSleep(uint64_t ms)
{
    Sleep(ms);
}

LRESULT PlatformWin32::Win32ProcessMessage(HWND hWnd, uint32_t Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_ERASEBKGND:
            return 1;
        case WM_CLOSE:
            {
                EventContext Data {};
                EventSystem::Get()->FireEvent(static_cast<uint16_t>(SystemEventCode::EVENT_CODE_APPLICATION_QUIT), nullptr, Data);
                return true;
            }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                bool bPressed = (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN);
                KeyboardKey Key = (KeyboardKey)wParam;

                bool bExtended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

                if (wParam == VK_MENU) // Alt Key
                {
                    Key = bExtended ? KEY_RALT : KEY_LALT;
                } 
                else if (wParam == VK_SHIFT) // Shift Key
                {
                    uint32_t LeftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                    uint32_t ScanCode = ((lParam & (0xFF << 16)) >> 16);
                    Key = ScanCode == LeftShift ? KEY_LSHIFT : KEY_RSHIFT;
                }
                else if (wParam == VK_CONTROL) // Ctrl Key
                {
                    Key = bExtended ? KEY_RCONTROL : KEY_LCONTROL;
                }

                InputSystem::Get()->ProcessKey(Key, bPressed);
            }
            break;
        case WM_MOUSEMOVE:
            {
                int32_t XPos = GET_X_LPARAM(lParam);
                int32_t YPos = GET_Y_LPARAM(lParam);
                
                InputSystem::Get()->ProcessMouseMove(XPos, YPos);
            }
            break;
        case WM_MOUSEWHEEL:
            {
                int32_t WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                if (WheelDelta != 0)
                {
                    WheelDelta = (WheelDelta < 0) ? -1 : 1;
                    
                    InputSystem::Get()->ProcessMouseWheel(WheelDelta);
                }
            }
            break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            {
                bool bPressed = (Message == WM_LBUTTONDOWN || Message == WM_MBUTTONDOWN || Message == WM_RBUTTONDOWN);
                
                MouseButton Button = MouseButton::MOUSE_BUTTON_MAX;
                switch (Message)
                {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    Button = MouseButton::MOUSE_BUTTON_LEFT;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    Button = MouseButton::MOUSE_BUTTON_RIGHT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    Button = MouseButton::MOUSE_BUTTON_MIDDLE;
                    break;        
                default:
                    break;
                }

                InputSystem::Get()->ProcessMouseButton(Button, bPressed);
            }
            break;
        default:
            break;
    }

    return DefWindowProcA(hWnd, Message, wParam, lParam);
}

#endif // MPLATFORM_WINDOWS

#ifdef MPLATFORM_LINUX
#endif // MPLATFORM_LINUX