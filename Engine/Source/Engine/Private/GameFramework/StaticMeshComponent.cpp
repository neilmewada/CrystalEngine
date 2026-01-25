#include "Engine.h"

namespace CE
{

    StaticMeshComponent::StaticMeshComponent()
    {
        meshChanged = true;
		canTick = true;
    }

    StaticMeshComponent::~StaticMeshComponent()
    {
	    if (meshHandle.IsValid())
	    {
            Ref<CE::Scene> scene = GetScene();
            if (scene != nullptr)
            {
                RPI::Scene* rpiScene = scene->GetRpiScene();
                RPI::StaticMeshFeatureProcessor* fp = rpiScene->GetFeatureProcessor<RPI::StaticMeshFeatureProcessor>();

                fp->ReleaseMesh(meshHandle);
            }
	    }
    }

    u32 StaticMeshComponent::GetLodCount() const
    {
        if (staticMesh)
        {
            return staticMesh->GetLodCount();
        }
	    return Super::GetLodCount();
    }

    u32 StaticMeshComponent::GetLodSubMeshCount(u32 lodIndex)
    {
        if (staticMesh)
        {
            return staticMesh->GetLodSubMeshCount(lodIndex);
        }
	    return Super::GetLodSubMeshCount(lodIndex);
    }

    int StaticMeshComponent::GetLodSubMeshMaterialIndex(u32 lodIndex, u32 subMeshIndex)
    {
        if (staticMesh)
        {
            return staticMesh->GetLodSubMeshMaterialIndex(lodIndex, subMeshIndex);
        }
	    return Super::GetLodSubMeshMaterialIndex(lodIndex, subMeshIndex);
    }

    void StaticMeshComponent::SetStaticMesh(Ref<StaticMesh> staticMesh)
    {
        this->staticMesh = staticMesh;

        if (materialsPerLod.IsEmpty() && staticMesh != nullptr)
        {
            for (int lod = 0; lod < staticMesh->GetLodCount(); ++lod)
            {
                materialsPerLod.Add({});
                LodMaterial& lodMaterial = materialsPerLod.Top();

                for (int i = 0; i < staticMesh->GetBuiltinMaterialCount(); ++i)
                {
                    lodMaterial.materials.Add(staticMesh->GetBuiltinMaterial(i));
                    lodMaterial.materials.GetLast()->MarkDirty();
                    lodMaterial.materials.GetLast()->ApplyProperties();
                }
            }

            SetMaterialDirty(true);
        }

        meshChanged = true;
    }

    void StaticMeshComponent::OnEnabled()
    {
	    Super::OnEnabled();

        if (meshHandle.IsValid())
        {
            meshHandle->flags.visible = true;
        }
    }

    void StaticMeshComponent::OnDisabled()
    {
        Super::OnDisabled();

        if (meshHandle.IsValid())
        {
            meshHandle->flags.visible = false;
        }
    }

    void StaticMeshComponent::Tick(f32 delta)
    {
	    Super::Tick(delta);
        
        Ref<CE::Scene> scene = GetScene();
        if (!scene)
            return;

        RPI::Scene* rpiScene = scene->GetRpiScene();
	    RPI::StaticMeshFeatureProcessor* fp = rpiScene->GetFeatureProcessor<RPI::StaticMeshFeatureProcessor>();
        if (!fp)
            return;

        if (staticMesh == nullptr && meshHandle.IsValid())
        {
            fp->ReleaseMesh(meshHandle);
        }

        if (!staticMesh)
            return;

        RPI::Model* model = staticMesh->GetModelAsset()->GetModel();
        if (!model)
            return;

        if (IsMaterialDirty() && !meshChanged && meshHandle.IsValid())
        {
            SetMaterialDirty(false);

            meshHandle->materialMap = GetRpiMaterialMap();
            meshHandle->materialMap[DefaultCustomMaterialId] = gEngine->GetErrorMaterial()->GetRpiMaterial();
        }

        if (meshChanged)
        {
            if (meshHandle.IsValid())
            {
                fp->ReleaseMesh(meshHandle);
            }

            auto materialMap = GetRpiMaterialMap();
            materialMap[DefaultCustomMaterialId] = gEngine->GetErrorMaterial()->GetRpiMaterial();

            RPI::ModelHandleDescriptor descriptor{};
            descriptor.debugName = staticMesh->GetName();
            descriptor.model = model;
            descriptor.originalModel = staticMesh->GetModelAsset().Get();

            meshHandle = fp->AcquireMesh(descriptor, materialMap);
        }

        if (meshHandle.IsValid())
        {
            meshHandle->localToWorldTransform = GetTransform();
        }

        meshChanged = false;
    }

} // namespace CE
