#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define WM_PREVENT_SLEEP_COMMAND   (WM_USER + 1)
#define WM_PREVENT_SLEEP_DUPLICATE (WM_USER + 2)

#define PREVENT_SLEEP_TITLE L"PreventSleep"

static UINT WM_TASKBARCREATED;
static HICON PreventSleepIcon;

static void* memcpy(void* dst, const void* src, size_t count)
{
    __asm__ __volatile__("rep movsb" : "+D"(dst), "+S"(src), "+c"(count) : : "memory");
    return dst;
}

static void* memset(void* dst, int value, size_t count)
{
    __asm__ __volatile__("rep stosb" : "+D"(dst), "+c"(count) : "a"(value) : "memory");
    return dst;
}

static void AddTrayIcon(HWND window)
{
    NOTIFYICONDATAW data =
    {
        .cbSize = sizeof(data),
        .hWnd = window,
        .uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP,
        .uCallbackMessage = WM_PREVENT_SLEEP_COMMAND,
        .hIcon = PreventSleepIcon,
        .szTip = PREVENT_SLEEP_TITLE,
    };
    Shell_NotifyIconW(NIM_ADD, &data);
}

static void RemoveTrayIcon(HWND window)
{
    NOTIFYICONDATAW data =
    {
        .cbSize = sizeof(data),
        .hWnd = window,
    };
    Shell_NotifyIconW(NIM_DELETE, &data);
}

static void ShowDuplicateNotification(HWND window)
{
    NOTIFYICONDATAW data =
    {
        .cbSize = sizeof(data),
        .hWnd = window,
        .uFlags = NIF_INFO,
        .dwInfoFlags = NIIF_INFO,
        .szInfo = PREVENT_SLEEP_TITLE L" is already running!",
        .szInfoTitle = PREVENT_SLEEP_TITLE,
    };
    Shell_NotifyIconW(NIM_MODIFY, &data);
}

static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            AddTrayIcon(window);
            return 0;
        }

        case WM_DESTROY:
        {
            RemoveTrayIcon(window);
            PostQuitMessage(0);
            return 0;
        }

        case WM_PREVENT_SLEEP_COMMAND:
        {
            if (lparam == WM_RBUTTONUP)
            {
                const UINT_PTR CMD_QUIT = 1;

                HMENU menu = CreatePopupMenu();
                AppendMenuW(menu, MF_STRING, CMD_QUIT, L"Quit");

                POINT mouse;
                GetCursorPos(&mouse);

                SetForegroundWindow(window);
                int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, mouse.x, mouse.y, 0, window, NULL);
                if (cmd == CMD_QUIT)
                {
                    DestroyWindow(window);
                }

                DestroyMenu(menu);
            }
            break;
        }

        case WM_PREVENT_SLEEP_DUPLICATE:
        {
            ShowDuplicateNotification(window);
            return 0;
        }

        default:
        {
            if (WM_TASKBARCREATED != 0 && msg == WM_TASKBARCREATED)
            {
                AddTrayIcon(window);
                return 0;
            }
            break;
        }
    }

    return DefWindowProcW(window, msg, wparam, lparam);
}

int WinMainCRTStartup(void)
{
    WNDCLASSEXW wc =
    {
        .cbSize = sizeof(wc),
        .lpfnWndProc = WindowProc,
        .hInstance = GetModuleHandleW(NULL),
        .lpszClassName = PREVENT_SLEEP_TITLE L"WindowClass",
    };

    HWND existing = FindWindowW(wc.lpszClassName, NULL);
    if (existing)
    {
        PostMessageW(existing, WM_PREVENT_SLEEP_DUPLICATE, 0, 0);
        return 0;
    }

    RegisterClassExW(&wc);

    WM_TASKBARCREATED = RegisterWindowMessageW(L"TaskbarCreated");
    PreventSleepIcon = LoadIconA(NULL, IDI_EXCLAMATION);

    HWND window = CreateWindowExW(0, wc.lpszClassName, PREVENT_SLEEP_TITLE,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);

    EXECUTION_STATE prev = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
    if (prev == 0)
    {
        WCHAR* msg;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&msg, 0, NULL);
        MessageBoxW(NULL, msg, L"Error", MB_OK | MB_ICONERROR);
        LocalFree(msg);
        return 0;
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    SetThreadExecutionState(prev);
    return 0;
}
