#pragma once

namespace CE
{

	class FUSIONCORE_API FRenderSnapshot : public IntrusiveBase
	{
	public:

	private:

		FUIVertexArray vertexArray;
		FUIIndexArray indexArray;
		FUIDrawItemArray drawItems;
		FUIDrawCmdArray drawCmdArray;

	};

}
