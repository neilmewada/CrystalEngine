#pragma once

namespace CE::RPI
{
    
	class CORERPI_API Model final
	{
	public:

		Model();
		~Model();

		inline u32 GetModelLodCount() const { return lods.GetSize(); }

		inline ModelLod* GetModelLod(u32 lod) { return lods[lod]; }

	private:

		Array<Ptr<ModelLod>> lods{};

		friend class ModelAsset;

#if PAL_TRAIT_BUILD_EDITOR
		friend class CE::Editor::StaticMeshAssetImportJob;
#endif
	};

} // namespace CE::RPI
