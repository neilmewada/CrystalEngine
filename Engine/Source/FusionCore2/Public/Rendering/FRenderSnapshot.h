#pragma once

namespace CE
{

	class FUSIONCORE_API FRenderSnapshot : public IntrusiveBase
	{
	public:

	private:

		using FSplitPointArray = StableDynamicArray<u32, 128, false>;

		FUIVertexArray vertexArray;
		FSplitPointArray vertexSplits;

		FUIIndexArray indexArray;
		FSplitPointArray indexSplits;

		FUIDrawItemArray drawItemArray;
		FSplitPointArray drawItemSplits;

		FUIDrawCmdArray drawCmdArray;
		FSplitPointArray drawCmdSplits;

	};

}
