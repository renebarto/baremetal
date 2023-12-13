@echo off
rmdir /s /q cmake-Baremetal-Debug
mkdir cmake-Baremetal-Debug
pushd cmake-Baremetal-Debug

::cmake .. -G Ninja -DVERSION_NUMBER="1.0.0" -DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3 -DBAREMETAL_CONSOLE_UART0=ON -DCMAKE_TOOLCHAIN_FILE=..\baremetal.toolchain
cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
