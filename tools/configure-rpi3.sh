rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build

cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DBAREMETAL_TARGET=RPI3 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../baremetal.toolchain

popd
