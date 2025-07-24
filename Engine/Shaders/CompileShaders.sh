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

DxCompiler=${VULKAN_SDK}/bin/dxc

CompileShader() {

    local FileName=$1".hlsl"
    local VertOut="../Source/FusionCore/Resources/Shaders/"$1".vert.spv"
    local FragOut="../Source/FusionCore/Resources/Shaders/"$1".frag.spv"

    local VertReflection="../Source/FusionCore/Resources/Shaders/"$1".vert.json"
    local FragReflection="../Source/FusionCore/Resources/Shaders/"$1".frag.json"

    ${DxCompiler} -spirv -E VertMain -T vs_6_0 -D COMPILE=1 -D VERTEX=1 -fspv-preserve-bindings -Fo ${VertOut} ${FileName} -fspv-debug=vulkan-with-source -Zi #-fvk-use-dx-layout
    ${DxCompiler} -spirv -E FragMain -T ps_6_0 -D COMPILE=1 -D FRAGMENT=1 -fspv-preserve-bindings -Fo ${FragOut} ${FileName} -fspv-debug=vulkan-with-source -Zi #-fvk-use-dx-layout

    spirv-cross --dump-resources ${VertOut} --reflect --output ${VertReflection}
    spirv-cross --dump-resources ${FragOut} --reflect --output ${FragReflection}

    echo "Compiled: " ${FileName}
}

CompileCS() {
    local FileName=$1".hlsl"
    local Out="../Source/VulkanSandbox/Resources/Shaders/"$1".spv"

    ${DxCompiler} -spirv -E $2 -T cs_6_0 -fspv-preserve-bindings -fspv-extension=KHR -Fo ${Out} ${FileName}

    echo "Compiled: " ${FileName}
}

CompileShader "FusionShader"
CompileShader "FusionShader2"
CompileShader "FusionSDFGlyphGen"

