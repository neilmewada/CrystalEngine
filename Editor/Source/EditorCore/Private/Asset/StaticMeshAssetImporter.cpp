#include "EditorCore.h"

namespace CE::Editor
{

    Array<AssetImportJob*> StaticMeshAssetImporter::CreateImportJobs(const Array<IO::Path>& sourceAssets, const Array<IO::Path>& productAssets)
    {
        Array<AssetImportJob*> jobs{};

		for (int i = 0; i < sourceAssets.GetSize(); i++)
		{
			auto job = new StaticMeshAssetImportJob(this, sourceAssets[i], productAssets[i]);
			job->optimizeMeshes = optimizeMeshes;
			job->generateUV1 = generateUV1;

			jobs.Add(job);
		}

        return jobs;
    }

    bool StaticMeshAssetImportJob::ProcessAsset(const Ref<Bundle>& bundle)
    {
		if (bundle == nullptr)
			return false;
		if (!sourcePath.Exists())
			return false;

		Uuid meshUuid = Uuid::Random();
		Uuid modelAssetUuid = Uuid::Random();
		Uuid lodAssetUuid = Uuid::Random();

		for (int i = 0; i < bundle->GetSubObjectCount(); ++i)
		{
			Ref<Object> object = bundle->GetSubObject(i);

			if (object->IsOfType<StaticMesh>())
			{
				Ref<StaticMesh> staticMesh = static_cast<Ref<StaticMesh>>(object);
				meshUuid = staticMesh->GetUuid();

				if (staticMesh->GetSubObjectCount() >= 1 &&
					staticMesh->GetSubObject(0)->IsOfType<RPI::ModelAsset>())
				{
					Ref<RPI::ModelAsset> modelAsset = static_cast<RPI::ModelAsset*>(staticMesh->GetSubObject(0));

					modelAssetUuid = modelAsset->GetUuid();

					if (modelAsset->GetSubObjectCount() >= 1 &&
						modelAsset->GetSubObject(0)->IsOfType<RPI::ModelLodAsset>())
					{
						lodAssetUuid = modelAsset->GetSubObject(0)->GetUuid();
					}
				}

				break;
			}
		}

		// Clear the bundle of any subobjects/assets, we will build the asset from scratch
		bundle->DestroyAllSubObjects();

		String extension = sourcePath.GetFileName().GetExtension().GetString().ToLower();
		String fileName = sourcePath.GetFileName().RemoveExtension().GetString();
		// Make sure we can use the fileName as name of an object
		fileName = FixObjectName(fileName);

		FileStream fileStream = FileStream(sourcePath, Stream::Permissions::ReadOnly);
		fileStream.SetBinaryMode(true);
		u8* data = (u8*)malloc(fileStream.GetLength());
		fileStream.Read(data, fileStream.GetLength());
    	defer(&)
    	{
    		free(data);
    	};

		ModelImporter importer{};
		ModelLoadConfig config{};
		config.postProcessFlags |= ModelPostProcessFlags::GenNormals | ModelPostProcessFlags::CalcTangentSpace;
		if (optimizeMeshes)
		{
			config.postProcessFlags |= ModelPostProcessFlags::OptimizeMeshes | ModelPostProcessFlags::OptimizeGraph;
		}

		if (extension == ".fbx")
		{
			config.fileFormat = ModelFileFormat::FBX;
		}
		else
		{
			errorMessage = "Invalid source extension: " + extension;
			return false;
		}

		CMScene* scene = importer.ImportScene(data, fileStream.GetLength(), config);
		if (scene == nullptr)
		{
			errorMessage = importer.GetErrorMessage();
			return false;
		}

    	defer(&)
		{
			delete scene;
		};

		Ref<StaticMesh> staticMesh = CreateObject<StaticMesh>(bundle.Get(), fileName,
			OF_NoFlags, StaticMesh::StaticClass(), nullptr,
			meshUuid);

		Ref<RPI::ModelAsset> modelAsset = CreateObject<RPI::ModelAsset>(staticMesh.Get(), "ModelAsset",
			OF_NoFlags, RPI::ModelAsset::StaticClass(), nullptr,
			modelAssetUuid);
		staticMesh->modelAsset = modelAsset;

		Ref<RPI::ModelLodAsset> lod = CreateObject<RPI::ModelLodAsset>(modelAsset.Get(), "Lod0", 
			OF_NoFlags, RPI::ModelLodAsset::StaticClass(), nullptr,
			lodAssetUuid);
		modelAsset->lods.Add(lod);
		
		const Array<CMMesh>& meshes = scene->GetMeshes();

		u64 totalVertices = 0;
		u64 totalIndices = 0;
		u64 totalColorCount = 0;
		u64 totalUVCounts[4] = { 0, 0, 0, 0 };
		for (int i = 0; i < meshes.GetSize(); i++)
		{
			totalVertices += meshes[i].positions.GetSize();
			totalIndices += meshes[i].indices.GetSize();
			totalColorCount += meshes[i].colors.GetSize();

			for (int j = 0; j < 4; j++)
			{
				totalUVCounts[j] += meshes[i].uvs[j].GetSize();
			}
		}

		RHI::IndexFormat indexFormat = RHI::IndexFormat::Uint32;
		u32 bytesPerIndex = 4;
		if (totalVertices < NumericLimits<u16>::Max())
		{
			indexFormat = RHI::IndexFormat::Uint16;
			bytesPerIndex = 2;
		}

		lod->numVertices = totalVertices;

		lod->positionsData.Reserve(totalVertices * sizeof(Vec4));
		lod->normalData.Reserve(totalVertices * sizeof(Vec4));
		lod->tangentData.Reserve(totalVertices * sizeof(Vec4));
		if (totalColorCount > 0)
		{
			lod->color0Data.Reserve(totalVertices * sizeof(Color));
		}
		
		if (totalUVCounts[0] > 0)
			lod->uv0Data.Reserve(totalVertices * sizeof(Vec2));
		if (totalUVCounts[1] > 0)
			lod->uv1Data.Reserve(totalVertices * sizeof(Vec2));
		if (totalUVCounts[2] > 0)
			lod->uv2Data.Reserve(totalVertices * sizeof(Vec2));
		if (totalUVCounts[3] > 0)
			lod->uv3Data.Reserve(totalVertices * sizeof(Vec2));

		u32 indexStartValueOffset = 0;
		u64 vec4Offset = 0;
		u64 vec2Offset = 0;
		
		lod->subMeshes.Resize(meshes.GetSize());

		for (int i = 0; i < meshes.GetSize(); i++)
		{
			const CMMesh& mesh = meshes[i];
			u32 numVertices = mesh.positions.GetSize();
			u32 numIndices = mesh.indices.GetSize();

			u8* positionsPtr = lod->positionsData.GetDataPtr();
			u8* normalsPtr = lod->normalData.GetDataPtr();
			u8* tangentsPtr = lod->tangentData.GetDataPtr();
			u8* colors0Ptr = lod->color0Data.GetDataPtr();
			u8* uv0Ptr = lod->uv0Data.GetDataPtr();

			memcpy(positionsPtr + vec4Offset, mesh.positions.GetData(), numVertices * sizeof(Vec4));
			memcpy(normalsPtr + vec4Offset, mesh.normals.GetData(), numVertices * sizeof(Vec4));
			memcpy(tangentsPtr + vec4Offset, mesh.tangents.GetData(), numVertices * sizeof(Vec4));

			if (colors0Ptr != nullptr && totalColorCount > 0)
			{
				if (mesh.colors.NotEmpty())
					memcpy(colors0Ptr + vec4Offset, mesh.colors.GetData(), numVertices * sizeof(Vec4));
				else
					memset(colors0Ptr + vec4Offset, 0, numVertices * sizeof(Vec4));
			}

			if (uv0Ptr != nullptr && totalUVCounts[0] > 0)
			{
				if (mesh.uvs[0].NotEmpty())
					memcpy(uv0Ptr + vec2Offset, mesh.uvs[0].GetData(), numVertices * sizeof(Vec2));
				else
					memset(uv0Ptr + vec2Offset, 0, numVertices * sizeof(Vec2));
			}

			vec4Offset += numVertices * sizeof(Vec4);
			vec2Offset += numVertices * sizeof(Vec2);
			
			RPI::ModelLodSubMesh& subMesh = lod->subMeshes[i];

			subMesh.indexFormat = indexFormat;
			subMesh.numIndices = numIndices;
			subMesh.indicesData.Reserve(numIndices * bytesPerIndex);

			for (u32 i = 0; i < numIndices; i++)
			{
				if (indexFormat == RHI::IndexFormat::Uint16)
				{
					*((u16*)subMesh.indicesData.GetDataPtr() + i) = indexStartValueOffset + mesh.indices[i];
				}
				else // Uint32
				{
					*((u32*)subMesh.indicesData.GetDataPtr() + i) = indexStartValueOffset + mesh.indices[i];
				}
			}

			indexStartValueOffset += numVertices;

			subMesh.materialIndex = mesh.materialIndex;
		}

        return true;
    }

} // namespace CE::Editor
