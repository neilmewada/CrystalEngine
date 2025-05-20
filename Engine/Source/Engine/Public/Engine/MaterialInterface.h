#pragma once

namespace CE
{
    class Texture;
    class Material;
    class MaterialInstance;

    ENUM()
    enum class MaterialPropertyType
    {
        None = 0,
        UInt, Int,
        Float,
        Vector,
        Color,
        Matrix,
        Texture
    };
    ENUM_CLASS(MaterialPropertyType);

    STRUCT()
    struct ENGINE_API MaterialTextureValue
    {
        CE_STRUCT(MaterialTextureValue)
    public:

        MaterialTextureValue() = default;

        MaterialTextureValue(const Name& textureName) : textureName(textureName)
        {}

        MaterialTextureValue(CE::Texture* texture, Vec2 offset = Vec2(0, 0), Vec2 scaling = Vec2(1, 1))
	        : texture(texture), offset(offset), scaling(scaling)
        {}

        FIELD()
        Name textureName = "";

        FIELD(EditAnywhere)
        WeakRef<CE::Texture> texture = nullptr;

        FIELD(EditAnywhere)
        Vec2 offset = Vec2(0, 0);

        FIELD(EditAnywhere)
        Vec2 scaling = Vec2(1, 1);
    };

    STRUCT()
    struct ENGINE_API MaterialProperty
    {
        CE_STRUCT(MaterialProperty)
    public:

        FIELD()
        Name name{};

        FIELD()
        Name displayName{};

        FIELD()
        MaterialPropertyType propertyType = MaterialPropertyType::None;

        FIELD(EditAnywhere, DisplayName = "UInt Value", ShowIf = ".propertyType == UInt")
        u32 u32Value = 0;

        FIELD(EditAnywhere, DisplayName = "Int Value", ShowIf = ".propertyType == Int")
        s32 s32Value = 0;

        FIELD(EditAnywhere, ShowIf = ".propertyType == Float")
        f32 floatValue = 0;

        FIELD(EditAnywhere, ShowIf = ".propertyType == Vector")
        Vec4 vectorValue = {};

        FIELD(EditAnywhere, ShowIf = ".propertyType == Color")
        Color colorValue = {};

        FIELD()
        Matrix4x4 matrixValue = {};

        FIELD(EditAnywhere, ShowIf = ".propertyType == Texture", StructTypeName = "")
        MaterialTextureValue textureValue = {};
    };

    CLASS(Abstract)
    class ENGINE_API MaterialInterface : public Asset
    {
        CE_CLASS(MaterialInterface, Asset)
    public:

        MaterialInterface();
        virtual ~MaterialInterface();

        virtual void SetProperty(const Name& name, u32 value) = 0;

        virtual void SetProperty(const Name& name, s32 value) = 0;

        virtual void SetProperty(const Name& name, f32 value) = 0;

        virtual void SetProperty(const Name& name, Vec4 value) = 0;

        virtual void SetProperty(const Name& name, Color value) = 0;

        virtual void SetProperty(const Name& name, const Matrix4x4& value) = 0;

        virtual void SetProperty(const Name& name, const Ref<CE::Texture>& value, const Vec2& offset = Vec2(0, 0), const Vec2& scaling = Vec2(1, 1)) = 0;

        virtual void ApplyProperties() = 0;

        virtual void SetCustomPass(u32 passIndex) = 0;

        virtual RPI::Material* GetRpiMaterial() = 0;

        virtual const Ref<CE::Shader>& GetShader() = 0;

    protected:

        virtual HashMap<Name, MaterialProperty> GetAllProperties() = 0;

        int passIndex = -1;

        friend class CE::Material;
        friend class CE::MaterialInstance;
    };

} // namespace CE

#include "MaterialInterface.rtti.h"
