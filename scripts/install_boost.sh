wget "https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.zip" -O boost.zip
unzip boost.zip -d ../dependencies/
rm boost.zip

cd ../dependencies/boost-1.86.0/ || (echo "Can't find boost directory" && exit 1)
find . -type f -name '*.sh' -exec chmod +x {} \;

./bootstrap.sh

chmod +x ./b2
./b2 variant=release debug-symbols=on link=static
