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

        Ref<FAnimation> Play()
        {
            Ref<FAnimation> anim = Build(owner, name);
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, name);
            return anim;
        }

        Ref<FAnimation> Play(Name slot)
        {
            this->name = slot;
            Ref<FAnimation> anim = Build(owner, slot);
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, slot);
            return anim;
        }

    private:

        Ref<FAnimation> Build(Ref<Object> owner, Name name)
        {
            if (!owner.IsValid())
                return nullptr;

            T from = hasFromGetter ? fromGetter() : fromValue;

            Ref<FTweenAnimation> anim = CreateObject<FTweenAnimation>(owner.Get(), FixObjectName(name.GetString()));
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

        Name                          name;
        Ref<Object>                   owner;
        std::function<void(const T&)> setter;
        std::function<T()>            fromGetter;
        T                             fromValue{};
        T                             toValue{};
        bool                          hasFromGetter = false;
        FEasingCurve                  easing = FEasingType::EaseInOutCubic;
        f32                           duration = 0.3f;
        f32                           delay = 0.0f;
        FAnimationLoopMode            loopMode = FAnimationLoopMode::Once;
        std::function<void()>         onComplete;
        bool                          onCompleteValid = false;

        friend class FAnimate;
    };

    template<typename T>
    class FSpringBuilder
    {
    public:

	    FSpringBuilder& Target(T t)           { target = MoveTemp(t); targetValid = true; return *this; }
	    FSpringBuilder& Stiffness(f32 s)      { stiffness     = s; return *this; }
	    FSpringBuilder& Damping(f32 d)        { damping       = d; return *this; }
	    FSpringBuilder& SettleEpsilon(f32 e)  { settleEpsilon = e; return *this; }
	    FSpringBuilder& Delay(f32 d)          { delay         = d; return *this; }
	    FSpringBuilder& Owner(Ref<Object> o)  { owner = MoveTemp(o); return *this; }

        FSpringBuilder& OnComplete(std::function<void()> cb)
        {
            onComplete = MoveTemp(cb);
            onCompleteValid = true;
            return *this;
        }

        Ref<FAnimation> Play()
        {
            Ref<FAnimation> anim = Build(name);
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, name);
            return anim;
        }

        Ref<FAnimation> Play(Name slot)
        {
            name = slot;
            Ref<FAnimation> anim = Build(slot);
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, name);
            return anim;
        }

    private:

        Ref<FAnimation> Build(Name name)
        {
            if (!targetValid || !owner.IsValid())
            {
	            return nullptr;
            }

            const T current = getter();

            Ref<FSpringAnimation> anim = CreateObject<FSpringAnimation>(owner.Get(), FixObjectName(name.GetString()));
            anim->stiffness     = stiffness;
            anim->damping       = damping;
            anim->settleEpsilon = settleEpsilon;
            anim->SetSpring(current, target, MoveTemp(setter));
            anim->SetDelay(delay);

            if (onCompleteValid)
                anim->OnComplete() += onComplete;

            return anim;
        }

        Name                          name;
        Ref<Object>                   owner;
        std::function<void(const T&)> setter;
        std::function<T()>            getter;
        T                             target{};
        bool                          targetValid = false;
        f32                           stiffness     = 200.0f;
        f32                           damping       =  20.0f;
        f32                           settleEpsilon =   0.001f;
        f32                           delay         =   0.0f;
        std::function<void()>         onComplete;
        bool                          onCompleteValid = false;

        friend class FAnimate;
    };

    class FAnimate
    {
    public:


        template<WidgetClassType TWidgetType, typename T>
        static FTweenBuilder<T> Tween(Name name, TWidgetType* target, T (TWidgetType::*getter)() const, void (TWidgetType::*setter)(const T&))
        {
            FTweenBuilder<T> builder{};
            builder.name = name;
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            builder.fromGetter = [target, getter]() -> T { return (target->*getter)(); };
            builder.owner = target;
            builder.hasFromGetter = true;
            return builder;
        }

        template<WidgetClassType TWidgetType, typename T>
        static FTweenBuilder<T> Tween(Name name, TWidgetType* target, T(TWidgetType::* getter)() const, void (TWidgetType::*setter)(T))
        {
            FTweenBuilder<T> builder{};
            builder.name = name;
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            builder.fromGetter = [target, getter]() -> T { return (target->*getter)(); };
            builder.owner = target;
            builder.hasFromGetter = true;
            return builder;
        }

        // Spring — widget pointer + method pointers (owner is captured automatically)
        template<WidgetClassType TWidgetType, typename T>
        static FSpringBuilder<T> Spring(Name name, TWidgetType* target,
                                         T (TWidgetType::*getter)() const,
                                         void (TWidgetType::*setter)(const T&))
        {
            FSpringBuilder<T> builder{};
            builder.name = name;
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            builder.getter = [target, getter]() -> T { return (target->*getter)(); };
            builder.owner  = target;
            return builder;
        }

        template<WidgetClassType TWidgetType, typename T>
        static FSpringBuilder<T> Spring(Name name, TWidgetType* target,
                                         T (TWidgetType::*getter)() const,
                                         void (TWidgetType::*setter)(T))
        {
            FSpringBuilder<T> builder{};
            builder.name = name;
            builder.setter = [target, setter](const T& v) { (target->*setter)(v); };
            builder.getter = [target, getter]() -> T { return (target->*getter)(); };
            builder.owner  = target;
            return builder;
        }
    };

}