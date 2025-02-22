#pragma once

namespace CE
{

    CLASS(Abstract)
    class ENGINE_API MaterialInstance : public MaterialInterface
    {
        CE_CLASS(MaterialInstance, MaterialInterface)
    public:

        ~MaterialInstance();

        void SetProperty(const Name& name, u32 value) override;

        void SetProperty(const Name& name, s32 value) override;

        void SetProperty(const Name& name, f32 value) override;

        void SetProperty(const Name& name, Vec4 value) override;

        void SetProperty(const Name& name, Color value) override;

        void SetProperty(const Name& name, const Matrix4x4& value) override;

        void SetProperty(const Name& name, const Ref<CE::Texture>& value, const Vec2& offset = Vec2(0, 0), const Vec2& scaling = Vec2(1, 1)) override;

        RPI::Material* GetRpiMaterial() override;

        void OnAfterDeserialize() override;

        void ApplyProperties() override;

    private:

        HashMap<Name, MaterialProperty> GetAllProperties() override;

        FIELD()
        CE::MaterialInterface* baseMaterial = nullptr;

        FIELD()
        Array<MaterialProperty> properties{};

        HashMap<Name, MaterialProperty> propertyMap{};

        RPI::Material* materialOverride = nullptr;

        bool valuesModified = true;
    };
    
} // namespace CE

#include "MaterialInstance.rtti.h"