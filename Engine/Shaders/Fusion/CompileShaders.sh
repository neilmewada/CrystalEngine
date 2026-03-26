#!/bin/sh

exe=""

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    platform="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    platform="mac"
elif [[ "$OSTYPE" == "cygwin" ]]; then
    platform="windows"
elif [[ "$OSTYPE" == "msys" ]]; then
    platform="windows"
elif [[ "$OSTYPE" == "win32" ]]; then
    platform="windows"
    exe=".exe"
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    platform="linux"
else
    echo "Unknown Operating System"
    exit 1
fi

#DxCompiler=${VULKAN_SDK}/bin/dxc
DxCompiler=$(eval echo ${VULKAN_SDK}/bin/dxc)

CompileShader() {

    local FileName=$1".hlsl"
    local VertOut="../../Source/FusionCore2/Resources/Shaders/"$1".vert.spv"
    local FragOut="../../Source/FusionCore2/Resources/Shaders/"$1".frag.spv"

    local VertReflection="../../Source/FusionCore2/Resources/Shaders/"$1".vert.json"
    local FragReflection="../../Source/FusionCore2/Resources/Shaders/"$1".frag.json"

    local VertMslOut="../../Source/FusionCore2/Resources/Shaders/"$1".vert.msl"
    local FragMslOut="../../Source/FusionCore2/Resources/Shaders/"$1".frag.msl"

    ${DxCompiler} -spirv -E VertMain -T vs_6_0 -D COMPILE=1 -D VERTEX=1 -fspv-preserve-bindings -Fo ${VertOut} ${FileName} -I "../" -fspv-debug=vulkan-with-source -Zi #-fvk-use-dx-layout
    ${DxCompiler} -spirv -E FragMain -T ps_6_0 -D COMPILE=1 -D FRAGMENT=1 -fspv-preserve-bindings -Fo ${FragOut} ${FileName} -I "../" -fspv-debug=vulkan-with-source -Zi #-fvk-use-dx-layout

    spirv-cross --dump-resources ${VertOut} --reflect --output ${VertReflection}
    spirv-cross --dump-resources ${FragOut} --reflect --output ${FragReflection}

    spirv-cross --msl --output ${VertMslOut} --msl-version 20300 --entry VertMain --msl-argument-buffers --msl-argument-buffer-tier 2 ${VertOut}
    spirv-cross --msl --output ${FragMslOut} --msl-version 20300 --entry FragMain --msl-argument-buffers --msl-argument-buffer-tier 2 --msl-device-argument-buffer 0 ${FragOut}

    echo "Compiled: " ${FileName}
}

CompileShader "Fusion"

# spirv-cross --msl --output FusionSDFGlyphGen.vert.msl --msl-version 20300 --entry VertMain --msl-argument-buffers --msl-decoration-binding FusionSDFGlyphGen.vert.spv
# spirv-cross --msl --output FusionSDFGlyphGen.frag.msl --msl-version 20300 --entry FragMain --msl-argument-buffers --msl-decoration-binding FusionSDFGlyphGen.frag.spv

