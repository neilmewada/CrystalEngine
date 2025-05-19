#pragma once

namespace CE
{
    class FListWidget;
    class FListItem;

    CLASS()
    class FUSION_API FListWidgetModel : public FDataModel
    {
        CE_CLASS(FListWidgetModel, FDataModel)
    public:

        FListWidgetModel();

        virtual ~FListWidgetModel();

    protected:

        void OnModelPropertyEdited(const CE::Name& propertyName, Object* modifyingObject) override;

    public:

        MODEL_PROPERTY(Array<FListItem*>, ItemList);

    };
    
} // namespace CE

#include "FListWidgetModel.rtti.h"
