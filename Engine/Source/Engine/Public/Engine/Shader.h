#pragma once


namespace CE
{
#if PAL_TRAIT_BUILD_EDITOR
	namespace Editor { class ShaderAssetImportJob; }
#endif

	CLASS()
	class ENGINE_API ShaderBlob : public Object
	{
		CE_CLASS(ShaderBlob, Object)
	public:

		virtual ~ShaderBlob();

		inline bool IsValid() const
		{
			return byteCode.IsValid();
		}

		FIELD()
		ShaderBlobFormat format = ShaderBlobFormat::Spirv;

		FIELD()
		RHI::ShaderStage shaderStage = RHI::ShaderStage::None;

		FIELD()
		BinaryBlob byteCode{};

		friend class CE::Shader;
	};

	STRUCT()
	struct ENGINE_API ShaderVariant
	{
		CE_STRUCT(ShaderVariant)
	public:

		void Release();

		FIELD()
		Array<String> defineFlags{};

		FIELD()
		SIZE_T variantHash = 0;

		FIELD()
		Array<ShaderBlob*> shaderStageBlobs{};

		FIELD()
		ShaderReflection reflectionInfo{};

		ShaderBlob* GetShaderBlobForStage(RHI::ShaderStage stage);

		friend class CE::Shader;
	};

	STRUCT()
	struct ENGINE_API ShaderPass 
	{
		CE_STRUCT(ShaderPass)
	public:

		FIELD()
		Name passName = "Main";

		FIELD()
		Array<ShaderTagEntry> tags{};

		FIELD()
		Array<CE::ShaderVariant> variants{};

		FIELD()
		Name vertexEntry = "VertMain";

		FIELD()
		Name fragmentEntry = "FragMain";

		FIELD()
		BinaryBlob hlslSource{};

		FIELD()
		Array<String> features{};

		bool TagExists(const Name& key) const
		{
			return tags.Exists([&](const ShaderTagEntry& entry) { return entry.key == key; });
		}

		String GetTagValue(const Name& key) const
		{
			int index = tags.IndexOf([&](const ShaderTagEntry& entry) { return entry.key == key; });
			if (index >= 0 && index < tags.GetSize())
			{
				return tags[index].value;
			}
			return "";
		}

		friend class CE::Shader;
	};

	STRUCT()
	struct ENGINE_API SubShader
	{
		CE_STRUCT(SubShader)
	public:

		FIELD()
		Array<ShaderTagEntry> tags{};

		FIELD()
		Array<ShaderPass> passes{};

		inline bool TagExists(const Name& key) const
		{
			return tags.Exists([&](const ShaderTagEntry& entry) { return entry.key == key; });
		}

		inline String GetTagValue(const Name& key) const
		{
			int index = tags.IndexOf([&](const ShaderTagEntry& entry) { return entry.key == key; });
			if (index >= 0 && index < tags.GetSize())
			{
				return tags[index].value;
			}
			return "";
		}
	};


	CLASS()
	class ENGINE_API Shader : public Asset
	{
		CE_CLASS(Shader, Asset)
	public:

		Shader();
		virtual ~Shader(); 

		void OnAfterConstruct() override;

		inline Name GetShaderName() const
		{
			return shaderName;
		}

		inline u32 GetShaderPassCount() { return GetSubshader()->passes.GetSize(); }

		inline ShaderPass* GetShaderPass(int index)
		{
			return &GetSubshader()->passes[index];
		}

		inline Name GetShaderPassName(int index)
		{
			return GetSubshader()->passes[index].passName;
		}

		RPI::ShaderCollection* GetShaderCollection();

		const Array<ShaderPropertyEntry>& GetProperties() const { return properties; }

	protected:

		SubShader* GetSubshader();

		SharedMutex rpiShaderMutex{};
		RPI::ShaderCollection shaderCollection{};

		FIELD()
		Name shaderName{};

		FIELD()
		Array<ShaderPropertyEntry> properties{};

		FIELD()
		Array<SubShader> subShaders{};

#if PAL_TRAIT_BUILD_EDITOR
		friend class CE::Editor::ShaderAssetImportJob;
#endif
	};
    
} // namespace CE

#include "Shader.rtti.h"
