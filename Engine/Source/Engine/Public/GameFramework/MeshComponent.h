#pragma once

namespace CE
{
	class MaterialInterface;

	STRUCT()
    struct ENGINE_API LodMaterial final
    {
        CE_STRUCT(LodMaterial)
    public:

        //! @brief Materials per sub-mesh.
        FIELD(EditAnywhere)
        Array<Ref<MaterialInterface>> materials{};
    };

	CLASS(Abstract)
	class ENGINE_API MeshComponent : public GeometryComponent
	{
		CE_CLASS(MeshComponent, GeometryComponent)
	public:

		MeshComponent();
		virtual ~MeshComponent();

		virtual u32 GetLodCount() const { return 1; }

		virtual u32 GetLodSubMeshCount(u32 lodIndex) { return 1; }

		RPI::CustomMaterialMap GetRpiMaterialMap();

		Ref<MaterialInterface> GetMaterial(u32 subMeshIndex = 0);

		void SetMaterial(MaterialInterface* material, u32 subMeshIndex = 0);

		Ref<MaterialInterface> GetMaterial(u32 lodIndex, u32 subMeshIndex);

		void SetMaterial(MaterialInterface* material, u32 lodIndex, u32 subMeshIndex);

		bool IsMaterialDirty() const { return materialDirty; }

		void SetMaterialDirty(bool set = true) { materialDirty = set; }

	protected:

		void OnFieldEdited(const Name& fieldName) override;

	protected:

		//! @brief Materials per LOD mesh
		FIELD(EditAnywhere, Category = "Materials", ArrayEditorMode = "Static")
        Array<LodMaterial> materialsPerLod{};


	private:

		b8 materialDirty = false;
	};
    
} // namespace CE

#include "MeshComponent.rtti.h"
