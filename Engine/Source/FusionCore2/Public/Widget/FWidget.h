#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FWidget : public Object
    {
        CE_CLASS(FWidget, CE::Object)
    protected:

        FWidget();
        
    public:

        virtual void MarkPaintDirty();

        virtual void MarkLayoutDirty();

    protected:

		virtual void OnFusionPropertyModified(const Name& propertyName) {}


    private:

        FIELD()
		WeakRef<FWidget> parentWidget;

        FIELD()
		WeakRef<FSurface> parentSurface;

        FUSION_WIDGET;
    };
    
} // namespace CE

#include "FWidget.rtti.h"
