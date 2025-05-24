#pragma once

namespace CE
{
    
	class CORE_API Asset : public Object
	{
		CE_CLASS(Asset, CE::Object)
	public:

		bool IsAsset() const override final { return true; }

		virtual void ResetAsset() {}

	private:

	};

}

CE_RTTI_CLASS(CORE_API, CE, Asset,
	CE_SUPER(CE::Object),
	CE_ABSTRACT,
	CE_ATTRIBS(Abstract),
	CE_FIELD_LIST(

	),
	CE_FUNCTION_LIST()
)
