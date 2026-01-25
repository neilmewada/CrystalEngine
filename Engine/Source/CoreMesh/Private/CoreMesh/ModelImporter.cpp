#include "CoreMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace CE
{
	ModelImporter::ModelImporter()
	{
		
	}

	ModelImporter::~ModelImporter()
	{

	}

	inline Vec3 ToVec3(aiVector3D v)
	{
		return Vec3(v.x, v.y, v.z);
	}

	inline Vec2 ToVec2(aiVector2D v)
	{
		return Vec2(v.x, v.y);
	}

	inline Vec4 ToVec4(aiColor4D c)
	{
		return Vec4(c.r, c.g, c.b, c.a);
	}

	inline Vec4 ToVec4(aiColor3D c)
	{
		return Vec4(c.r, c.g, c.b, 1.0f);
	}

	CMScene* ModelImporter::ImportScene(u8* data, u64 dataSize, const ModelLoadConfig& config)
	{
		using namespace Assimp;

		auto postProcessFlags = (aiPostProcessSteps)config.postProcessFlags
			| aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
		
		Assimp::Importer importer{};

		const char* hint = nullptr;
		if (config.fileFormat == ModelFileFormat::FBX)
		{
			hint = ".fbx";
		}

		const aiScene* scene = importer.ReadFileFromMemory(data, dataSize, postProcessFlags, hint);
		if (scene == nullptr)
		{
			error = ERR_AssImpFailed;
			errorMessage = importer.GetErrorString();
			return nullptr;
		}

		CMScene* outScene = new CMScene();
		
		int numMeshes = scene->mNumMeshes;

		if (scene->HasMaterials())
		{
			for (int i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* material = scene->mMaterials[i];

				outScene->materials.Add({});
				CMMaterial& outMat = outScene->materials[i];

				aiColor3D color(0.f, 0.f, 0.f);
				material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				outMat.diffuse = ToVec4(color);

				color = aiColor3D(0.f, 0.f, 0.f);
				material->Get(AI_MATKEY_COLOR_SPECULAR, color);
				outMat.specular = ToVec4(color);

				color = aiColor3D(0.f, 0.f, 0.f);
				material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
				outMat.emissive = ToVec4(color);

				color = aiColor3D(0.f, 0.f, 0.f);
				material->Get(AI_MATKEY_COLOR_AMBIENT, color);
				outMat.ambient = ToVec4(color);

				color = aiColor3D(0.f, 0.f, 0.f);
				material->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
				outMat.transparent = ToVec4(color);

				color = aiColor3D(0.f, 0.f, 0.f);
				material->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
				outMat.reflective = ToVec4(color);

				float floatValue = 0;
				material->Get(AI_MATKEY_REFLECTIVITY, floatValue);
				outMat.reflectivity = floatValue;

				floatValue = 0;
				material->Get(AI_MATKEY_SHININESS, floatValue);
				outMat.shininess = floatValue;

				floatValue = 1;
				material->Get(AI_MATKEY_OPACITY, floatValue);
				outMat.opacity = floatValue;

				floatValue = 1;
				material->Get(AI_MATKEY_METALLIC_FACTOR, floatValue);
				outMat.metallicFactor = Math::Clamp01(floatValue);

				floatValue = 1;
				material->Get(AI_MATKEY_ROUGHNESS_FACTOR, floatValue);
				outMat.roughnessFactor = 1.0f; // Note: There's some issue with roughness from assimp. It returns a negative number for sponza scene.

				floatValue = 0;
				material->Get(AI_MATKEY_ANISOTROPY_FACTOR, floatValue);
				outMat.anisotropyFactor = floatValue;

				aiString map{};
				material->GetTexture(aiTextureType_DIFFUSE, 0, &map);
				outMat.diffuseMap = map.C_Str();

				map = "";
				material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &map);
				outMat.ambientOcclussionMap = map.C_Str();

				map = "";
				material->GetTexture(aiTextureType_EMISSIVE, 0, &map);
				outMat.emissiveMap = map.C_Str();

				map = "";
				int count = material->GetTextureCount(aiTextureType_NORMALS);
				material->GetTexture(aiTextureType_NORMALS, 0, &map);
				outMat.normalMap = map.C_Str();

				map = "";
				material->GetTexture(aiTextureType_HEIGHT, 0, &map);
				outMat.heightMap = map.C_Str();

				map = "";
				material->GetTexture(aiTextureType_METALNESS, 0, &map);
				outMat.metallicMap = map.C_Str();

				map = "";
				material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &map);
				outMat.roughnessMap = map.C_Str();
			}
		}

		for (int i = 0; i < numMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];

			outScene->meshes.Add({});

			CMMesh& outMesh = outScene->meshes.Top();

			for (u32 j = 0; j < mesh->mNumVertices; j++)
			{
				Vec3 pos = ToVec3(mesh->mVertices[j]);
				outMesh.positions.Add(pos);

				if (mesh->HasNormals())
				{
					outMesh.normals.Add(ToVec3(mesh->mNormals[j]));
				}

				if (mesh->HasTangentsAndBitangents())
				{
					outMesh.tangents.Add(ToVec3(mesh->mTangents[j]));
				}

				if (mesh->HasVertexColors(0))
				{
					outMesh.colors.Add(ToVec4(mesh->mColors[0][j]));
				}

				if (mesh->HasTextureCoords(0))
				{
					outMesh.uvs[0].Add(ToVec3(mesh->mTextureCoords[0][j]));
				}

				if (mesh->HasTextureCoords(1))
				{
					outMesh.uvs[1].Add(ToVec3(mesh->mTextureCoords[1][j]));
				}
			}

			outMesh.materialIndex = mesh->mMaterialIndex;
			
			for (u32 j = 0; j < mesh->mNumFaces; j++)
			{
				u32 numIndices = mesh->mFaces[j].mNumIndices;
				u32* indices = mesh->mFaces[j].mIndices;

				for (u32 k = 0; k < numIndices; k++)
				{
					outMesh.indices.Add(indices[k]);
				}
			}
		}

		importer.FreeScene();

		return outScene;
	}

} // namespace CE
