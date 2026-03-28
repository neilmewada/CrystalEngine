#pragma once

namespace CE::RHI
{
	class TextureView;

	class CORERHI_API ShaderResourceGroup : public RHIResource
	{
	protected:
		ShaderResourceGroup() : RHIResource(ResourceType::ShaderResourceGroup)
		{}

	public:

		virtual bool HasVariable(const Name& variableName) = 0;

		virtual bool Bind(Name name, RHI::BufferView bufferView) = 0;
		virtual bool Bind(Name name, RHI::Texture* texture) = 0;
		virtual bool Bind(Name name, RHI::TextureView* textureViews) = 0;
		virtual bool Bind(Name name, RHI::Sampler* sampler) = 0;

		virtual bool Bind(Name name, u32 count, RHI::BufferView* bufferViews) = 0;
		virtual bool Bind(Name name, u32 count, RHI::Texture** textures) = 0;
		virtual bool Bind(Name name, u32 count, RHI::TextureView** textureViews) = 0;
		virtual bool Bind(Name name, u32 count, RHI::Sampler** samplers) = 0;

		virtual bool Bind(u32 imageIndex, Name name, RHI::BufferView bufferView) = 0;
		virtual bool Bind(u32 imageIndex, Name name, RHI::Texture* texture) = 0;
		virtual bool Bind(u32 imageIndex, Name name, RHI::TextureView* textureView) = 0;
		virtual bool Bind(u32 imageIndex, Name name, RHI::Sampler* sampler) = 0;

		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::BufferView* bufferViews) = 0;
		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::Texture** textures) = 0;
		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::TextureView** textureViews) = 0;
		virtual bool Bind(u32 imageIndex, Name name, u32 count, RHI::Sampler** samplers) = 0;

		virtual bool Bind(Name name, u32 firstArrayElement, u32 count, RHI::Texture** textures) = 0;
		virtual bool Bind(u32 imageIndex, Name name, u32 firstArrayElement, u32 count, RHI::Texture** textures) = 0;
		virtual bool Bind(Name name, u32 firstArrayElement, u32 count, RHI::TextureView** textureViews) = 0;
		virtual bool Bind(u32 imageIndex, Name name, u32 firstArrayElement, u32 count, RHI::TextureView** textureViews) = 0;

		inline SRGType GetSRGType() const
		{
			return srgType;
		}

		inline const ShaderResourceGroupLayout& GetLayout() const
		{
			return srgLayout;
		}

		inline bool IsCompiled() const
		{
			return isCompiled;
		}

		/// @brief Compile the Shader Resource Group.
		virtual void Compile() = 0;

		inline bool IsCommitted() const
		{
			return isCommitted;
		}

		//! @brief Apply the binding updates to the shader resource.
		virtual void FlushBindings() = 0;

	protected:

		b8 isCompiled = false;

		b8 isCommitted = false;
		SRGType srgType{};

		ShaderResourceGroupLayout srgLayout{};

	};
    
} // namespace CE::RHI

