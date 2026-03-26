#pragma once

namespace CE
{
    // Extracts T from a setter callable of the form void(const T&)
    template<typename>
    struct FSetterArgType;

    template<typename T>
    struct FSetterArgType<void(*)(const T&)> { using Type = T; };

    template<typename C, typename T>
    struct FSetterArgType<void(C::*)(const T&)> { using Type = T; };

    template<typename C, typename T>
    struct FSetterArgType<void(C::*)(const T&) const> { using Type = T; };

    // Fallback for lambdas/functors — deduces from operator()
    template<typename C>
    struct FSetterArgType : FSetterArgType<decltype(&C::operator())> {};



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

        
        template<typename WidgetType, typename T>
        static FTweenBuilder<T> Tween(WidgetType* target, void (WidgetType::*setter)(const T&))
        {
            FTweenBuilder<T> builder{};
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            return builder;
        }

        template<typename WidgetType, typename T>
        static FTweenBuilder<T> Tween(WidgetType* target, void (WidgetType::*setter)(T))
        {
            FTweenBuilder<T> builder{};
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            return builder;
        }

    };

}