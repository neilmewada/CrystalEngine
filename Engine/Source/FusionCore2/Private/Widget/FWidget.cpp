#include "FusionCore.h"

namespace CE
{

    FWidget::FWidget()
    {
        m_MaxHeight = NumericLimits<f32>::Infinity();
        m_MaxWidth = NumericLimits<f32>::Infinity();
    }

    void FWidget::OnAfterConstruct()
    {
        ZoneScoped;

        Super::OnAfterConstruct();

        try
        {
            Construct();
        }
        catch (const Exception& exception)
        {
            CE_LOG(Critical, All, "Exception occurred during widget [{}] construction: {}\n{}", GetClass()->GetName().GetLastComponent(), exception.what(), exception.GetStackTraceString(true));
	        widgetFlags |= FWidgetFlags::Faulted;
        }
    }

    void FWidget::Construct()
    {
        ZoneScoped;

    }

    void FWidget::MarkPaintDirty()
    {
        ZoneScoped;

		if (IsPaintDirty())
            return;

		widgetFlags |= FWidgetFlags::PaintDirty;

		if (Ref<FSurface> surface = parentSurface.Lock())
        {
		    Ref<FWidget> parent = this;

		    while (parent != nullptr)
		    {
		        if (parent != this && parent->IsPaintDirty())
		        {
		            break;
		        }

		        if (parent->IsPaintBoundary())
		        {
                    parent->SetWidgetFlag(FWidgetFlags::PaintDirty, true);
		            break;
		        }

		        parent = parent->GetParentWidget();
		    }
        }
    }

    void FWidget::MarkLayoutDirty()
    {
        ZoneScoped;
        
        if (IsLayoutDirty())
            return;

		widgetFlags |= FWidgetFlags::LayoutDirty;

		if (Ref<FSurface> surface = parentSurface.Lock())
        {
			Ref<FWidget> parent = this;

            while (parent != nullptr)
            {
                if (parent != this && parent->IsLayoutDirty())
                {
	                break;
                }

	            if (parent->IsLayoutBoundary())
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

    void FWidget::SetLayoutPosition(Vec2 newPosition)
    {
        ZoneScoped;

		if (layoutPosition == newPosition)
            return;

		layoutPosition = newPosition;

        MarkPaintDirty();
    }

    bool FWidget::IsLayoutBoundary()
    {
        ZoneScoped;

        const bool isFixedSize = Math::ApproxEquals(m_MinWidth, m_MaxWidth) && Math::ApproxEquals(m_MinHeight, m_MaxHeight);
        if (isFixedSize)
            return true;

		const bool isRootWidget = parentWidget.IsNull() && parentSurface.IsValid();
        if (isRootWidget)
            return true;

        return false;
    }

    bool FWidget::IsPaintBoundary()
    {
        return TestWidgetFlags(FWidgetFlags::CachedPaintBoundary) || TestWidgetFlags(FWidgetFlags::CachedCompositingBoundary);
    }

    bool FWidget::IsCompositingBoundary()
    {
        return TestWidgetFlags(FWidgetFlags::CachedCompositingBoundary);
    }

    void FWidget::UpdateBoundaryFlags()
    {
        bool isCompositing = (m_Opacity < 0.999f) || EnumHasFlag(widgetFlags, FWidgetFlags::ForceCompositingBoundary);
        bool isPaint = (parentWidget == nullptr) || EnumHasFlag(widgetFlags, FWidgetFlags::ForcePaintBoundary) || isCompositing;

        bool wasCompositing = EnumHasFlag(widgetFlags, FWidgetFlags::CachedCompositingBoundary);
        bool wasPaint = EnumHasFlag(widgetFlags, FWidgetFlags::CachedPaintBoundary);

        SetWidgetFlag(FWidgetFlags::CachedCompositingBoundary, isCompositing);
        SetWidgetFlag(FWidgetFlags::CachedPaintBoundary, isPaint);

        if (isCompositing != wasCompositing || isPaint != wasPaint)
        {
            if (Ref<FSurface> surface = parentSurface.Lock())
                surface->MarkLayerTreeDirty();
        }
    }

    void FWidget::Paint(FPainter& painter)
    {
        ZoneScoped;

        // TODO
    }

    void FWidget::NotifyStyleStateChanged()
    {
        if (!m_Style.IsValid())
            return;

        FStyleSet* styleSet = nullptr;

        if (Ref<FSurface> surface = parentSurface.Lock())
        {
            styleSet = surface->GetStyleSet().Get();
        }

        if (!styleSet)
            styleSet = FApplication::Get()->GetDefaultStyleSet().Get();
        
        if (!styleSet)
            return;

        if (Ref<FStyle> style = styleSet->FindStyle(m_Style))
        {
            style->MakeStyle(*this);
        }
    }

    void FWidget::SetWidgetFlag(FWidgetFlags flag, bool set)
    {
        if (set)
        {
            widgetFlags |= flag;
        }
        else
        {
            widgetFlags &= ~flag;
        }
    }

    void FWidget::OnFusionPropertyModified(const CE::Name& propertyName)
    {
        thread_local const CE::Name styleProperty = "Style";
        thread_local const CE::Name opacityProperty = "Opacity";

        if (propertyName == styleProperty)
        {
            NotifyStyleStateChanged();
        }
        else if (propertyName == opacityProperty)
        {
            UpdateBoundaryFlags();
        }
    }

    Vec2 FWidget::GetMinimumContentSize()
    {
        return Vec2(m_MinWidth + m_Padding.left + m_Padding.right, m_MinHeight + m_Padding.top + m_Padding.bottom);
    }

    Vec2 FWidget::MeasureContent(Vec2 availableSize)
    {
		return desiredSize = GetMinimumContentSize();
    }

    void FWidget::ArrangeContent(Vec2 finalSize)
    {
        ZoneScoped;

        widgetFlags &= ~FWidgetFlags::LayoutDirty;

		Vec2 newLayoutSize = ApplyLayoutConstraints(finalSize);
        if (layoutSize == newLayoutSize)
			return;

		layoutSize = newLayoutSize;

        MarkPaintDirty();
    }

    void FWidget::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        ZoneScoped;

        this->parentSurface = surface;

        const bool wasPaintDirty = IsPaintDirty();
        const bool wasLayoutDirty = IsLayoutDirty();
        widgetFlags &= ~(FWidgetFlags::PaintDirty | FWidgetFlags::LayoutDirty);

        if (wasPaintDirty)
            MarkPaintDirty();
        if (wasLayoutDirty)
            MarkLayoutDirty();
    }

    void FWidget::DetachFromParent()
    {
        if (Ref<FWidget> parent = parentWidget.Lock())
        {
            parent->DetachChild(this);
        }
    }

} // namespace CE

