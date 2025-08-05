#pragma once

namespace CE
{

    class CORE_API DataAsset : public Asset
    {
        CE_CLASS(DataAsset, Asset)
    protected:

        DataAsset();

    public:
        
    };
    
} // namespace CE

CE_RTTI_CLASS(CORE_API, CE, DataAsset,
    CE_SUPER(CE::Asset),
    CE_ABSTRACT,
    CE_ATTRIBS(Abstract),
    CE_FIELD_LIST(

    ),
    CE_FUNCTION_LIST()
)
