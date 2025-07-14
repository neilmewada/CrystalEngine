#include "CoreRPI.h"

namespace CE::RPI
{

	MeshDrawPacket::~MeshDrawPacket()
	{
		for (int i = 0; i < perDrawSrgs.GetSize(); ++i)
		{
			delete perDrawSrgs[i]; perDrawSrgs[i] = nullptr;
		}
		perDrawSrgs.Clear();

		drawPacket = nullptr;
	}

	MeshDrawPacket::MeshDrawPacket(ModelLod* modelLod, u32 modelLodMeshIndex, RHI::ShaderResourceGroup* objectSrg, RPI::Material* material)
		: modelLod(modelLod), modelLodMeshIndex(modelLodMeshIndex), objectSrg(objectSrg), material(material)
	{
		
	}

	bool MeshDrawPacket::Update(RPI::Scene* scene, bool forceUpdate)
	{
		if (needsUpdate || forceUpdate)
		{
			DoUpdate(scene);
			return true;
		}

		return false;
	}

	void MeshDrawPacket::SetMaterial(RPI::Material* material)
	{
		this->material = material;

		needsUpdate = true;
	}

	MeshDrawPacket::MeshDrawPacket(MeshDrawPacket&& move) noexcept
	{
		Move(move);
	}

	MeshDrawPacket& MeshDrawPacket::operator=(MeshDrawPacket&& move) noexcept
	{
		Move(move);
		return *this;
	}

	MeshDrawPacket::MeshDrawPacket(const MeshDrawPacket& copy)
	{
		CopyFrom(copy);
	}

	MeshDrawPacket& MeshDrawPacket::operator=(const MeshDrawPacket& copy)
	{
		CopyFrom(copy);
		return *this;
	}

	void MeshDrawPacket::Move(MeshDrawPacket& move)
	{
		drawPacket = move.drawPacket;
		modelLod = move.modelLod;
		modelLodMeshIndex = move.modelLodMeshIndex;
		objectSrg = move.objectSrg;
		perDrawSrgs = move.perDrawSrgs;
		material = move.material;
		drawListFilter = move.drawListFilter;
		stencilRef = move.stencilRef;
		needsUpdate = move.needsUpdate;

		move.drawPacket = nullptr;
		move.modelLod = nullptr;
		move.modelLodMeshIndex = 0;
		move.objectSrg = nullptr;
		move.perDrawSrgs = {};
		move.material = nullptr;
		move.drawListFilter = {};
		move.stencilRef = 0;
		move.needsUpdate = false;
	}

	void MeshDrawPacket::CopyFrom(const MeshDrawPacket& from)
	{
		drawPacket = from.drawPacket;
		modelLod = from.modelLod;
		modelLodMeshIndex = from.modelLodMeshIndex;
		objectSrg = from.objectSrg;
		perDrawSrgs = from.perDrawSrgs;
		material = from.material;
		drawListFilter = from.drawListFilter;
		stencilRef = from.stencilRef;
		needsUpdate = from.needsUpdate;
	}

	void MeshDrawPacket::DoUpdate(RPI::Scene* scene)
	{
		needsUpdate = false;

		Mesh* mesh = modelLod->GetMesh(modelLodMeshIndex);

		if (!material)
		{
			//CE_LOG(Error, All, "MeshDrawPacket does not have material set!");
			return;
		}

		drawPacket = nullptr;

		ShaderCollection* shaderCollection = material->GetShaderCollection();
		if (shaderCollection == nullptr)
			return;

		RHI::DrawPacketBuilder builder{};

		builder.SetDrawArguments(mesh->drawArguments);

		material->FlushProperties();

		builder.AddShaderResourceGroup(objectSrg);

		builder.SetDebugName(debugName);

		for (int i = 0; i < perDrawSrgs.GetSize(); ++i)
		{
			delete perDrawSrgs[i]; perDrawSrgs[i] = nullptr;
		}
		perDrawSrgs.Clear();

		auto opaqueTag = RPISystem::Get().GetBuiltinDrawListTag(BuiltinDrawItemTag::Opaque);
		auto transparentTag = RPISystem::Get().GetBuiltinDrawListTag(BuiltinDrawItemTag::Transparent);

		for (int i = 0; i < shaderCollection->GetSize(); i++)
		{
			const ShaderCollection::Item& shaderItem = shaderCollection->At(i);

			RHI::DrawListTag drawListTag = shaderItem.drawListOverride;
			if (!drawListTag.IsValid())
			{
				drawListTag = shaderItem.shader->GetDrawListTag();
			}

			if (!shaderItem.enabled || !drawListTag.IsValid())
			{
				continue;
			}

			RPI::Shader* shader = shaderItem.shader;

			if (shader == nullptr)
			{
				continue;
			}

			// TODO: Implement dynamic shader variant selection based on flags & shader options
			RPI::ShaderVariant* variant = shader->GetVariant(shader->GetDefaultVariantIndex());

			const auto& shaderReflection = variant->GetShaderReflection();

			RHI::DrawPacketBuilder::DrawItemRequest drawItem{};
			drawItem.drawFilterMask = RHI::DrawFilterMask::ALL;
			drawItem.drawItemTag = drawListTag;
			drawItem.stencilRef = stencilRef;

			if (variant->HasSrgLayout(RHI::SRGType::PerDraw))
			{
				const RHI::ShaderResourceGroupLayout& drawSrgLayout = variant->GetSrgLayout(RHI::SRGType::PerDraw);
				RHI::ShaderResourceGroup* perDrawSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(drawSrgLayout);
				perDrawSrgs.Add(perDrawSrg);
				drawItem.uniqueShaderResourceGroups.Add(perDrawSrg);
			}

			if (drawListTag == opaqueTag ||
				drawListTag == transparentTag ||
				material->GetCurrentShader() == shader)
			{
				drawItem.uniqueShaderResourceGroups.Add(material->GetShaderResourceGroup());
			}

			RHI::MultisampleState multisampleState{};
			scene->GetPipelineMultiSampleState(drawListTag, multisampleState);

			// TODO: Get correct pipeline based on MSAA, color formats, etc. from the RPI::Scene's cache
			drawItem.pipelineState = variant->GetPipeline(multisampleState);

			for (const Name& vertexInputName : shaderReflection.vertexInputs)
			{
				RHI::ShaderSemantic semantic = RHI::ShaderSemantic::Parse(vertexInputName.GetString());

				if (semantic.attribute != RHI::VertexInputAttribute::None)
				{
					for (const auto& vertInfo : mesh->vertexBufferInfos)
					{
						if (vertInfo.semantic.attribute != semantic.attribute)
							continue;

						auto vertBufferView = RHI::VertexBufferView(modelLod->GetBuffer(vertInfo.bufferIndex), vertInfo.byteOffset, vertInfo.byteCount, vertInfo.stride);
						drawItem.vertexBufferViews.Add(vertBufferView);
						break;
					}
				}
			}

			drawItem.indexBufferView = mesh->indexBufferView;
			
			builder.AddDrawItem(drawItem);
		}

		drawPacket = builder.Build();
	}

} // namespace CE::RPI
