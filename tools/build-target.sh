rootdir=`pwd`

pushd tutorial/01-building

cmake --build $rootdir/cmake-build --target $1

popd
