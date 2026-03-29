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

        Ref<FAnimation> Build(Object* outer)
        {
            T from = hasFromGetter ? fromGetter() : fromValue;

            Ref<FTweenAnimation> anim = CreateObject<FTweenAnimation>(outer, FixObjectName(name.GetString()));
            anim->AssignOwner(owner);
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

        Ref<FAnimation> Play()
        {
            if (!owner.IsValid())
                return nullptr;
            Ref<FAnimation> anim = Build(owner.Get());
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, name);
            return anim;
        }

        Ref<FAnimation> Play(Name slot)
        {
            if (!owner.IsValid())
                return nullptr;
            this->name = slot;
            Ref<FAnimation> anim = Build(owner.Get());
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, slot);
            return anim;
        }

    private:

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

        Ref<FAnimation> Build(Object* outer)
        {
            if (!targetValid)
                return nullptr;

            const T current = getter();

            Ref<FSpringAnimation> anim = CreateObject<FSpringAnimation>(outer, FixObjectName(name.GetString()));
            anim->AssignOwner(owner);
            anim->stiffness     = stiffness;
            anim->damping       = damping;
            anim->settleEpsilon = settleEpsilon;
            anim->SetSpring(current, target, MoveTemp(setter));
            anim->SetDelay(delay);

            if (onCompleteValid)
                anim->OnComplete() += onComplete;

            return anim;
        }

        Ref<FAnimation> Play()
        {
            if (!targetValid || !owner.IsValid())
                return nullptr;
            Ref<FAnimation> anim = Build(owner.Get());
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, name);
            return anim;
        }

        Ref<FAnimation> Play(Name slot)
        {
            if (!targetValid || !owner.IsValid())
                return nullptr;
            name = slot;
            Ref<FAnimation> anim = Build(owner.Get());
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, owner, name);
            return anim;
        }

    private:

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

    class FSequenceBuilder
    {
        using StepFactory = std::function<Ref<FAnimation>(Object*)>;

        struct Step
        {
            StepFactory factory;
            f32 delayBefore = 0.f;
        };

    public:

        template<typename TBuilder>
        FSequenceBuilder& Then(TBuilder childBuilder, f32 delayBefore = 0.f)
        {
            steps.Add({
                [b = std::move(childBuilder)](Object* outer) mutable -> Ref<FAnimation>
                {
                    return b.Build(outer);
                },
                delayBefore
            });
            return *this;
        }

        FSequenceBuilder& Then(Ref<FAnimation> anim, f32 delayBefore = 0.f)
        {
            steps.Add({ [anim](Object*) -> Ref<FAnimation> { return anim; }, delayBefore });
            return *this;
        }

        FSequenceBuilder& Loop(FAnimationLoopMode m) { loopMode = m; return *this; }
        FSequenceBuilder& Delay(f32 d) { delay = d; return *this; }

        FSequenceBuilder& OnComplete(std::function<void()> cb)
        {
            onComplete = std::move(cb);
            onCompleteValid = true;
            return *this;
        }

        Ref<FAnimation> Build(Object* outer)
        {
            Ref<FSequenceAnimation> seq = CreateObject<FSequenceAnimation>(outer, "Sequence");
            seq->AssignOwner(anchor);

            for (int i = 0; i < steps.GetSize(); i++)
            {
                Ref<FAnimation> child = steps[i].factory(seq.Get());
                if (child.IsValid())
                    seq->Add(child, steps[i].delayBefore);
            }

            seq->loopMode = loopMode;
            seq->SetDelay(delay);
            if (onCompleteValid)
                seq->OnComplete() += onComplete;

            return seq;
        }

        Ref<FAnimation> Play()
        {
            if (!anchor.IsValid())
                return nullptr;
            Ref<FAnimation> anim = Build(anchor.Get());
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, anchor, slot);
            return anim;
        }

    private:

        Array<Step>            steps;
        Ref<Object>            anchor;
        Name                   slot;
        FAnimationLoopMode     loopMode        = FAnimationLoopMode::Once;
        f32                    delay           = 0.f;
        std::function<void()>  onComplete;
        bool                   onCompleteValid = false;

        friend class FAnimate;
    };

    class FParallelBuilder
    {
        using StepFactory = std::function<Ref<FAnimation>(Object*)>;

        struct Step
        {
            StepFactory factory;
            f32 delayBefore = 0.f;
        };

    public:

        template<typename TBuilder>
        FParallelBuilder& With(TBuilder childBuilder, f32 delayBefore = 0.f)
        {
            steps.Add({
                [b = std::move(childBuilder)](Object* outer) mutable -> Ref<FAnimation>
                {
                    return b.Build(outer);
                },
                delayBefore
            });
            return *this;
        }

        FParallelBuilder& With(Ref<FAnimation> anim, f32 delayBefore = 0.f)
        {
            steps.Add({ [anim](Object*) -> Ref<FAnimation> { return anim; }, delayBefore });
            return *this;
        }

        FParallelBuilder& Loop(FAnimationLoopMode m) { loopMode = m; return *this; }
        FParallelBuilder& Delay(f32 d) { delay = d; return *this; }

        FParallelBuilder& OnComplete(std::function<void()> cb)
        {
            onComplete = std::move(cb);
            onCompleteValid = true;
            return *this;
        }

        Ref<FAnimation> Build(Object* outer)
        {
            Ref<FParallelAnimation> par = CreateObject<FParallelAnimation>(outer, "Parallel");
            par->AssignOwner(anchor);

            for (int i = 0; i < steps.GetSize(); i++)
            {
                Ref<FAnimation> child = steps[i].factory(par.Get());
                if (child.IsValid())
                    par->Add(child, steps[i].delayBefore);
            }

            par->loopMode = loopMode;
            par->SetDelay(delay);
            if (onCompleteValid)
                par->OnComplete() += onComplete;

            return par;
        }

        Ref<FAnimation> Play()
        {
            if (!anchor.IsValid())
                return nullptr;
            Ref<FAnimation> anim = Build(anchor.Get());
            FApplication::Get()->GetService<FAnimationService>()->Play(anim, anchor, slot);
            return anim;
        }

    private:

        Array<Step>            steps;
        Ref<Object>            anchor;
        Name                   slot;
        FAnimationLoopMode     loopMode        = FAnimationLoopMode::Once;
        f32                    delay           = 0.f;
        std::function<void()>  onComplete;
        bool                   onCompleteValid = false;

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

        // Top-level sequence: registered with the animation service under the given slot.
        static FSequenceBuilder Sequence(Object* anchor, Name slot)
        {
            FSequenceBuilder builder{};
            builder.anchor = anchor;
            builder.slot = slot;
            return builder;
        }

        // Top-level sequence: registered with the animation service under the given slot.
        static FSequenceBuilder Sequence(Ref<Object> anchor, Name slot)
        {
            FSequenceBuilder builder{};
            builder.anchor = anchor;
            builder.slot = slot;
            return builder;
        }

        // Nested sequence: owned by anchor but not registered with the service directly.
        static FSequenceBuilder Sequence(Object* anchor)
        {
            FSequenceBuilder builder{};
            builder.anchor = anchor;
            return builder;
        }

        // Top-level parallel: registered with the animation service under the given slot.
        static FParallelBuilder Parallel(Object* anchor, Name slot)
        {
            FParallelBuilder builder{};
            builder.anchor = anchor;
            builder.slot = slot;
            return builder;
        }

        // Nested parallel: owned by anchor but not registered with the service directly.
        static FParallelBuilder Parallel(Object* anchor)
        {
            FParallelBuilder builder{};
            builder.anchor = anchor;
            return builder;
        }
    };

}
