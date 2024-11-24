thisdir=$(dirname "$0")
echo thisdir=$thisdir

echo "$thisdir/build-image-rpi3.sh $1"
$thisdir/build-image-rpi3.sh $1

echo qemu-system-aarch64 -M raspi3b -kernel $thisdir/../deploy/Debug/$1-image/kernel8.img -serial null -serial stdio -s -S
qemu-system-aarch64 -M raspi3b -kernel $thisdir/../deploy/Debug/$1-image/kernel8.img -serial null -serial stdio -s -S
