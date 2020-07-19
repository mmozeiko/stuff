"%ProgramFiles%\LLVM\bin\clang.exe" -Os -fno-unwind-tables -nostdlib -fuse-ld=lld -Wl,-fixed,-merge:.rdata=.text,-subsystem:windows,user32.lib TurnOffMonitors.c -o TurnOffMonitors.exe
