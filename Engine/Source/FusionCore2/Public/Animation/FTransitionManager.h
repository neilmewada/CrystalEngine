#pragma once

namespace CE
{
    class FWidget;

    struct FTweenTransition
    {
        f32          duration = 0.3f;
        FEasingCurve easing = FEasingType::EaseInOutCubic;
        f32          delay = 0.f;

        FTweenTransition& Duration(f32 d) { duration = d; return *this; }
        FTweenTransition& Easing(FEasingCurve e) { easing = e; return *this; }
        FTweenTransition& Delay(f32 d) { delay = d; return *this; }
    };

    struct FSpringTransition
    {
        f32 stiffness = 200.f;
        f32 damping = 20.f;
        f32 settleEpsilon = 0.001f;
        f32 delay = 0.f;

        FSpringTransition& Stiffness(f32 s) { stiffness = s; return *this; }
        FSpringTransition& Damping(f32 d) { damping = d; return *this; }
        FSpringTransition& SettleEpsilon(f32 e) { settleEpsilon = e; return *this; }
        FSpringTransition& Delay(f32 d) { delay = d; return *this; }
    };

    template<typename T>
    concept TransitionTypeName = TContainsType<T, FTweenTransition, FSpringTransition>::Value;

    class IFTransitionEntry : public IntrusiveBase
    {
    public:

        virtual ~IFTransitionEntry() = default;

        virtual TypeId GetPropertyTypeId() = 0;

    };

    template<typename T>
    class FTransitionEntry : public IFTransitionEntry
    {
    public:

        TypeId GetPropertyTypeId() override
        {
            return TYPEID(T);
        }

        ClassType* registeredClass = nullptr;

        std::function<void(FWidget* widget, const T& value)> applyTransition;
    };

    class FUSIONCORE_API FTransitionManager
    {
        CE_NO_COPY_MOVE(FTransitionManager);
    public:

        static FTransitionManager& Get();

        template<WidgetClassType TWidget, typename T, TransitionTypeName TTransition>
        FTransitionManager& Register(Name propertyName,
            T (TWidget::* getter)() const,
            void (TWidget::* animateSetter)(const T&),
            const TTransition& transition)
        {
            Ptr<FTransitionEntry<T>> entry = new FTransitionEntry<T>;

            entry->registeredClass = TWidget::StaticClass();

            entry->applyTransition = [propertyName, getter, animateSetter, transition](FWidget* instance, const T& value)
            {
                // TODO: finish this

                TWidget* widget = static_cast<TWidget*>(instance);

                if (widget->GetParentWidget().IsNull() && widget->GetParentSurface().IsNull())
                {
                    (static_cast<TWidget*>(widget)->*animateSetter)(value);
                    return;
                }

                if constexpr (TIsSameType<TTransition, FTweenTransition>::Value)
                {
                    FAnimate::Tween<TWidget, T>(propertyName, widget, getter, animateSetter)
                    .To(value)
                    .Delay(transition.delay)
                    .Duration(transition.duration)
                    .Easing(transition.easing)
                    .Loop(FAnimationLoopMode::Once)
                    .Play();
                }
                else if constexpr (TIsSameType<TTransition, FSpringTransition>::Value)
                {
                    FAnimate::Spring<TWidget, T>(propertyName, widget, getter, animateSetter)
                	.Target(value)
                    .Stiffness(transition.stiffness)
                    .Damping(transition.damping)
                    .SettleEpsilon(transition.settleEpsilon)
                    .Delay(transition.delay)
                	.Play();
                }
            };

            transitionEntries[{entry->registeredClass, propertyName}] = static_cast<IFTransitionEntry*>(entry.Get());

            return *this;
        }

        template<WidgetClassType TWidget, typename T>
        bool ApplyTransition(TWidget* widget, const CE::Name& propertyName, const T& value)
        {
            if (!widget)
                return false;

            ZoneScoped;

            Pair<ClassType*, Name> pair = { TWidget::StaticClass(), propertyName };
            static const ClassType* objectClass = Object::StaticClass();

            while (pair.first != nullptr && pair.first != objectClass)
            {
                auto it = transitionEntries.Find(pair);
                if (it != transitionEntries.End() && it->second && it->second->GetPropertyTypeId() == TYPEID(T))
                {
                    FTransitionEntry<T>* entry = static_cast<FTransitionEntry<T>*>(it->second.Get());
                    entry->applyTransition(widget, value);
                    return true;
                }

                pair.first = pair.first->GetSuperClass(0);
            }

            return false;
        }

    fusioncore_internal:

        void Shutdown();

    private:

        FTransitionManager() = default;

        HashMap<Pair<ClassType*, Name>, Ptr<IFTransitionEntry>> transitionEntries;
    };
    
}
