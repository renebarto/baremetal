rm -rf cmake-Baremetal-RPI5-Debug/
mkdir cmake-Baremetal-RPI5-Debug
pushd cmake-Baremetal-RPI5-Debug

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DBAREMETAL_TARGET=RPI5 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
