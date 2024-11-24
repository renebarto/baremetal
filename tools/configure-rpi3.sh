rm -rf cmake-Baremetal-RPI3-Debug/
mkdir cmake-Baremetal-RPI3-Debug
pushd cmake-Baremetal-RPI3-Debug

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DBAREMETAL_TARGET=RPI3 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
