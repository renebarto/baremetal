rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build

cmake ../tutorial/01-building -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/01-building/baremetal.toolchain

popd
