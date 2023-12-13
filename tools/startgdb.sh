rootdir=`pwd`
gdb-multiarch -x $rootdir/tools/gdb-commands.txt -symbols=$rootdir/output/Debug/bin/$1.elf
