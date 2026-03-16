#pragma once

namespace CE
{
	class FLayer;

	class FUSIONCORE_API FRenderSnapshot : public IntrusiveBase
	{
	public:

		FRenderSnapshot(RHI::DrawListTag drawListTag);

		void Clear();

		void BuildSnapshot(Ref<FLayer> layer);

		void FlushDrawPackets(u32 frameIndex);

	private:

		void DoLayer(Ref<FLayer> layer, int layerIndex);

		struct FSplitRange
		{
			SIZE_T startIndex = 0;
			SIZE_T count = 0;
		};

		struct FRenderPass
		{
			AttachmentID renderTarget = nullptr;

			SIZE_T layerIndex = 0;
			SIZE_T drawCmdStartIndex = 0;
			SIZE_T drawCmdCount = 0;
		};

		using FSplitRangeArray = StableDynamicArray<FSplitRange, 64, false>;
		using FRenderPassArray = StableDynamicArray<FRenderPass, 32, false>;

		FUIVertexArray vertexArray;
		FSplitRangeArray vertexSplits;

		FUIIndexArray indexArray;
		FSplitRangeArray indexSplits;

		FUIDrawItemArray drawItemArray;
		FSplitRangeArray drawItemSplits;

		FUIDrawCmdArray drawCmdArray;
		FSplitRangeArray drawCmdSplits;

		FRenderPassArray renderPassArray;

		RHI::DrawListTag drawListTag = RHI::DrawListTag::NullValue;

		friend class FSurface;
		friend class FNativeSurface;
	};

}
