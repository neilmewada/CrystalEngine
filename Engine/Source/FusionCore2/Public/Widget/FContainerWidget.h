#pragma once

namespace CE
{
    CLASS(Abstract)
    class FUSIONCORE_API FContainerWidget : public FWidget
    {
        CE_CLASS(FContainerWidget, FWidget)
    protected:

        FContainerWidget();

    public: // - Public API -

        void SetParentSurfaceRecursive(Ref<FSurface> surface) override;

        void AddChildWidget(Ref<FWidget> childWidget);
    	
    	void RemoveChildWidget(Ref<FWidget> childWidget);

        void DetachChild(Ref<FWidget> child) override;

		int GetChildCount() const { return (int)children.GetSize(); }

		Ref<FWidget> GetChildAt(int index) const
		{
			if (index < 0 || index >= (int)children.GetSize())
            {
                return nullptr;
            }
			return children[index];
		}

    protected: // - Internal -

        FIELD()
        Array<Ref<FWidget>> children;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FContainerWidget.rtti.h"
