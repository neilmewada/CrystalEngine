#pragma once

namespace CE
{
    class FWidget;

    DECLARE_SCRIPT_EVENT(FPaintEvent, FPainter*);

    struct FWidgetBuilder
    {
        FWidgetBuilder() {}
    };

    CLASS()
    class FUSIONCORE_API FWidget : public Object
    {
        CE_CLASS(FWidget, Object)
    public:

        FWidget();

        virtual ~FWidget();

        // - Public API -

        Ref<FFusionContext> GetContext();

        Ref<FNativeContext> GetNativeContext();

        virtual void CalculateIntrinsicSize();

        virtual void PlaceSubWidgets();

        virtual void ApplyIntrinsicSizeConstraints();

        virtual void ApplySizeConstraints();

        void UpdateLocalTransform();

        virtual void OnChildWidgetDestroyed(FWidget* child) {}

        virtual void OnPostComputeLayout();

        bool AddChild(FWidget* child);
        bool AddChild(FWidget& child);
        void RemoveChild(FWidget* child);

        FWidget* GetParent() const { return parent.Get(); }

        virtual void ApplyStyle();

        virtual void ApplyStyleRecursively();

        template<typename... TArgs>
        struct TValidate_Children : TFalseType
        {};

        template<typename T>
        struct TValidate_Children<T> : TBoolConst<TIsBaseClassOf<FWidget, T>::Value or TIsBaseClassOf<FWidgetBuilder, T>::Value>
        {};

        template<typename TFirst, typename... TRest>
        struct TValidate_Children<TFirst, TRest...> : TBoolConst<TValidate_Children<TFirst>::Value and TValidate_Children<TRest...>::Value>
        {};

        template<typename... TArgs> requires TValidate_Children<TArgs...>::Value and (sizeof...(TArgs) > 0)
        FWidget& operator()(const TArgs&... childWidget)
        {
            using TupleType = std::tuple<const TArgs&...>;
            TupleType args = { childWidget... };

            constexpr_for<0, sizeof...(TArgs), 1>([&](auto i)
                {
                    using ArgTypeBase = std::tuple_element_t<i(), TupleType>;
                    using ArgType = std::remove_cvref_t<ArgTypeBase>;
                    if constexpr (TIsBaseClassOf<FWidget, ArgType>::Value)
                    {
                        this->AddChild(const_cast<ArgType*>(&std::get<i()>(args)));
                    }
                    else if constexpr (TIsBaseClassOf<FWidgetBuilder, ArgType>::Value)
                    {
                        const_cast<ArgType*>(&std::get<i()>(args))->Build(this);
                    }
                });

            return *this;
        }

        virtual void SetContextRecursively(FFusionContext* context);

        Vec2 GetIntrinsicSize() const { return intrinsicSize; }

        void QueueDestroy();

        void MarkLayoutDirty();

        //! @brief Mark the widget dirty for re-rendering
        void MarkDirty();

        Vec2 GetGlobalPosition() const;

        Vec2 GetComputedPosition() const { return computedPosition; }

        Vec2 GetComputedSize() const { return computedSize; }

        const Matrix4x4& GetLocalTransform() const { return localTransform; }
        const Matrix4x4& GetGlobalTransform() const { return globalTransform; }

        void SetComputedPosition(Vec2 pos) { computedPosition = pos; }

        void SetComputedSize(Vec2 size) { computedSize = size; }

        virtual void OnPaint(FPainter* painter);

        virtual void HandleEvent(FEvent* event);

        virtual FWidget* HitTest(Vec2 localMousePos);

        FWidget* SelfHitTest(Vec2 localMousePos);

        virtual bool ChildExistsRecursive(FWidget* child) { return this == child; }

        virtual bool ParentExistsRecursive(FWidget* parent);

        virtual bool FocusParentExistsRecursive(FWidget* parent);

        virtual bool SupportsMouseEvents() const { return false; }

        virtual bool SupportsKeyboardEvents() const { return false; }

    	virtual bool SupportsDragEvents() const { return false; }

		virtual bool SupportsDropTarget() const { return false; }

        virtual bool SupportsFocusEvents() const { return false; }

        virtual bool SupportsKeyboardFocus() const { return SupportsFocusEvents() && SupportsKeyboardEvents(); }

        virtual bool CapturesMouseWheel() const { return false; }

        virtual void ClearStyle();

        bool IsFocused() const { return isFocused; }
        bool IsCulled() const { return isCulled; }

        bool IntrinsicSizeExists();

        virtual bool IsSizeDependentOnContent() const { return true; }
        virtual bool IsSizeDependentOnParent() const { return true; }

        void Focus();
        void Unfocus();

        bool IsInsidePopup();

        FPopup* FindPopupInParent();

        //! @brief Returns true if this widget is enabled in parent hierarchy.
        //! Use this method sparingly, especially in recursive functions as it can add overhead.
        bool IsEnabledInHierarchy();

        //! @brief Returns true if this widget is both enabled and visible in parent hierarchy.
        //! Use this method sparingly, especially in recursive functions as it can add overhead.
        bool IsVisibleInHierarchy();

        //! @brief Returns true if this widget is both enabled and visible.
        bool IsVisible();

        bool IsTranslationOnly() const { return isTranslationOnly; }

        //! @brief Internal use only! Never call this function directly.
        void SetParent(FWidget* parent);

    private:

    protected:

        virtual void OnGotFocus() {}
        virtual void OnLostFocus() {}

        virtual void OnAttachedToParent(FWidget* parent);
        virtual void OnDetachedFromParent(FWidget* parent);

        virtual bool TryAddChild(FWidget* child) { return false; }

        virtual bool TryRemoveChild(FWidget* child) { return false; }

        virtual void OnFusionPropertyModified(const Name& propertyName);

        void OnAfterConstruct() override;

        void OnBeginDestroy() override;

        virtual void Construct();

        //! @brief Computed position in parent widget's coordinate space.
        Vec2 computedPosition;

        Vec2 computedSize;

        Vec2 intrinsicSize;

        //! @brief Global position of the widget, i.e. position in the leaf/terminal context.
        Vec2 globalPosition;

        //! @brief Transformation matrix that is pushed by parent widget to coordinate space stack.
        Matrix4x4 localTransform;

        Matrix4x4 mouseTransform;

        Matrix4x4 globalTransform;

        FIELD()
        CE::Name styleKey;

    private:  // - Fields -

        FIELD()
        WeakRef<FFusionContext> context = nullptr;

        FIELD()
        WeakRef<FWidget> parent = nullptr;

        // - Flags -

        FIELD(ReadOnly)
        bool isFocused = false;
        bool isTranslationOnly = false;

    protected:

        FIELD(ReadOnly)
        bool isCulled = false;

    protected: // - Fusion Fields -

        FIELD(FusionProperty)
        FStyle* m_Style = nullptr;

    public:  // - Fusion Properties -

        FUSION_EVENT(ScriptEvent<void(FEvent*)>, OnEvent);

        FUSION_LAYOUT_PROPERTY(Vec4, Padding);
        FUSION_LAYOUT_PROPERTY(Vec4, Margin);
        FUSION_PROPERTY(Vec2, Anchor);

        FUSION_LAYOUT_PROPERTY(CE::VAlign, VAlign);
        FUSION_LAYOUT_PROPERTY(CE::HAlign, HAlign);

        FUSION_LAYOUT_PROPERTY(bool, Enabled);
        FUSION_LAYOUT_PROPERTY(bool, WindowDragHitTest);
        FUSION_LAYOUT_PROPERTY(bool, IgnoreHitTest);
        FUSION_LAYOUT_PROPERTY(bool, Visible);

        FUSION_LAYOUT_PROPERTY(f32, MinWidth);
        FUSION_LAYOUT_PROPERTY(f32, MaxWidth);

        FUSION_LAYOUT_PROPERTY(f32, MinHeight);
        FUSION_LAYOUT_PROPERTY(f32, MaxHeight);

        Self& Width(f32 width);
        Self& Height(f32 height);

        FUSION_LAYOUT_PROPERTY(f32, FillRatio);

        FUSION_PROPERTY(Vec2, Translation);
        FUSION_PROPERTY(f32, Angle);
        FUSION_PROPERTY(Vec2, Scale);

        Self& Style(FStyle* style);

        Self& Style(const CE::Name& styleKey);
        Self& Style(FStyleSet* styleSet, const CE::Name& styleKey);

        template<typename TWidget> requires TIsBaseClassOf<FWidget, TWidget>::Value
        TWidget& As()
        {
            TWidget* cast = Object::CastTo<TWidget>(this);
            if (cast == nullptr)
            {
                throw FusionException(String::Format("Cannot cast object ({}) to type {}",
                    GetName(), GetStaticClass<TWidget>()->GetName()), 
                    this);
            }
            return (TWidget&)*cast;
        }

        Self& Name(const Name& newName)
        {
            this->SetName(newName);
            return *this;
        }

        template<typename TWidget> requires TIsBaseClassOf<FWidget, TWidget>::Value
        TWidget& Assign(TWidget*& out)
        {
            out = (TWidget*)this;
            return *out;
        }

        template<typename TWidget> requires TIsBaseClassOf<FWidget, TWidget>::Value
        TWidget& Assign(Ref<TWidget>& out)
        {
            out = (TWidget*)this;
            return *out;
        }

        template<typename T>
        struct TValidate_IfTrue
        {
            using Traits = TFunctionTraits<T>;
            using ReturnType = typename Traits::ReturnType;
            using Arg0 = typename Traits::template Arg<0>::Type;

            static constexpr bool Value = (Traits::NumArgs == 1)
                and TIsSameType<ReturnType, void>::Value
        		and TIsBaseClassOf<FWidget, std::remove_cvref_t<Arg0>>::Value
        		and std::is_reference_v<Arg0>;
        };

    private:


        FUSION_WIDGET;
    };

    struct FForEach : FWidgetBuilder
    {
        FForEach(int loopCount, Delegate<FWidget& (int index)> lambda)
            : loopCount(loopCount), lambda(lambda)
        {}

        int loopCount;
        Delegate<FWidget& (int index)> lambda;

        void Build(FWidget* parent)
        {
	        if (loopCount > 0 && lambda.IsValid())
	        {
		        for (int i = 0; i < loopCount; ++i)
		        {
                    parent->AddChild(&lambda(i));
		        }
	        }
        }
    };

    template<typename TWidget> requires (!TIsAbstract<TWidget>::Value) && TIsBaseClassOf<FWidget, TWidget>::Value
    TWidget& GetDefaultWidget()
    {
        CE_ASSERT(TWidget::Type()->CanBeInstantiated(), "The given widget class ({}) is an abstract class!", TWidget::Type()->GetName());
        return *GetMutableDefaults<TWidget>();
    }
    
} // namespace CE

#include "FWidget.rtti.h"
