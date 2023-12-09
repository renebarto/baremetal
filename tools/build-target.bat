set ROOT=%CD%

pushd tutorial/01-building

cmake --build %ROOT%/cmake-build --target %1

popd