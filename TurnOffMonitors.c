// clang -Os -fno-unwind-tables -nostdlib -fuse-ld=lld -Wl,-fixed,-merge:.rdata=.text,-subsystem:windows,user32.lib TurnOffMonitors.c -o TurnOffMonitors.exe

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WinMainCRTStartup(void)
{
    SendMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
    return 0;
}
