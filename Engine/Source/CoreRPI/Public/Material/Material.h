#pragma once

namespace CE
{
	class Material;
	class MaterialInstance;
	class MaterialInterface;
}

namespace CE::RPI
{
	class Shader;
	class ShaderCollection;
	class ShaderVariant;

	typedef HashMap<Name, MaterialPropertyValueArray> MaterialPropertyValueMap;

	class CORERPI_API Material
	{
	public:

		Material(Shader* shader);

		Material(ShaderCollection* shaderCollection);
		~Material();

		ShaderCollection* GetShaderCollection() const;

		void SetShaderCollection(ShaderCollection* shaderCollection);

		void SetDefaultShaderItem(int shaderItem);

		Shader* GetOpaqueShader() const;

		Shader* GetCurrentShader();

		//ShaderVariant* GetCurrentOpaqueShader();

		void FlushProperties();
        
        bool IsCompiled() const
        {
            return shaderResourceGroup != nullptr && shaderResourceGroup->IsCompiled();
        }

		bool PropertyExists(Name name) const;
        
		void ClearAllValues();

		void SetCurrentShaderItem(int itemIndex);

		RPI::Shader* GetCurrentShaderItem();

        void SetPropertyValue(Name propertyName, const MaterialPropertyValue& value);

		//! Used for shader resource arrays
		void InsertPropertyValue(Name propertyName, const MaterialPropertyValue& value, int index = -1);

		//! Used for shader resource arrays
		void RemovePropertyValue(Name propertyName, int index);

		void ClearPropertyValue(Name propertyName);

		const MaterialPropertyValue& GetPropertyValue(Name propertyName);

		//! Used for shader resource arrays
		const MaterialPropertyValue& GetPropertyValue(Name propertyName, int index);
        
        RHI::ShaderResourceGroup* GetShaderResourceGroup() const { return shaderResourceGroup; }

		void CopyPropertiesFrom(RPI::Material* other);

		void SetBaseMaterial(RPI::Material* mat) { baseMaterial = mat; }

	private:

		void FlushProperties(u32 imageIndex);

		void RecreateShaderResourceGroup();
		
		RHI::ShaderResourceGroup* shaderResourceGroup = nullptr;

		RPI::Material* baseMaterial = nullptr;

		MaterialPropertyValueMap properties{};
        
        StaticArray<HashMap<Name, RHI::Buffer*>, RHI::Limits::MaxSwapChainImageCount> buffersByVariableNamePerImage{};
		StaticArray<bool, RHI::Limits::MaxSwapChainImageCount> updateRequired{};

        HashMap<Name, Array<u64>> memberOffsetsByVariableName{};

		bool ownsShaderCollection;
		ShaderCollection* shaderCollection;

		RPI::Shader* currentShader = nullptr;

		//u32 shaderVariantIndex = 0;

		int currentShaderItem = -1;

		friend class MaterialInstance;
		friend class MaterialInterface;
		friend class CE::Material;
		friend class MaterialSystem;
		friend class Renderer2D;
	};
    
} // namespace CE::RPI
