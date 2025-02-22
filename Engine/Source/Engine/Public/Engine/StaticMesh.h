#pragma once

namespace CE
{

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

    private:

        FIELD()
        Ref<RPI::ModelAsset> modelAsset = nullptr;
        
#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::StaticMeshAssetImportJob;
#endif
    };
    
} // namespace CE

#include "StaticMesh.rtti.h"