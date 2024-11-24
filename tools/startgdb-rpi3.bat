@echo off
set thisdir=%~dp0

aarch64-none-elf-gdb.exe -x %thisdir%\gdb-commands.txt -symbols=%CD%\output\RPI3\Debug\bin\%1.elf --args %CD%\output\RPI3\Debug\bin\%1.elf
