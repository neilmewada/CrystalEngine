#include "FusionCore.h"

namespace CE
{

    FWidget::FWidget()
    {
        m_MaxHeight = NumericLimits<f32>::Infinity();
        m_MaxWidth = NumericLimits<f32>::Infinity();
    }

    void FWidget::MarkPaintDirty()
    {
        ZoneScoped;

		if (IsPaintDirty())
            return;

		flags |= FWidgetFlags::PaintDirty;

		if (Ref<FSurface> surface = parentSurface.Lock())
        {
            // TODO: Mark paint root as dirty
        }
    }

    void FWidget::MarkLayoutDirty()
    {
        ZoneScoped;

        if (IsLayoutDirty())
            return;

		flags |= FWidgetFlags::LayoutDirty;

		if (Ref<FSurface> surface = parentSurface.Lock())
        {
			Ref<FWidget> parent = this;

            while (parent != nullptr)
            {
                if (parent != this && parent->IsLayoutDirty())
                {
	                break;
                }

	            if (parent->IsLayoutRoot())
	            {
                    surface->AddPendingLayoutRoot(parent);
                    break;
	            }

				parent = parent->GetParentWidget();
            }
        }
    }

    Vec2 FWidget::ApplyLayoutConstraints(Vec2 desiredSize)
    {
        ZoneScoped;

        f32 constrainedWidth = Math::Clamp(desiredSize.x, m_MinWidth + m_Padding.left + m_Padding.right, m_MaxWidth + m_Padding.left + m_Padding.right);
        f32 constrainedHeight = Math::Clamp(desiredSize.y, m_MinHeight + m_Padding.top + m_Padding.bottom, m_MaxHeight + m_Padding.top + m_Padding.bottom);

		return Vec2(constrainedWidth, constrainedHeight);
    }

    bool FWidget::IsLayoutRoot()
    {
        const bool isFixedSize = Math::ApproxEquals(m_MinWidth, m_MaxWidth) && Math::ApproxEquals(m_MinHeight, m_MaxHeight);
        if (isFixedSize)
            return true;

		const bool isRootWidget = parentWidget.IsNull() && parentSurface.IsValid();
        if (isRootWidget)
            return true;

        return false;
    }

    Vec2 FWidget::GetMinimumContentSize()
    {
        return Vec2(m_MinWidth + m_Padding.left + m_Padding.right, m_MinHeight + m_Padding.top + m_Padding.bottom);
    }

    Vec2 FWidget::MeasureContent(Vec2 availableSize)
    {
        return GetMinimumContentSize();
    }

    void FWidget::ArrangeContent(Vec2 finalSize)
    {
        ZoneScoped;

		layoutSize = finalSize;
    }

    FWidget::Self& FWidget::Width(f32 width)
    {
        return (*this)
            .MinWidth(width)
            .MaxWidth(width);
    }

    FWidget::Self& FWidget::Height(f32 height)
    {
        return (*this)
            .MinHeight(height)
            .MaxHeight(height);
    }

} // namespace CE

