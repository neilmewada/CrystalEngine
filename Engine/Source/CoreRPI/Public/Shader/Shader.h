#pragma once

namespace CE::RPI
{
	/// @brief RPI::Shader is effectively a collection of shader variants.
	class CORERPI_API Shader final
	{
	public:

		Shader(RHI::DrawListTag drawListTag = {});
		~Shader();

		Name GetName();

		inline u32 GetVariantCount() const { return variants.GetSize(); }

		inline RPI::ShaderVariant* GetVariant(u32 index) const 
		{ 
			if (index >= variants.GetSize()) 
				return nullptr;
			return variants[index]; 
		}

		RPI::ShaderVariant* GetDefaultVariant() const { return GetVariant(GetDefaultVariantIndex()); }

		RHI::PipelineState* GetDefaultPipeline() const
		{
			RPI::ShaderVariant* variant = GetDefaultVariant();

			return variant->GetPipeline();
		}

		RPI::ShaderVariant* AddVariant(const ShaderVariantDescriptor& variantDesc);

		inline u32 GetDefaultVariantIndex() const { return defaultVariantIndex; }

		inline void SetDefaultVariantIndex(u32 index) { defaultVariantIndex = index; }

		void SetDrawListTag(RHI::DrawListTag tag) { drawListTag = tag; }

		RHI::DrawListTag GetDrawListTag() const { return drawListTag; }

	private:

		u32 defaultVariantIndex = 0;

		Array<RPI::ShaderVariant*> variants{};

		RHI::DrawListTag drawListTag{};

		friend class Material;
	};

} // namespace CE::RPI
