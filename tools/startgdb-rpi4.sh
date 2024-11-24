rootdir=`pwd`
gdb-multiarch -x $rootdir/tools/gdb-commands.txt -symbols=$rootdir/output/RPI4/Debug/bin/$1.elf --args $rootdir/output/RPI4/Debug/bin/$1.elf
