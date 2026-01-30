#pragma once

namespace CE::Vulkan
{
	class Device;
    
	class VULKANRHI_API MergedShaderResourceGroup final : public ShaderResourceGroup
	{
	public:

		MergedShaderResourceGroup(Device* device, u32 srgCount, Vulkan::ShaderResourceGroup** srgs);

		~MergedShaderResourceGroup();

		inline SIZE_T GetMergedHash() const
		{
			return mergedHash;
		}

		bool IsMerged() const override final { return true; }

		SIZE_T GetHash() const override { return mergedHash; }

	private:

		Array<ShaderResourceGroup*> combinedSRGs{};
		SIZE_T mergedHash = 0;

		friend class ShaderResourceManager;
		friend class CommandList;
	};

} // namespace CE::Vulkan
