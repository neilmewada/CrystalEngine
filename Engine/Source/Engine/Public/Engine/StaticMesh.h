#pragma once

namespace CE
{
    class Material;

    CLASS()
    class ENGINE_API StaticMesh : public Asset
    {
        CE_CLASS(StaticMesh, Asset)
    public:

        StaticMesh();
        ~StaticMesh();

        Ref<RPI::ModelAsset> GetModelAsset() const
        {
	        return modelAsset;
        }

        void SetModelAsset(RPI::ModelAsset* modelAsset)
        {
            this->modelAsset = modelAsset;
        }

        u32 GetLodCount() const
        {
            if (!modelAsset)
                return 0;
	        return modelAsset->GetLodCount();
        }

        u32 GetLodSubMeshCount(u32 lodIndex) const
        {
            if (!modelAsset || lodIndex >= modelAsset->GetLodCount())
                return 0;
            Ref<ModelLodAsset> lod = modelAsset->GetModelLod(lodIndex);
            if (lod.IsNull())
                return 0;
            return lod->GetSubMeshCount();
        }

        u32 GetLodSubMeshMaterialIndex(u32 lodIndex, u32 subMeshIndex)
        {
            if (!modelAsset || lodIndex >= modelAsset->GetLodCount())
                return -1;
            Ref<ModelLodAsset> lod = modelAsset->GetModelLod(lodIndex);
            if (lod.IsNull())
                return -1;
            if (subMeshIndex < 0 || subMeshIndex >= lod->GetSubMeshCount())
                return -1;
            return lod->GetSubMesh(subMeshIndex).materialIndex;
        }

        int GetBuiltinMaterialCount() const { return builtinMaterials.GetSize(); }

        Ref<CE::Material> GetBuiltinMaterial(u32 index) const
        {
            if (index >= builtinMaterials.GetSize())
                return nullptr;

            return builtinMaterials[index];
        }

    private:

        FIELD()
        Ref<RPI::ModelAsset> modelAsset = nullptr;

        FIELD()
        Array<Ref<CE::Material>> builtinMaterials;
        
#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::StaticMeshAssetImportJob;
#endif
    };
    
} // namespace CE

#include "StaticMesh.rtti.h"