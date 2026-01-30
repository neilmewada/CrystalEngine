#include "CoreRPI.h"

namespace CE::RPI
{
	View::View()
	{
		
	}

	View::~View()
	{
		Free();
	}

	void View::Free()
	{
		for (int i = 0; i < viewConstantBuffers.GetSize(); ++i)
		{
			delete viewConstantBuffers[i]; viewConstantBuffers[i] = nullptr;
		}

		delete shaderResourceGroup; shaderResourceGroup = nullptr;
	}

	View* View::CreateView(const Name& name, UsageFlags usageFlags)
	{
		View* view = new View();
		view->name = name;
		view->usageFlags = usageFlags;

		if (gDynamicRHI != nullptr)
		{
			RHI::ShaderResourceGroupDescriptor viewSrgDesc{};
			viewSrgDesc.name = "SRG_PerView";
			viewSrgDesc.layout = RPISystem::Get().GetViewSrgLayout();

			view->shaderResourceGroup = gDynamicRHI->CreateShaderResourceGroup(viewSrgDesc);

			RHI::BufferDescriptor bufferDescriptor{};
			bufferDescriptor.name = "ViewConstants";
			bufferDescriptor.bindFlags = BufferBindFlags::ConstantBuffer;
			bufferDescriptor.defaultHeapType = MemoryHeapType::Upload;
			bufferDescriptor.bufferSize = sizeof(viewConstants);
			bufferDescriptor.structureByteStride = sizeof(viewConstants);

			for (int i = 0; i < view->viewConstantBuffers.GetSize(); ++i)
			{
				view->viewConstantBuffers[i] = gDynamicRHI->CreateBuffer(bufferDescriptor);

				view->shaderResourceGroup->Bind("_PerViewData", view->viewConstantBuffers[i]);
			}

			view->shaderResourceGroup->FlushBindings();
		}

		return view;
	}

	void View::SetDrawListMask(const RHI::DrawListMask& mask)
	{
		if (drawListMask != mask)
		{
			drawListMask = mask;
			drawListContext.Shutdown();
			drawListContext.Init(drawListMask);
		}
	}

	void View::SetShaderResourceGroup(RHI::ShaderResourceGroup* viewSrg)
	{
		shaderResourceGroup = viewSrg;
	}
	
	RHI::DrawList& View::GetDrawList(RHI::DrawListTag drawItemTag)
	{
		return drawListContext.GetDrawListForTag(drawItemTag);
	}

	void View::Reset()
	{
		drawListMask.Reset();
		drawListContext.Shutdown();
	}

	void View::Init(RHI::DrawListMask drawListMask)
	{
		this->drawListMask = drawListMask;
		drawListContext.Init(drawListMask);
	}

	void View::UpdateSrg(int imageIndex)
	{
		void* data;
		viewConstantBuffers[imageIndex]->Map(0, viewConstantBuffers[imageIndex]->GetBufferSize(), &data);
		{
			memcpy(data, &viewConstants, sizeof(viewConstants));
		}
		viewConstantBuffers[imageIndex]->Unmap();
	}

	void View::AddDrawPacket(DrawPacket* drawPacket, f32 depth)
	{
		drawListContext.AddDrawPacket(drawPacket, depth);
	}

} // namespace CE::RPI
