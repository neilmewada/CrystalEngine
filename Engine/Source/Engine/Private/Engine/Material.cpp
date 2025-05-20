#include "Engine.h"

namespace CE
{

    CE::Material::Material()
    {
        
    }

    CE::Material::~Material()
    {
        if (material)
        {
            delete material;
            material = nullptr;
        }
    }

    void CE::Material::SetShader(Ref<CE::Shader> shader)
    {
        if (this->shader == shader && material != nullptr && shaderCollection != nullptr)
            return;

        valuesModified = true;

        shaderCollection = shader->GetShaderCollection();

        this->shader = shader;
        auto newMaterial = new RPI::Material(shaderCollection);

        if (material)
        {
            newMaterial->CopyPropertiesFrom(material);

            delete material;
            material = nullptr;
        }

        this->material = newMaterial;

        propertyMap.Clear();

        for (const auto& prop : properties)
        {
            propertyMap[prop.name] = prop;
        }

        const auto& shaderDefaults = shader->GetProperties();

        for (const auto& defaultProp : shaderDefaults)
        {
	        if (!propertyMap.KeyExists(defaultProp.name))
	        {
                MaterialProperty property{};

		        switch (defaultProp.propertyType)
		        {
		        case ShaderPropertyType::UInt:
                    property.propertyType = MaterialPropertyType::UInt;
                    property.u32Value = (u32)defaultProp.defaultFloatValue;
			        break;
		        case ShaderPropertyType::Int:
                    property.propertyType = MaterialPropertyType::Int;
                    property.s32Value = (s32)defaultProp.defaultFloatValue;
			        break;
		        case ShaderPropertyType::Float:
                    property.propertyType = MaterialPropertyType::Float;
                    property.floatValue = defaultProp.defaultFloatValue;
			        break;
		        case ShaderPropertyType::Color:
                    property.propertyType = MaterialPropertyType::Color;
                    property.colorValue = defaultProp.defaultVectorValue;
			        break;
		        case ShaderPropertyType::Vector:
                    property.propertyType = MaterialPropertyType::Vector;
                    property.vectorValue = defaultProp.defaultVectorValue;
			        break;
		        case ShaderPropertyType::Tex2D:
                    property.propertyType = MaterialPropertyType::Texture;
                    property.textureValue.textureName = defaultProp.defaultStringValue;
			        break;
		        case ShaderPropertyType::Tex3D:
                    property.propertyType = MaterialPropertyType::Texture;
                    property.textureValue.textureName = defaultProp.defaultStringValue;
			        break;
		        case ShaderPropertyType::TexCube:
                    property.propertyType = MaterialPropertyType::Texture;
                    property.textureValue.textureName = defaultProp.defaultStringValue;
			        break;
		        case ShaderPropertyType::None:
                    continue;
		        }

                property.name = defaultProp.name;
	            property.displayName = defaultProp.displayName;

                properties.Add(property);
                propertyMap[property.name] = property;
	        }
        }
    }

    void CE::Material::SetCustomPass(u32 passIndex)
    {
        if (this->passIndex == passIndex || material == nullptr)
            return;

        if (material)
        {
            this->passIndex = passIndex;
            material->SetCurrentShaderItem(passIndex);
        }
    }

    RPI::Material* CE::Material::GetRpiMaterial()
    {
        return material;
    }

    void CE::Material::OnAfterDeserialize()
    {
        Super::OnAfterDeserialize();

        valuesModified = true;

        if (shader != nullptr)
        {
            // We need to actually apply the shader if it was already serialized!
            Ref<CE::Shader> resetShader = shader;
            shaderCollection = nullptr;
            this->shader = nullptr;

            SetShader(resetShader.Get());
            ApplyProperties();
        }
        else
        {
            propertyMap.Clear();

            for (const auto& prop : properties)
            {
                propertyMap[prop.name] = prop;
            }
        }
    }

    void CE::Material::ResetAsset()
    {
        Super::ResetAsset();

        AssetManager* assetManager = AssetManager::Get();

        if (assetManager != nullptr)
        {
            shader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        }
    }

    void CE::Material::OnFieldEdited(const Name& fieldName)
    {
        Super::OnFieldChanged(fieldName);

        valuesModified = true;

        ApplyProperties();
    }

    void CE::Material::OnFieldChanged(const Name& fieldName)
    {
        Super::OnFieldChanged(fieldName);

        valuesModified = true;

        ApplyProperties();
    }

    HashMap<Name, MaterialProperty> CE::Material::GetAllProperties()
    {
        return propertyMap;
    }

    void CE::Material::SetProperty(const Name& name, u32 value)
    {
        valuesModified = true;

        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.u32Value = value;
        prop.propertyType = MaterialPropertyType::UInt;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::SetProperty(const Name& name, s32 value)
    {

        valuesModified = true;
        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.s32Value = value;
        prop.propertyType = MaterialPropertyType::Int;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::SetProperty(const Name& name, f32 value)
    {
        valuesModified = true;

        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.floatValue = value;
        prop.propertyType = MaterialPropertyType::Float;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::SetProperty(const Name& name, Vec4 value)
    {
        valuesModified = true;

        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.vectorValue = value;
        prop.propertyType = MaterialPropertyType::Vector;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::SetProperty(const Name& name, Color value)
    {
        valuesModified = true;

        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.colorValue = value;
        prop.propertyType = MaterialPropertyType::Color;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::SetProperty(const Name& name, const Matrix4x4& value)
    {
        valuesModified = true;

        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.matrixValue = value;
        prop.propertyType = MaterialPropertyType::Matrix;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::SetProperty(const Name& name, const Ref<CE::Texture>& value, const Vec2& offset, const Vec2& scaling)
    {
        valuesModified = true;

        if (propertyMap.KeyExists(name))
        {
            properties.RemoveFirst([&](const MaterialProperty& prop) { return prop.name == name; });
        }

        MaterialProperty prop{};
        prop.name = name;
        prop.textureValue.texture = value;
        prop.textureValue.offset = offset;
        prop.textureValue.scaling = scaling;
        prop.propertyType = MaterialPropertyType::Texture;

        properties.Add(prop);
        propertyMap[name] = prop;
    }

    void CE::Material::ApplyProperties()
    {
        if (material == nullptr)
        {
            CE_LOG(Error, All, "SetProperty() called on a material without a shader!");
            return;
        }

        if (!valuesModified)
            return;

        for (const auto& prop : properties)
        {
            switch (prop.propertyType)
            {
            case MaterialPropertyType::UInt:
                material->SetPropertyValue(prop.name, prop.u32Value);
                break;
            case MaterialPropertyType::Int:
                material->SetPropertyValue(prop.name, prop.s32Value);
                break;
            case MaterialPropertyType::Float:
                material->SetPropertyValue(prop.name, prop.floatValue);
                break;
            case MaterialPropertyType::Vector:
                material->SetPropertyValue(prop.name, prop.vectorValue);
                break;
            case MaterialPropertyType::Color:
                material->SetPropertyValue(prop.name, prop.colorValue);
                break;
            case MaterialPropertyType::Matrix:
                material->SetPropertyValue(prop.name, prop.matrixValue);
                break;
            case MaterialPropertyType::Texture:
                if (prop.textureValue.texture != nullptr)
                {
                    material->SetPropertyValue(prop.name, prop.textureValue.texture->GetRpiTexture());
                    material->SetPropertyValue(prop.name.GetString() + "Transform", Vec4(prop.textureValue.offset, prop.textureValue.scaling));
                }
                else if (prop.textureValue.textureName.IsValid())
                {
                    auto& rpiSystem = RPI::RPISystem::Get();
                    RPI::Texture* builtinTexture = rpiSystem.FindBuiltinTexture(prop.textureValue.textureName);

                    if (builtinTexture != nullptr)
                    {
                        material->SetPropertyValue(prop.name, builtinTexture);
                        material->SetPropertyValue(prop.name.GetString() + "Transform", Vec4(prop.textureValue.offset, prop.textureValue.scaling));
                    }
                }
                break;
            case MaterialPropertyType::None:
                break;
            }
        }

        valuesModified = false;
    }
    
} // namespace CE
