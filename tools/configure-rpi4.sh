rm -rf cmake-build-rpi4/
mkdir cmake-build-rpi4
pushd cmake-build-rpi4

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DBAREMETAL_TARGET=RPI4 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
