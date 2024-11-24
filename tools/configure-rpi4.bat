@echo off
del /S /f /q cmake-Baremetal-RPI4-Debug\*.*
rmdir /s /q cmake-Baremetal-RPI4-Debug
mkdir cmake-Baremetal-RPI4-Debug
pushd cmake-Baremetal-RPI4-Debug

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DBAREMETAL_TARGET=RPI4 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
