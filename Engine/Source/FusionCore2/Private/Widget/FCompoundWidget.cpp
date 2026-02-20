#include "FusionCore.h"

namespace CE
{

    FCompoundWidget::FCompoundWidget()
    {

    }

    void FCompoundWidget::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        ZoneScoped;

        Super::SetParentSurfaceRecursive(surface);

        if (m_Child)
        {
            m_Child->SetParentSurfaceRecursive(surface);
        }
    }

    void FCompoundWidget::DetachChild(Ref<FWidget> child)
    {
        ZoneScoped;

        Super::DetachChild(child);

        if (m_Child == child && m_Child != nullptr)
        {
            m_Child->SetParentSurfaceRecursive(nullptr);
            m_Child->SetParentWidget(nullptr);
            m_Child = nullptr;

            MarkLayoutDirty();
        }
    }

    void FCompoundWidget::SetChild(Ref<FWidget> widget)
    {
        ZoneScoped;

        if (m_Child == widget)
            return;

        DetachChild(m_Child);

        if (widget)
        {
            widget->DetachFromParent();
        }

        m_Child = widget;

        if (m_Child)
        {
            m_Child->SetParentWidget(this);

            if (m_Child->GetParentSurface() != GetParentSurface())
            {
                m_Child->SetParentSurfaceRecursive(GetParentSurface());
            }
        }

        MarkLayoutDirty();
    }

    FCompoundWidget& FCompoundWidget::Child(FWidget& widget)
    {
        SetChild(&widget);
        return *this;
    }
}

