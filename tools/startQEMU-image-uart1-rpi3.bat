@echo off
set thisdir=%~dp0

call %thisdir%\build-image-rpi3 %1
"c:\Program Files\qemu\qemu-system-aarch64.exe" -M raspi3b -kernel %thisdir%\..\deploy\Debug\%1-image\kernel8.img -serial null -serial stdio -s -S
