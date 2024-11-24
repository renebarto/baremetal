rootdir=`pwd`
gdb-multiarch -x $rootdir/tools/gdb-commands.txt -symbols=$rootdir/output/RPI3/Debug/bin/$1.elf --args $rootdir/output/RPI3/Debug/bin/$1.elf
