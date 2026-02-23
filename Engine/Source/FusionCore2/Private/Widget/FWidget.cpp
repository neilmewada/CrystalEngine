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
            CE_LOG(Error, All, "Exception occurred during widget [{}] construction: {}\nStack Trace:\n{}", GetClass()->GetName().GetLastComponent(), exception.what(), exception.GetStackTraceString(true));
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

        // TODO: Update this method

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

		        if (parent->IsPaintRoot())
		        {
		            surface->AddDirtyPaintRoot(parent);
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

    void FWidget::SetLayoutPosition(Vec2 newPosition)
    {
        ZoneScoped;

		if (layoutPosition == newPosition)
            return;

		layoutPosition = newPosition;

        MarkPaintDirty();
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

    bool FWidget::IsPaintRoot()
    {
        return ownedLayer.IsValid();
    }

    bool FWidget::ShouldOwnLayer()
    {
        if (EnumHasAnyFlags(widgetFlags, FWidgetFlags::ForceOwnLayer))
            return true;

        if (parentWidget.IsNull() && parentSurface.IsValid())
            return true;

        if (m_Opacity < 0.9999f)
            return true;

        return false;
    }

    void FWidget::UpdateLayerOwnership()
    {
        const bool should = ShouldOwnLayer();
		const bool has = ownedLayer.IsValid();

		if (should && !has)
        {
            PromoteToLayerOwner();
        }
        else if (!should && has)
        {
            DemoteFromLayerOwner();
        }
    }

    void FWidget::PromoteToLayerOwner()
    {
        ownedLayer = CreateObject<FLayer>(this, "Layer");
		ownedLayer->SetOwningWidget(this);
        
    	if (FLayer* parentLayer = FindNearestAncestorLayer())
        {
            parentLayer->AddChild(ownedLayer);
		}

        MarkPaintDirty();
    }

    void FWidget::DemoteFromLayerOwner()
    {
        if (!ownedLayer)
            return;

        Ref<FLayer> parentLayer = ownedLayer->GetParentLayer();

        // Re-parent all child layers up to our former parent.
        // Iterate until empty since AddChild removes children from ownedLayer as it goes.
        while (ownedLayer->GetChildCount() > 0)
        {
            Ref<FLayer> child = ownedLayer->GetChildAt(0);
            if (!child)
                break;

            if (parentLayer)
                parentLayer->AddChild(child);
            else
                ownedLayer->RemoveChild(child);
        }

        // Detach our layer from the tree
        if (parentLayer)
            parentLayer->RemoveChild(ownedLayer);

        ownedLayer = nullptr;

        // Re-bake this widget into the parent layer
        MarkPaintDirty();
    }

    FLayer* FWidget::FindNearestAncestorLayer()
    {
        Ref<FWidget> ancestor = parentWidget.Lock();
        while (ancestor)
        {
            if (ancestor->ownedLayer)
                return ancestor->ownedLayer.Get();
            ancestor = ancestor->GetParentWidget();
        }
        return nullptr;
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
        UpdateLayerOwnership();
    }

    void FWidget::DetachFromParent()
    {
        if (Ref<FWidget> parent = parentWidget.Lock())
        {
            parent->DetachChild(this);
        }
    }

} // namespace CE

