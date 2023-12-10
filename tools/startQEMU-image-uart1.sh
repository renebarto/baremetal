./build-target $1
set rootdir=`pwd`
qemu-system-aarch64 -M raspi3b -kernel $rootdir/deploy/Debug/$$1-image/kernel8.img -serial null -serial stdio -s -S
