pushd doc/tutorials
ln -s ../images images
ln -s ../pdf pdf
popd

pushd doc/boards/raspberrypi
ln -s ../../images images
ln -s ../../pdf pdf
popd

pushd doc/cpu/arm
ln -s ../../images images
ln -s ../../pdf pdf
popd
