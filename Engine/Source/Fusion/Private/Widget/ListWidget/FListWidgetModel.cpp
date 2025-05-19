#include "Fusion.h"

namespace CE
{

    FListWidgetModel::FListWidgetModel()
    {

    }

    FListWidgetModel::~FListWidgetModel()
    {
        
    }

    void FListWidgetModel::OnModelPropertyEdited(const CE::Name& propertyName, Object* modifyingObject)
    {
	    Super::OnModelPropertyEdited(propertyName, modifyingObject);

    }

} // namespace CE

