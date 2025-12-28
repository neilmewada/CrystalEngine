#pragma once

namespace CE::RPI
{
    class Model;

    CLASS()
    class CORERPI_API ModelAsset : public Object
    {
        CE_CLASS(ModelAsset, Object)
    public:

        virtual ~ModelAsset();

        u32 GetLodCount() const { return lods.GetSize(); }

        Ref<ModelLodAsset> GetModelLod(u32 index) const { return lods[index]; }

        Model* GetModel();

        void AddModelLod(ModelLodAsset* lod);

    private:

        Model* CreateModel();

        Model* model = nullptr;

        FIELD()
        Array<Ref<ModelLodAsset>> lods{};

#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::StaticMeshAssetImportJob;
#endif
    };

} // namespace CE

#include "ModelAsset.rtti.h"
