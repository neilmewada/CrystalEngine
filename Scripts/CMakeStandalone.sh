#!/bin/sh

# Setup variables

base_dir=$(pwd)
DefaultGenerator="Ninja Multi-Config"
Compiler=""

cd $1

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    platform="Linux"
    SystemName="Linux"
    Compiler="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    platform="Mac"
    DefaultGenerator="Xcode"
    SystemName="Darwin"
elif [[ "$OSTYPE" == "cygwin" ]]; then
    platform="Windows"
    SystemName="Windows"
    DefaultGenerator="Visual Studio 18"
elif [[ "$OSTYPE" == "msys" ]]; then
    platform="Windows"
    SystemName="Windows"
    DefaultGenerator="Visual Studio 18"
elif [[ "$OSTYPE" == "win32" ]]; then
    platform="Windows"
    SystemName="Windows"
    DefaultGenerator="Visual Studio 18"
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    platform="Linux"
    SystemName="Linux"
    Compiler="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
else
    echo "Unknown Operating System"
    exit 1
fi

cmake -B "${base_dir}/Build/${platform}-Standalone" -S "${base_dir}" -G "${DefaultGenerator}" -DCE_STANDALONE=ON -DCE_HOST_BUILD_DIR="Build/${platform}/Debug" -Wno-dev "$@" #-DCMAKE_SYSTEM_NAME="${SystemName}"

