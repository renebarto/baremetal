doxygen doxygen/doxygen.conf
rm -rf doxygen/html/images
mkdir -p doxygen/html/images
cp -f doc/images/* doxygen/html/images
rm -rf doxygen/html/pdf
mkdir -p doxygen/html/pdf
cp -f doc/pdf/* doxygen/html/pdf
