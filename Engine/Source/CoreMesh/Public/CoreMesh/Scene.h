#pragma once

namespace CE
{
    constexpr u32 MaxUVChannelCount = 4;

    enum class CMSceneLoader
    {
        None = 0,
        FBX
    };
    ENUM_CLASS(CMSceneLoader);

    enum class CMObjectType
    {
        Root = 0,
        Geometry,
        Shape,
        Material,
        Mesh,
        Texture,
        Camera,
        Light,
    };
    ENUM_CLASS(CMObjectType);

    struct CMObject
    {
        Matrix4x4 localTransform{};
        Matrix4x4 globalTransform{};

        Vec3 localTranslation{};
        Vec3 localRotation{};
        Vec3 localScaling{};

        virtual CMObjectType GetObjectType() const { return CMObjectType::Root; }
    };

    enum class CMLightType
    {
        Point = 0,
        Directional,
        Spot,
    };

    enum class CMDecayType
    {
        NoDecay = 0,
        Linear,
        Quadratic,
        Cubic,
        COUNT
    };

    struct CMLight : CMObject
    {
        virtual CMObjectType GetObjectType() const override final { return CMObjectType::Light; }

        CMLightType lightType = CMLightType::Point;
        bool castLight = true;
        Vec4 color = { 1, 1, 1, 1};
        f64 intensity = 0;

        f64 innerAngle = 0.0;
        f64 outerAngle = 45.0;

        f64 fog = 50;

        CMDecayType decayType = CMDecayType::Quadratic;
        f64 decayStart = 1.0;

        bool enableNearAttenuation = false;
        f64 nearAttenuationStart = 0.0;
        f64 nearAttenuationEnd = 0.0;

        bool enableFarAttenuation = false;
        f64 farAttenuationStart = 0.0;
        f64 farAttenuationEnd = 0.0;

        bool castShadows = true;
        Vec4 shadowColor = { 0, 0, 0, 0 };
    };

    struct CMSubMesh
    {
        
    };

    struct CMMaterial
    {
        Vec4 diffuse{};
        Vec4 specular{};
        Vec4 ambient{};
        Vec4 emissive{};
        Vec4 transparent{};
        Vec4 reflective{};

        float reflectivity = 0.0f;
        float shininess = 0.0f;
        float opacity = 1.0f;
        float transparencyFactor = 0.0f;

        float metallicFactor = 0.0f;
        float roughnessFactor = 0.0f;
        float anisotropyFactor = 0.0f;

        String diffuseMap{};
        String normalMap{};
        String ambientOcclussionMap{};
        String emissiveMap{};
        String heightMap{};
        String metallicMap{};
        String roughnessMap{};
    };

    struct CMMesh : CMObject
    {
        Array<Vec3> positions{};
        Array<Vec3> normals{};
        Array<Vec4> colors{};
        Array<Vec3> tangents{};
        StaticArray<Array<Vec2>, MaxUVChannelCount> uvs{};
        Array<u32> indices{};
        u32 materialIndex = 0;
    };

    class COREMESH_API CMScene final
    {
    private:
        CMScene();

    public:

        ~CMScene();

        const Array<CMMesh>& GetMeshes() const { return meshes; }

        const Array<CMMaterial>& GetMaterials() const { return materials; }

    private:

        CMSceneLoader loader = CMSceneLoader::None;

        Array<CMLight> lights{};
        Array<CMMesh> meshes{};
        Array<CMMaterial> materials{};

        friend class FbxImporter;
        friend class ModelImporter;
    };

} // namespace CE
