#pragma once

namespace CE::RHI
{

	class CORERHI_API ShaderModule : public RHIResource
	{
	protected:
		ShaderModule() : RHIResource(ResourceType::ShaderModule)
		{}

	public:
		virtual ~ShaderModule() = default;

		// - Public API -

		inline const Name& GetDebugName() const { return name; }

		inline bool IsValidShader() const { return isValid; }

		inline ShaderStage GetShaderStage() const { return stage; }

		bool IsVertexShader() { return GetShaderStage() == ShaderStage::Vertex; }
		bool IsFragmentShader() { return GetShaderStage() == ShaderStage::Fragment; }

		inline SIZE_T GetHash() const { return hash; }

        inline const Name& GetDefaultEntryPoint() const { return defaultEntryPoint; }
        
	protected:

		Name name{};
		bool isValid = false;
		SIZE_T hash = 0;
        Name defaultEntryPoint{};

		ShaderStage stage{};

	};
    
} // namespace CE::RHI
