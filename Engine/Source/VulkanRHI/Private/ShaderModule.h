#pragma once

namespace CE::Vulkan
{
    
	class ShaderModule : public RHI::ShaderModule
	{
	public:

		ShaderModule(Device* device, const RHI::ShaderModuleDescriptor& desc);
		~ShaderModule();

		inline VkShaderModule GetHandle() const { return shaderModule; }

	private:

		Device* device = nullptr;
		VkShaderModule shaderModule = nullptr;

		HashMap<Name, int> variableBindingMap{};

	};

} // namespace CE
