#pragma once

#include "CoreMinimal.h"
#include "CoreApplication.h"
#include "CoreRHI.h"

#include "vulkan/vulkan.h"

//#define VULKAN_CPU_ALLOCATOR &CE::Vulkan::gVulkanAllocators
#define VULKAN_CPU_ALLOCATOR nullptr

#include "DeviceLimits.h"
#include "VulkanRHI.h"
#include "CommandQueue.h"
#include "Device.h"
#include "SwapChain.h"
#include "Viewport.h"
#include "VulkanRenderPass.h"
#include "RenderPassCache.h"

#include "RenderPass.h"
#include "RenderPassFrameBuffer.h"

#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "ShaderResourceGroup.h"
#include "MergedShaderResourceGroup.h"
#include "MemoryHeap.h"
#include "Sampler.h"
#include "Texture.h"
#include "Buffer.h"
#include "TextureView.h"

#include "ShaderModule.h"
#include "Pipeline.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "PipelineState.h"

#include "FrameBuffer.h"
#include "FrameBufferCache.h"
#include "SwapChain.h"
#include "ShaderResourceGroup.h"
#include "Fence.h"
#include "CommandList.h"
#include "CommandBufferAllocator.h"

// Ray Tracing
#include "RayTracingAccelerationStructure.h"
#include "RayTracingBlas.h"
#include "RayTracingTlas.h"

#include "AliasedHeap.h"

// Frame
#include "Scope.h"
#include "TransientAttachmentPool.h"
#include "FrameGraphCompiler.h"
#include "FrameGraphExecuter.h"

namespace CE::Vulkan
{

	extern VkAllocationCallbacks gVulkanAllocators;

} // namespace CE::Vulkan
