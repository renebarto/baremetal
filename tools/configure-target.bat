del /S /f /q cmake-build\*.*
rmdir cmake-build
mkdir cmake-build
pushd cmake-build

::cmake .. -G Ninja -DVERSION_NUMBER="1.0.0" -DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3 -DBAREMETAL_CONSOLE_UART0=ON -DCMAKE_TOOLCHAIN_FILE=..\baremetal.toolchain
cmake ../tutorial/01-building -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/01-building/baremetal.toolchain

popd
