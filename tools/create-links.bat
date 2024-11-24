pushd doc\tutorials
mklink /D images ..\images
mklink /D pdf ..\pdf
popd

pushd doc\board
mklink /D images ..\images
mklink /D pdf ..\pdf
popd

pushd doc\cpu
mklink /D images ..\images
mklink /D pdf ..\pdf
popd
