#pragma once

namespace CE
{

    template<typename T>
    class FTweenBuilder
    {
    public:

        FTweenBuilder& From(T from)
        {
            fromValue = MoveTemp(from);
            hasFromGetter = false;
            return *this;
        }

        FTweenBuilder& From(std::function<T()> from)
        {
            fromGetter = MoveTemp(from);
            hasFromGetter = true;
            return *this;
        }

        FTweenBuilder& To(T value) { toValue = MoveTemp(value); return *this; }
        FTweenBuilder& Duration(f32 d) { duration = d; return *this; }
        FTweenBuilder& Easing(FEasingCurve e) { easing = e; return *this; }
        FTweenBuilder& Delay(f32 d) { delay = d; return *this; }
        FTweenBuilder& Loop(FAnimationLoopMode m) { loopMode = m; return *this; }

        FTweenBuilder& OnComplete(std::function<void()> cb)
        {
            onComplete = MoveTemp(cb);
            onCompleteValid = true;
            return *this;
        }

        Ref<FAnimation> Play(Ref<Object> owner, Name slot)
        {
            Ref<FAnimation> anim = Build(owner);
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, slot);
            return anim;
        }

    private:

        Ref<FAnimation> Build(Ref<Object> owner)
        {
            T from = hasFromGetter ? fromGetter() : fromValue;

            Ref<FTweenAnimation> anim = CreateObject<FTweenAnimation>(owner.Get(), "TweenAnimation");
            anim->SetInterpolator(MoveTemp(from), toValue, easing, setter);
            anim->SetDelay(delay);
            anim->duration = duration;
            anim->loopMode = loopMode;

            if (onCompleteValid)
            {
	            anim->OnComplete() += onComplete;
            }

            return anim;
        }

        std::function<void(const T&)> setter;
        std::function<T()>            fromGetter;
        T                             fromValue{};
        T                             toValue{};
        bool                          hasFromGetter = false;
        FEasingCurve                  easing{};
        f32                           duration = 0.3f;
        f32                           delay = 0.0f;
        FAnimationLoopMode            loopMode = FAnimationLoopMode::Once;
        std::function<void()>         onComplete;
        bool                          onCompleteValid;

        friend class FAnimate;
    };

    class FAnimate
    {
    public:

        
        template<WidgetClassType TWidgetType, typename T>
        static FTweenBuilder<T> Tween(TWidgetType* target, void (TWidgetType::*setter)(const T&))
        {
            FTweenBuilder<T> builder{};
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            return builder;
        }

        template<WidgetClassType TWidgetType, typename T>
        static FTweenBuilder<T> Tween(TWidgetType* target, void (TWidgetType::*setter)(T))
        {
            FTweenBuilder<T> builder{};
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            return builder;
        }

        template<WidgetClassType TWidgetType, typename T>
        static FTweenBuilder<T> Tween(Ref<TWidgetType> target, void (TWidgetType::* setter)(const T&))
        {
            FTweenBuilder<T> builder{};
            builder.setter = [target, setter](const T& v) { (target.Get()->*setter)(v); };
            return builder;
        }

        template<WidgetClassType TWidgetType, typename T>
        static FTweenBuilder<T> Tween(Ref<TWidgetType> target, void (TWidgetType::* setter)(T))
        {
            FTweenBuilder<T> builder{};
            builder.setter = [target, setter](const T& v) { (target.Get()->*setter)(v); };
            return builder;
        }
    };

}