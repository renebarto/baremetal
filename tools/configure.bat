@echo off
rmdir /s /q cmake-Baremetal-Debug
mkdir cmake-Baremetal-Debug
pushd cmake-Baremetal-Debug

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
