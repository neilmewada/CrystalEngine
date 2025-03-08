#pragma once

namespace CE::RPI
{
    class Material;
    class Scene;
    class ModelLod;

    //! @brief Maintains the draw packet for a particular mesh and other related data.
    class CORERPI_API MeshDrawPacket
    {
    public:

        MeshDrawPacket() = default;

        ~MeshDrawPacket();

        MeshDrawPacket(ModelLod* modelLod, u32 modelLodMeshIndex, RHI::ShaderResourceGroup* objectSrg, RPI::Material* material);

        MeshDrawPacket(MeshDrawPacket&& move) noexcept;
        MeshDrawPacket& operator=(MeshDrawPacket&& move) noexcept;

        MeshDrawPacket(const MeshDrawPacket& copy);
        MeshDrawPacket& operator=(const MeshDrawPacket& copy);

        bool Update(RPI::Scene* scene, bool forceUpdate = false);

        void SetStencilRef(u8 stencilRef) { this->stencilRef = stencilRef; }

        RHI::DrawPacket* GetDrawPacket() const { return drawPacket; }

        RPI::ModelLod* GetLodModel() const { return modelLod; }

        void SetMaterial(RPI::Material* material);

        RPI::Material* GetMaterial() const { return material; }

    private:

        void Move(MeshDrawPacket& move);

        void CopyFrom(const MeshDrawPacket& from);

        void DoUpdate(RPI::Scene* scene);

        Ptr<RHI::DrawPacket> drawPacket = nullptr;

        RPI::ModelLod* modelLod = nullptr;

        //! @brief Index of this SubMesh in the modelLod
        u32 modelLodMeshIndex = 0;

        RHI::ShaderResourceGroup* objectSrg = nullptr;

        FixedArray<RHI::ShaderResourceGroup*, RHI::DrawPacketBuilder::DrawItemCountMax> perDrawSrgs{};
        
        RPI::Material* material = nullptr;

        RHI::DrawListMask drawListFilter{};

        u8 stencilRef = 0;

        bool needsUpdate = true;

    };

    using MeshDrawPacketList = Array<MeshDrawPacket>;
    using MeshDrawPacketsByLod = FixedArray<MeshDrawPacketList, RPI::Limits::MaxLodCount>;

    using MaterialLodIndex = u32;
    static constexpr MaterialLodIndex MaxMaterialLodIndex = NumericLimits<MaterialLodIndex>::Max();

    using MaterialMeshIndex = u32;
    static constexpr MaterialMeshIndex MaxMaterialSubMeshIndex = NumericLimits<MaterialMeshIndex>::Max();

    using CustomMaterialId = Pair<MaterialLodIndex, MaterialMeshIndex>;
    using CustomMaterialMap = HashMap<CustomMaterialId, RPI::Material*>;

    static constexpr CustomMaterialId DefaultCustomMaterialId = CustomMaterialId(MaxMaterialLodIndex, MaxMaterialSubMeshIndex);
    
} // namespace CE::RPI
