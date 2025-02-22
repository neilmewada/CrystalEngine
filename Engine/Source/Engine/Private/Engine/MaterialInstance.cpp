#include "Engine.h"

namespace CE
{

    MaterialInstance::~MaterialInstance()
    {
        if (materialOverride)
        {
            delete materialOverride;
        }
    }

    void MaterialInstance::SetProperty(const Name& name, u32 value)
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

    void MaterialInstance::SetProperty(const Name& name, s32 value)
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

    void MaterialInstance::SetProperty(const Name& name, f32 value)
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

    void MaterialInstance::SetProperty(const Name& name, Vec4 value)
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

    void MaterialInstance::SetProperty(const Name& name, Color value)
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

    void MaterialInstance::SetProperty(const Name& name, const Matrix4x4& value)
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

    void MaterialInstance::SetProperty(const Name& name, const Ref<CE::Texture>& value, const Vec2& offset, const Vec2& scaling)
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

    RPI::Material* MaterialInstance::GetRpiMaterial()
    {
        return materialOverride;
    }

    void MaterialInstance::OnAfterDeserialize()
    {
        Super::OnAfterDeserialize();

        valuesModified = true;

        propertyMap.Clear();

        for (const auto& prop : properties)
        {
            propertyMap[prop.name] = prop;
        }

        if (materialOverride == nullptr && baseMaterial != nullptr)
        {
            materialOverride = new RPI::Material(baseMaterial->GetRpiMaterial()->GetShaderCollection());
        }
    }

    void MaterialInstance::ApplyProperties()
    {
        if (materialOverride == nullptr && baseMaterial != nullptr)
        {
            materialOverride = new RPI::Material(baseMaterial->GetRpiMaterial()->GetShaderCollection());
        }

        if (materialOverride == nullptr)
        {
            CE_LOG(Error, All, "SetProperty() called on a material without a shader!");
            return;
        }

        if (!valuesModified)
        {
            return;
        }

        valuesModified = false;

        HashMap<Name, MaterialProperty> allProperties = GetAllProperties();

        for (const auto& [name, prop] : allProperties)
        {
            switch (prop.propertyType)
            {
            case MaterialPropertyType::UInt:
                materialOverride->SetPropertyValue(prop.name, prop.u32Value);
                break;
            case MaterialPropertyType::Int:
                materialOverride->SetPropertyValue(prop.name, prop.s32Value);
                break;
            case MaterialPropertyType::Float:
                materialOverride->SetPropertyValue(prop.name, prop.floatValue);
                break;
            case MaterialPropertyType::Vector:
                materialOverride->SetPropertyValue(prop.name, prop.vectorValue);
                break;
            case MaterialPropertyType::Color:
                materialOverride->SetPropertyValue(prop.name, prop.colorValue);
                break;
            case MaterialPropertyType::Matrix:
                materialOverride->SetPropertyValue(prop.name, prop.matrixValue);
                break;
            case MaterialPropertyType::Texture:
                if (prop.textureValue.texture != nullptr)
                {
                    materialOverride->SetPropertyValue(prop.name, prop.textureValue.texture->GetRpiTexture());
                    materialOverride->SetPropertyValue(prop.name.GetString() + "Transform", Vec4(prop.textureValue.offset, prop.textureValue.scaling));
                }
                break;
            }
        }
    }

    HashMap<Name, MaterialProperty> MaterialInstance::GetAllProperties()
    {
        HashMap<Name, MaterialProperty> result{};

        if (baseMaterial != nullptr)
        {
            auto baseProps = baseMaterial->GetAllProperties();
            result.AddRange(baseProps);
        }

        result.AddRange(propertyMap);

        return result;
    }

} // namespace CE
