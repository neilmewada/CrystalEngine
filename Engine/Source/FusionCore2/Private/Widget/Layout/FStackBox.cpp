#include "FusionCore.h"

namespace CE
{

    FStackBox::FStackBox()
    {
        m_HAlign = HAlign::Fill;
        m_VAlign = VAlign::Fill;
    }

    void FStackBox::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        Super::SetParentSurfaceRecursive(surface);

        for (int i = 0; i < children.GetSize(); i++)
        {
            children[i]->SetParentSurfaceRecursive(surface);
        }
    }

    Vec2 FStackBox::MeasureContent(Vec2 availableSize)
    {
        return Vec2();
    }

    void FStackBox::ArrangeContent(Vec2 finalSize)
    {
	    
    }

    void FVerticalStack::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

		thread_local const CE::Name stackDirectionName = "StackDirection";

		if (propertyName == stackDirectionName)
        {
			m_StackDirection = FStackDirection::Vertical;
        }
    }

    void FHorizontalStack::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        thread_local const CE::Name stackDirectionName = "StackDirection";

        if (propertyName == stackDirectionName)
        {
            m_StackDirection = FStackDirection::Horizontal;
        }
    }

}

