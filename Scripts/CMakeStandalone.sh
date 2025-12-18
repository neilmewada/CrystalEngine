#!/bin/sh

# Setup variables

base_dir=$(pwd)
DefaultGenerator="Ninja Multi-Config"
Compiler=""

cd $1

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    host_platform="Linux"
    Compiler="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    host_platform="Mac"
    DefaultGenerator="Xcode"
elif [[ "$OSTYPE" == "cygwin" ]]; then
    host_platform="Windows"
    DefaultGenerator="Visual Studio 18"
elif [[ "$OSTYPE" == "msys" ]]; then
    host_platform="Windows"
    DefaultGenerator="Visual Studio 18"
elif [[ "$OSTYPE" == "win32" ]]; then
    host_platform="Windows"
    DefaultGenerator="Visual Studio 18"
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    host_platform="Linux"
    Compiler="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
else
    echo "Unknown Operating System"
    exit 1
fi

cmake -B "${base_dir}/Build/${host_platform}-Standalone" -S "${base_dir}" -G "${DefaultGenerator}" -DCE_STANDALONE=ON -DCE_HOST_BUILD_DIR="Build/${host_platform}/Debug" -Wno-dev "$@" #-DCMAKE_SYSTEM_NAME="${SystemName}"

