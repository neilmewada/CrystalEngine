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

		        if (parent->IsPaintBoundary())
		        {
                    surface->AddDirtyPaintRoot(parent->GetLayer());
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
        return ownedLayer.IsValid();
    }

    void FWidget::OnPaint(FPainter& painter)
    {
        ZoneScoped;

        SetWidgetFlag(FWidgetFlags::PaintDirty, false);

        cachedLayerSpaceTransform = painter.GetCurrentTransform() * FAffineTransform::Translation(layoutPosition) * m_Transform;
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

    SubClass<FLayer> FWidget::DetermineLayerType()
    {
        if (EnumHasAnyFlags(widgetFlags, FWidgetFlags::ForceCompositingBoundary))
            return FCompositingLayer::StaticClass();

        if (parentWidget.IsNull() && parentSurface.IsValid())
            return FCompositingLayer::StaticClass();

        if (m_Opacity < 0.9999f)
            return FCompositingLayer::StaticClass();

        if (EnumHasAnyFlags(widgetFlags, FWidgetFlags::ForcePaintBoundary))
            return FLayer::StaticClass();

        return nullptr;
    }

    void FWidget::UpdateLayerOwnership()
    {
        const SubClass<FLayer> layerType = DetermineLayerType();
        const bool should = layerType.IsValid();
		const bool has = ownedLayer.IsValid();

		if (should && !has)
        {
            PromoteToLayerOwner();
        }
        else if (!should && has)
        {
            DemoteFromLayerOwner();
        }
        else if (should && has && ownedLayer->GetClass() != layerType.GetClassType())
        {
            DemoteFromLayerOwner();
            PromoteToLayerOwner();
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

        if (propertyName == styleProperty)
        {
            NotifyStyleStateChanged();
        }
    }

    void FWidget::PromoteToLayerOwner()
    {
        ZoneScoped;

        const SubClass<FLayer> layerType = DetermineLayerType();
        if (!layerType.IsValid())
            return;

        ownedLayer = CreateObject<FLayer>(this, "Layer", OF_NoFlags, layerType);
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

        ZoneScoped;

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
        ZoneScoped;

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

        const bool wasPaintDirty = IsPaintDirty();
        const bool wasLayoutDirty = IsLayoutDirty();
        widgetFlags &= ~(FWidgetFlags::PaintDirty | FWidgetFlags::LayoutDirty);

        UpdateLayerOwnership();

        if (wasPaintDirty)
            MarkPaintDirty();
        if (wasLayoutDirty)
            MarkLayoutDirty();

        this->parentSurface = surface;
    }

    void FWidget::DetachFromParent()
    {
        if (Ref<FWidget> parent = parentWidget.Lock())
        {
            parent->DetachChild(this);
        }
    }

} // namespace CE

