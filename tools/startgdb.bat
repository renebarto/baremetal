@echo off
set thisdir=%~dp0

D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gdb.exe -x %thisdir%\gdb-commands.txt -symbols=%CD%\output\Debug\bin\%1.elf --args %CD%\output\Debug\bin\%1.elf
