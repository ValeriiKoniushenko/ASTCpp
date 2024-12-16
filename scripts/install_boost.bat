@echo off

cd ../dependencies/ || (echo Folder "dependencies" not found && pause && exit 1)

if not exist boost-1.86.0 (
    curl -L --output boost.zip "https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.zip"
    tar -xf boost.zip
    del boost.zip
)

cd boost-1.86.0/ || (echo Can't find the unzipped boost directory && pause && exit 1)

set defaultPath=%cd%

bootstrap.bat && (
    cd %defaultPath%
    b2 variant=release debug-symbols=on link=static || (echo Was met some error while running of boost "b2" && pause && exit 1)
) || (
    echo Was met some error while running of "bootstrap.bat" && pause && exit 1
)

exit 0