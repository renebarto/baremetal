pushd doc\tutorials
mklink /D images ..\images
mklink /D pdf ..\pdf
popd

pushd doc\boards\raspberrypi
mklink /D images ..\..\images
mklink /D pdf ..\..\pdf
popd

pushd doc\cpu\arm
mklink /D images ..\..\images
mklink /D pdf ..\..\pdf
popd
