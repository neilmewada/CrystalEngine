#pragma once

#if PAL_TRAIT_BUILD_EDITOR
namespace CE::Editor
{
	class StaticMeshAssetImportJob;
}
#endif

namespace CE::RPI
{
	using MaterialSlotId = u32;

	class Material;

	struct VertexBufferInfo
	{
		RHI::ShaderSemantic semantic{};
		u32 bufferIndex = 0;
		u32 byteOffset = 0;
		u32 byteCount = 0;
		u32 stride = 0;
		RHI::VertexAttributeDataType attributeType{};

		inline bool IsValid() const
		{
			return semantic.attribute != RHI::VertexInputAttribute::None;
		}
	};

	using VertexBufferList = FixedArray<VertexBufferInfo, RHI::Limits::Pipeline::MaxVertexAttribCount>;

	struct Mesh final
	{
		RHI::DrawArguments drawArguments{};
		RHI::IndexBufferView indexBufferView{};

		VertexBufferList vertexBufferInfos{};

		//! @brief Index of the material to use for this sub-mesh
		MaterialSlotId materialSlotId = 0;
	};

	class CORERPI_API ModelLod final
	{
	public:

		ModelLod();
		~ModelLod();

		static ModelLod* CreateCubeModel();

		static ModelLod* CreateSphereModel();

		void AddMesh(const Mesh& mesh);

		//! @brief Returns the number of sub meshes 
		u32 GetMeshCount() const { return meshes.GetSize(); }

		Mesh* GetMesh(u32 index)
		{
			if (index >= meshes.GetSize())
				return nullptr;
			return &meshes[index]; 
		}

		RHI::Buffer* GetBuffer(u32 index) const { return trackedBuffers[index]; }

		//! @brief Always add index buffers at the very end!
		void TrackBuffer(RHI::Buffer* buffer);

		//! Temporary function
		void BuildVertexInputSlotDescriptorList(u32 meshIndex, Array<RHI::VertexInputSlotDescriptor>& outInputSlots);

		//! Temporary function
		void BuildVertexInputAttributeList(u32 meshIndex, Array<RHI::VertexAttributeDescriptor>& outVertexAttribs);

		RHI::RayTracingBlas* GetBlas() const { return blas; }

	private:

		void OnPostProcess();

		RHI::RayTracingBlas* blas = nullptr;

		// Each model can have multiple meshes (aka SubMeshes)
		Array<Mesh> meshes{};

		Array<RHI::Buffer*> trackedBuffers{};

		u32 totalVertexBuffers = 0;
		FixedArray<u8*, RHI::Limits::Pipeline::MaxVertexAttribCount> vertexDatas{};

		friend class ModelLodAsset;

#if PAL_TRAIT_BUILD_EDITOR
		friend class CE::Editor::StaticMeshAssetImportJob;
#endif
	};
    
} // namespace CE::RPI

