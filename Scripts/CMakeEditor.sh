#!/bin/bash

base_dir=$(pwd)
#num_cores=$(nproc --all)
DefaultGenerator="Ninja Multi-Config"
Compiler=""

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

cmake -B "${base_dir}/Build/${platform}" -S "${base_dir}" -G "${DefaultGenerator}" ${Compiler} -Wno-dev "$@"

