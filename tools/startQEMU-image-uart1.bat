call build-target %1
"c:\Program Files\qemu\qemu-system-aarch64.exe" -M raspi3b -kernel %CD%\deploy\Debug\%1-image\kernel8.img -serial null -serial stdio -s -S
