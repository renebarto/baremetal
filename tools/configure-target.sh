rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build

cmake ../tutorial/01-building -G Ninja

popd
