#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WinMainCRTStartup(void)
{
    SendMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
    return 0;
}
