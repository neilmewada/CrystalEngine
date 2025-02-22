#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API Material : public MaterialInterface
    {
        CE_CLASS(Material, MaterialInterface)
    public:

        Material();
        ~Material();

        void SetShader(CE::Shader* shader);

        void SetProperty(const Name& name, u32 value) override;

        void SetProperty(const Name& name, s32 value) override;

        void SetProperty(const Name& name, f32 value) override;

        void SetProperty(const Name& name, Vec4 value) override;

        void SetProperty(const Name& name, Color value) override;

        void SetProperty(const Name& name, const Matrix4x4& value) override;

        void SetProperty(const Name& name, const Ref<CE::Texture>& value, const Vec2& offset = Vec2(0, 0), const Vec2& scaling = Vec2(1, 1)) override;

        void ApplyProperties() override;

        void SetCustomPass(u32 passIndex) override;

        RPI::Material* GetRpiMaterial() override;

        void OnAfterDeserialize() override;

        const Ref<CE::Shader>& GetShader() override
        {
            return shader;
        }

    private:

        HashMap<Name, MaterialProperty> GetAllProperties() override;

        RPI::Material* material = nullptr;

        FIELD()
        Ref<CE::Shader> shader = nullptr;

        FIELD()
        Array<MaterialProperty> properties{};

        bool valuesModified = true;
        ShaderCollection* shaderCollection = nullptr;

        HashMap<Name, MaterialProperty> propertyMap{};

        friend class MaterialInstance;
    };

} // namespace CE

#include "Material.rtti.h"
