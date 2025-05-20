#include "FusionCore.h"

namespace CE
{

    FWidget::FWidget()
    {
        m_Scale = Vec2(1, 1);
        m_Anchor = Vec2(0.5f, 0.5f);
        m_MaxHeight = NumericLimits<f32>::Infinity();
        m_MaxWidth = NumericLimits<f32>::Infinity();

        m_Enabled = true;
        m_Visible = true;
    }

    FWidget::~FWidget()
    {
        
    }

    FFusionContext* FWidget::GetContext()
    {
        if (!context && parent)
        {
            context = parent->GetContext();
        }
        return context;
    }

    void FWidget::OnPaint(FPainter* painter)
    {
        ZoneScoped;

        globalPosition = painter->GetTopCoordinateSpace() * Vec4(0, 0, 0, 1);
    }

    void FWidget::HandleEvent(FEvent* event)
    {
        if (event->stopPropagation)
            return;

        ZoneScoped;

        m_OnEvent(event);

        if (event->type == FEventType::HierarchyDetached)
        {
            OnLostFocus();
        }
        else if (event->type == FEventType::FocusChanged)
        {
            FFocusEvent* focusEvent = static_cast<FFocusEvent*>(event);

            if (isFocused != focusEvent->gotFocus)
            {
                isFocused = focusEvent->gotFocus;

                if (SupportsFocusEvents())
                {
                    if (isFocused)
                        OnGotFocus();
                    else
                        OnLostFocus();
                }
            }
        }

        if (parent != nullptr && event->direction == FEventDirection::BottomToTop)
        {
            parent->HandleEvent(event);
        }
    }

    void FWidget::UpdateLocalTransform()
    {
        isTranslationOnly = false;

        if (Math::ApproxEquals(m_Angle, 0.0f) &&
            Math::ApproxEquals(m_Scale.x, 1.0f) &&
            Math::ApproxEquals(m_Scale.y, 1.0f))
        {
            isTranslationOnly = true;

            localTransform = Matrix4x4::Translation(computedPosition + m_Translation);

            mouseTransform = Matrix4x4::Translation(Vec2());
        }
        else
        {
            localTransform =
                Matrix4x4::Translation(GetComputedPosition() + m_Translation + GetComputedSize() * m_Anchor) *
                Matrix4x4::Angle(m_Angle) *
                Matrix4x4::Scale(m_Scale) *
                Matrix4x4::Translation(-GetComputedSize() * m_Anchor);

            Vec3 invScale = Vec3(1 / m_Scale.x, 1 / m_Scale.y, 1);

            mouseTransform = Matrix4x4::Translation(computedPosition + m_Translation + computedSize * m_Anchor) *
                Matrix4x4::Angle(-m_Angle) *
                Matrix4x4::Scale(invScale) *
                Matrix4x4::Translation(-computedPosition - m_Translation - computedSize * m_Anchor);
        }

        if (parent != nullptr)
        {
            globalTransform = parent->globalTransform * localTransform;
        }
        else
        {
            globalTransform = localTransform;
        }
    }

    FWidget* FWidget::HitTest(Vec2 localMousePos)
    {
        ZoneScoped;

        if (!Enabled())
            return nullptr;

        Vec2 rectPos = computedPosition + m_Translation;
        Vec2 rectSize = computedSize;

        localMousePos = mouseTransform * Vec4(localMousePos.x, localMousePos.y, 0, 1);

        Rect rect = Rect::FromSize(rectPos, rectSize);

        return rect.Contains(localMousePos) ? this : nullptr;
    }

    bool FWidget::ParentExistsRecursive(FWidget* parent)
    {
        ZoneScoped;

        if (this->parent == nullptr)
            return false;
        if (this->parent == parent || this == parent)
            return true;

        return this->parent->ParentExistsRecursive(parent);
    }

    bool FWidget::FocusParentExistsRecursive(FWidget* parent)
    {
        return ParentExistsRecursive(parent);
    }

    void FWidget::OnAttachedToParent(FWidget* parent)
    {
        ApplyStyleRecursively();
    }

    void FWidget::OnDetachedFromParent(FWidget* parent)
    {
        ZoneScoped;

        FHierarchyEvent event{};
        event.direction = FEventDirection::TopToBottom;
        event.type = FEventType::HierarchyDetached;
        event.sender = this;
        event.parentWidget = parent;
        event.topMostWidget = this;

        HandleEvent(&event);
    }

    void FWidget::ClearStyle()
    {
        m_Style = nullptr;
    }

    bool FWidget::IntrinsicSizeExists()
    {
        return intrinsicSize.GetSqrMagnitude() <= std::numeric_limits<f32>::epsilon();
    }

    void FWidget::Focus()
    {
        ZoneScoped;

        FFusionContext* context = GetContext();
        if (context)
        {
            context->SetFocusWidget(this);
        }
    }

    void FWidget::Unfocus()
    {
        ZoneScoped;

        FFusionContext* context = GetContext();
        if (IsFocused() && context)
        {
            context->SetFocusWidget(parent.Get());
        }
    }

    bool FWidget::IsInsidePopup()
    {
        if (parent == nullptr)
            return false;
        if (parent->IsOfType<FPopup>())
            return true;

        return parent->IsInsidePopup();
    }

    FPopup* FWidget::FindPopupInParent()
    {
        if (parent == nullptr)
            return nullptr;
        if (parent->IsOfType<FPopup>())
            return (FPopup*)parent.Get();

        return parent->FindPopupInParent();
    }

    bool FWidget::IsEnabledInHierarchy()
    {
        ZoneScoped;

        return Enabled() && (parent == nullptr || parent->IsEnabledInHierarchy());
    }

    bool FWidget::IsVisibleInHierarchy()
    {
        ZoneScoped;

        return Enabled() && Visible() && (parent == nullptr || parent->IsVisibleInHierarchy());
    }

    bool FWidget::IsVisible()
    {
        ZoneScoped;

        return Enabled() && Visible();
    }

    void FWidget::SetParent(FWidget* parent)
    {
        this->parent = parent;
    }

    void FWidget::OnFusionPropertyModified(const CE::Name& propertyName)
    {
        ZoneScoped;

        static const HashSet<CE::Name> transformProperties = { "Translation", "Angle", "Scale" };
        static const CE::Name fillRatioName = "FillRatio";

        static const CE::Name enabledName = "Enabled";

        if (transformProperties.Exists(propertyName))
        {
	        UpdateLocalTransform();
        }

        if (propertyName == fillRatioName)
        {
            m_FillRatio = Math::Max(m_FillRatio, 0.0f);
        }
    }

    void FWidget::OnAfterConstruct()
    {
        ZoneScoped;

	    Super::OnAfterConstruct();

        Construct();
    }

    void FWidget::OnBeginDestroy()
    {
        ZoneScoped;

	    Super::OnBeginDestroy();

        FusionApplication* app = FusionApplication::TryGet();

        if (GetContext())
        {
            GetContext()->OnWidgetDestroyed(this);
        }
        else if (app)
        {
            app->GetRootContext()->OnWidgetDestroyed(this);
        }

        if (parent)
        {
            parent->OnChildWidgetDestroyed(this);
        }
        parent = nullptr;
    }

    void FWidget::CalculateIntrinsicSize()
    {
        ZoneScoped;

        intrinsicSize = Vec2(m_MinWidth + m_Padding.left + m_Padding.right, 
            m_MinHeight + m_Padding.top + m_Padding.bottom);

        ApplyIntrinsicSizeConstraints();
    }

    void FWidget::PlaceSubWidgets()
    {
        ZoneScoped;

        ApplySizeConstraints();

        UpdateLocalTransform();

        OnPostComputeLayout();
    }

    void FWidget::ApplyIntrinsicSizeConstraints()
    {
        intrinsicSize.width = Math::Clamp(intrinsicSize.width,
            m_MinWidth + m_Padding.left + m_Padding.right,
            m_MaxWidth + m_Padding.left + m_Padding.right);

        intrinsicSize.height = Math::Clamp(intrinsicSize.height,
            m_MinHeight + m_Padding.top + m_Padding.bottom,
            m_MaxHeight + m_Padding.top + m_Padding.bottom);
    }

    void FWidget::ApplySizeConstraints()
    {
        computedSize.width = Math::Clamp(computedSize.width, 
            m_MinWidth + m_Padding.left + m_Padding.right,
            m_MaxWidth + m_Padding.left + m_Padding.right);

        computedSize.height = Math::Clamp(computedSize.height,
            m_MinHeight + m_Padding.bottom + m_Padding.top,
            m_MaxHeight + m_Padding.bottom + m_Padding.top);
    }

    void FWidget::OnPostComputeLayout()
    {

    }

    bool FWidget::AddChild(FWidget* child)
    {
        ZoneScoped;

        if (child == nullptr)
            return false;

        Ref<FWidget> strongRef = child;

        if (TryAddChild(child))
        {
            if (child->GetOuter() == nullptr)
            {
                AttachSubobject(child);
            }

            child->parent = this;
            AttachSubobject(child);
            child->OnAttachedToParent(this);
            MarkLayoutDirty();

            ApplyStyle();

            return true;
        }

        return false;
    }

    bool FWidget::AddChild(FWidget& child)
    {
        return AddChild(&child);
    }

    void FWidget::RemoveChild(FWidget* child)
    {
        ZoneScoped;

        if (child == nullptr)
            return;

        Ref<FWidget> strongRef = child;

        if (TryRemoveChild(child))
        {
            child->OnDetachedFromParent(this);
            DetachSubobject(child);
            child->parent = nullptr;
            MarkLayoutDirty();

            ApplyStyle();
        }
    }

    void FWidget::ApplyStyle()
    {
        ZoneScoped;

        if (styleKey.IsValid() && m_Style == nullptr)
        {
            Style(styleKey);
        }
        else
        {
            Style(m_Style);
        }
    }

    void FWidget::ApplyStyleRecursively()
    {
        ApplyStyle();
    }

    void FWidget::SetContextRecursively(FFusionContext* context)
    {
        this->context = context;

        ApplyStyle();
    }

    void FWidget::QueueDestroy()
    {
        FusionApplication::Get()->QueueDestroy(this);
    }

    void FWidget::MarkLayoutDirty()
    {
        FFusionContext* context = GetContext();
        if (context)
        {
            context->MarkLayoutDirty();
        }
    }

    void FWidget::MarkDirty()
    {
        FFusionContext* context = GetContext();
        if (context)
        {
            context->MarkDirty();
        }
    }

    Vec2 FWidget::GetGlobalPosition() const
    {
        return globalPosition;
    }

    void FWidget::Construct()
    {
        
    }

    FWidget& FWidget::Width(f32 width)
    {
        return (*this)
            .MinWidth(width)
            .MaxWidth(width);
    }

    FWidget& FWidget::Height(f32 height)
    {
        return (*this)
            .MinHeight(height)
            .MaxHeight(height);
    }

    FWidget& FWidget::Style(FStyle* style)
    {
        ZoneScoped;

        if (style == nullptr)
        {
            m_Style = nullptr;
            MarkDirty();
        }
        
        if (style && IsOfType(style->GetWidgetClass()))
        {
            m_Style = style;
            if (!IsDefaultInstance())
            {
	            m_Style->MakeStyle(*this);
            }
            MarkDirty();
        }
        return *this;
    }

    FWidget& FWidget::Style(const CE::Name& styleKey)
    {
        ZoneScoped;

        if (!styleKey.IsValid())
            return *this;

        m_Style = nullptr;
        this->styleKey = styleKey;

        FFusionContext* context = GetContext();
        if (!context)
            return *this;

        FStyleSet* defaultStyleSet = context->GetDefaultStyleSet();
        if (!defaultStyleSet)
            return *this;

        FStyle* style = defaultStyleSet->FindStyle(styleKey);
        return Style(style);
    }

    FWidget& FWidget::Style(FStyleSet* styleSet, const CE::Name& styleKey)
    {
        ZoneScoped;

        if (!styleSet)
            return *this;

        FStyle* style = styleSet->FindStyle(styleKey);
        if (!style)
            return *this;

        return Style(style);
    }

} // namespace CE

