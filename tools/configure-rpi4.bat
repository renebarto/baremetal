@echo off
del /S /f /q cmake-Baremetal-Debug\*.*
rmdir /s /q cmake-Baremetal-Debug
mkdir cmake-Baremetal-Debug
pushd cmake-Baremetal-Debug

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DBAREMETAL_TARGET=RPI4 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
