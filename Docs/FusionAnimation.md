# FusionCore2 Animation System

## Overview

The animation system is built around three concerns kept deliberately separate:

- **Value Layer** — how to interpolate between two values of a given type (`FAnimatable<T>`)
- **Animation Layer** — how to advance time and produce interpolated values (`FAnimation` hierarchy)
- **Binding Layer** — how to apply values to widget properties (setter lambdas + `FTransitionManager`)

`FAnimationService` drives all active top-level animations each frame during the `PreUpdateSurfaces` tick phase, before layout and rendering occur.

---

## Feature Set

| Feature | Class(es) |
|---|---|
| Property tweening (A→B) | `FTweenAnimation` |
| Easing functions | `FEasingCurve`, `FEasingType` |
| Keyframe tracks | `FKeyframeAnimation` |
| Sequential composition | `FSequenceAnimation` |
| Parallel composition | `FParallelAnimation` |
| Playback control | `FAnimation` base (Play, Pause, Resume, Stop, Restart, loop modes) |
| Animation events | `OnStart`, `OnComplete`, `OnUpdate` delegates |
| Transition animations | `FTransitionManager` (per-widget, opt-in) |
| Spring physics | `FSpringAnimation` |
| Interruptibility | Built into tweens (set `from` to current value) and springs (update `target`) |
| Widget property binding | Setter lambdas on all animation types; `FAnimate` builder helpers |
| Transform animation | `FTweenAnimation` with `FAnimatable<FAffineTransform>` (decompose-lerp-recompose) |
| Serialization | `FIELD()` on structural metadata; setter/keyframe data are runtime-only |

---

## Layer 1 — `FAnimatable<T>`

A non-Object template trait that defines how to interpolate between two values of type `T`. The internal `FInterpolator<T>` (inside `FTweenAnimation`) and `FKeyframeTrack<T>` (inside `FKeyframeAnimation`) use this trait. Users can specialize it for any custom type — `FGradient`, `FBrush`, or anything else — and it will work with all animation classes automatically.

```cpp
template<typename T>
struct FAnimatable
{
    static T Lerp(const T& a, const T& b, f32 t);
    static T LerpUnclamped(const T& a, const T& b, f32 t);
    static T Identity(); // Identity/zero value — needed for spring initial velocity
};

// Built-in specializations:
template<> struct FAnimatable<Color>;
template<> struct FAnimatable<FAffineTransform>;
// Default template handles: f32, Vec2, Vec3, Vec4, and any type with +/- operators
```

`FAffineTransform` interpolation decomposes both transforms into `{ translation, rotation, scale, shear }`, lerps each component individually, then recomposes. This avoids the artifacts that occur when linearly interpolating matrix coefficients directly.

---

## Layer 2 — `FEasingCurve`

A plain serializable struct (not a `CE::Object`) representing a timing curve. Evaluated via `Evaluate(f32 t) -> f32` where both input and output are in `[0, 1]` (or slightly outside for overshoot curves like `Back` and `Elastic`).

```cpp
ENUM()
enum class FEasingType
{
    Linear,
    EaseInQuad,    EaseOutQuad,    EaseInOutQuad,
    EaseInCubic,   EaseOutCubic,   EaseInOutCubic,
    EaseInQuart,   EaseOutQuart,   EaseInOutQuart,
    EaseInSine,    EaseOutSine,    EaseInOutSine,
    EaseInExpo,    EaseOutExpo,    EaseInOutExpo,
    EaseInCirc,    EaseOutCirc,    EaseInOutCirc,
    EaseInBack,    EaseOutBack,    EaseInOutBack,
    EaseInBounce,  EaseOutBounce,  EaseInOutBounce,
    EaseInElastic, EaseOutElastic, EaseInOutElastic,
    CubicBezier,   // Uses controlPoint1 / controlPoint2
};

STRUCT()
struct FEasingCurve
{
    FIELD() FEasingType type          = FEasingType::Linear;
    FIELD() Vec2        controlPoint1 = {};  // Only used for CubicBezier
    FIELD() Vec2        controlPoint2 = {};  // Only used for CubicBezier

    f32 Evaluate(f32 t) const;
};
```

---

## Layer 3 — `FAnimation` (Base Class)

All animation types derive from this. It owns playback state, timing, loop behaviour, and fires events. Subclasses implement `Apply(f32 normalizedTime)` to write the actual value. `FSequenceAnimation` and `FParallelAnimation` override `Tick` entirely and leave `Apply` as a no-op.

```cpp
CLASS(Abstract)
class FAnimation : public CE::Object
```

### Playback

```
void Play()     — Start from beginning (respects delay)
void Pause()    — Freeze at current position
void Resume()   — Continue from where paused
void Stop()     — Reset to beginning, fire no events
void Restart()  — Stop then Play
```

### Serializable Fields

| Field | Type | Default | Description |
|---|---|---|---|
| `duration` | `f32` | `0` | Total duration in seconds |
| `delay` | `f32` | `0.0` | Wait before starting |
| `speed` | `f32` | `1.0` | Playback multiplier; negative plays in reverse |
| `loopMode` | `FAnimationLoopMode` | `Once` | `Once`, `Loop`, or `PingPong` |

### Events (runtime only, not serialized)

| Event | Signature | Fires when |
|---|---|---|
| `OnStart` | `void()` | Animation begins playing (after delay) |
| `OnComplete` | `void()` | Final iteration finishes |
| `OnUpdate` | `void(f32 t)` | Every tick; receives normalized time `[0,1]` |

### Subclass Contract

```cpp
// Subclasses implement this to apply the interpolated value.
// t is normalized time [0,1], already accounting for speed/reverse direction.
// Easing is applied by the subclass (e.g. inside FInterpolator<T>::Apply).
virtual void Apply(f32 normalizedTime) = 0;

// FSequenceAnimation and FParallelAnimation override the full Tick instead.
// FSpringAnimation also overrides Tick (no fixed duration).
virtual void Tick(f32 deltaTime);
```

---

## Layer 4 — Concrete Animation Types

> **Note:** Template classes cannot derive from `CE::Object` in this engine. Type-generic behaviour is achieved via non-Object internal interfaces (`IFInterpolator`, `IFKeyframeTrack`, `IFSpringState`). `FAnimatable<T>` is a pure utility trait — never a `CE::Object`.

---

### `FTweenAnimation` — Type-Erased Tween

Simple A→B interpolation over a fixed duration. Works with **any type** that has an `FAnimatable<T>` specialization.

```cpp
// Internal interface — not a CE::Object, never exposed directly
class IFInterpolator
{
public:
    virtual void Apply(f32 t) = 0;
    virtual ~IFInterpolator() = default;
};

template<typename T>
class FInterpolator : public IFInterpolator
{
    T from;
    T to;
    FEasingCurve easing;
    Delegate<void(const T&)> setter;
public:
    void Apply(f32 t) override
    {
        if (setter.IsValid())
            setter(FAnimatable<T>::Lerp(from, to, easing.Evaluate(t)));
    }
};

CLASS()
class FTweenAnimation : public FAnimation
{
    CE_CLASS(FTweenAnimation, FAnimation)

    template<typename T>
    void SetInterpolator(T from, T to, FEasingCurve easing, Delegate<void(const T&)> setter);

    void Apply(f32 normalizedTime) override; // Delegates to interpolator->Apply(normalizedTime)

private:
    UniquePtr<IFInterpolator> interpolator; // Runtime-only — holds from, to, easing, setter
};
```

**Interruptibility:** When interrupting a playing tween, read the widget's current rendered value and pass it as `from` to the new tween before calling `Play()`. This ensures the animation starts from the actual in-motion position.

---

### `FSpringAnimation` — Type-Erased Spring

Physics-based animation driven by a damped spring. Works with any type that has an `FAnimatable<T>` specialization. No fixed duration — runs until velocity and displacement both drop below `settleEpsilon`.

```cpp
// Internal interface — not a CE::Object
class IFSpringState
{
public:
    virtual void Tick(f32 dt) = 0;
    virtual bool HasSettled() const = 0;
    virtual ~IFSpringState() = default;
};

template<typename T>
class FSpringState : public IFSpringState
{
    T   current;
    T   velocity;
    T   target;
    f32 stiffness, damping, settleEpsilon;
    Delegate<void(const T&)> setter;
public:
    void Tick(f32 dt) override; // Semi-implicit Euler integration
    bool HasSettled() const override;
};

CLASS()
class FSpringAnimation : public FAnimation
{
    CE_CLASS(FSpringAnimation, FAnimation)

    FIELD() f32 stiffness     = 200.0f;
    FIELD() f32 damping       =  20.0f;
    FIELD() f32 settleEpsilon =   0.001f;

    void Tick(f32 dt) override; // Delegates to springState; fires OnComplete when settled
    void Apply(f32) override {} // Unused — spring drives its own apply in Tick

private:
    UniquePtr<IFSpringState> springState; // Runtime-only — holds current, velocity, setter
};
```

Spring integration per tick:

```
force    = stiffness * (target - current) - damping * velocity
velocity += force * dt
current  += velocity * dt
```

`OnComplete` fires once the spring settles. **Interruptibility is built-in:** update `target` at any time while the spring is playing and it naturally redirects from its current velocity.

---

### `FKeyframeAnimation` — Type-Erased Keyframe Track

Multi-waypoint animation over a fixed duration. Works with **any type** that has an `FAnimatable<T>` specialization. Keyframes are stored as a flat sorted array inside the type-erased track.

```cpp
// Internal interface — not a CE::Object, never exposed directly
class IFKeyframeTrack
{
public:
    virtual void Apply(f32 normalizedTime) = 0;
    virtual ~IFKeyframeTrack() = default;
};

template<typename T>
class FKeyframeTrack : public IFKeyframeTrack
{
    struct FKeyframe { f32 time; T value; FEasingCurve easing; };
    Array<FKeyframe>         keyframes; // sorted by time on first Apply after AddKey
    Delegate<void(const T&)> setter;
public:
    void AddKey(f32 time, T value, FEasingCurve easing = {});
    void Apply(f32 t) override; // Finds surrounding keyframes, lerps with segment easing
};

CLASS()
class FKeyframeAnimation : public FAnimation
{
    CE_CLASS(FKeyframeAnimation, FAnimation)

    template<typename T>
    FKeyframeTrack<T>* SetTrack(Delegate<void(const T&)> setter); // Returns track for AddKey calls

    void Apply(f32 normalizedTime) override; // Delegates to track->Apply(normalizedTime)

private:
    UniquePtr<IFKeyframeTrack> track; // Runtime-only — holds keyframe data and setter
};
```

Loop, PingPong, and Speed all work automatically via the base class — the track just receives a normalized time on each `Apply` call.

Serialization of keyframe data for asset pipelines is handled by higher-level engine modules outside of FusionCore2, which reconstruct the `FKeyframeAnimation` at load time.

---

### `FSequenceAnimation` — Chain

Plays children one after another. Fires `OnComplete` when the last child finishes. Overrides `Tick` entirely — `Apply` is a no-op.

```cpp
CLASS()
class FSequenceAnimation : public FAnimation
{
    CE_CLASS(FSequenceAnimation, FAnimation)
public:
    FIELD() Array<Ref<FAnimation>> children;

    void Add(Ref<FAnimation> animation);
    void Add(Ref<FAnimation> animation, f32 delayBefore); // Calls SetDelay on the child

    void Tick(f32 deltaTime) override;
    void Apply(f32 normalizedTime) override {} // no-op — sequence drives children in Tick
};
```

Uses the inherited `reversed` flag to iterate children in reverse order during `PingPong` passes.

---

### `FParallelAnimation` — Simultaneous

Plays all children simultaneously. Fires `OnComplete` when the last child finishes. Overrides `Tick` entirely — `Apply` is a no-op.

```cpp
CLASS()
class FParallelAnimation : public FAnimation
{
    CE_CLASS(FParallelAnimation, FAnimation)
public:
    FIELD() Array<Ref<FAnimation>> children;

    void Add(Ref<FAnimation> animation);
    void Add(Ref<FAnimation> animation, f32 delayBefore); // Calls SetDelay on the child

    void Tick(f32 deltaTime) override;
    void Apply(f32 normalizedTime) override {} // no-op — parallel drives children in Tick
};
```

**Stagger** is achieved by setting `delay` on individual children via the two-argument `Add()`.

---

## Layer 5 — `FTransitionManager`

An optional per-widget component that provides CSS-transition-style automatic animations. When a registered property changes, the manager creates a new `FTweenAnimation` from the current animated value to the new target — no manual animation setup required.

### Property Macro

Properties that support transitions are declared with `FUSION_TRANSITION_PROPERTY` instead of `FUSION_PROPERTY`. This macro generates **two setters**:

```cpp
// Normal setter — called by user code
void Opacity(f32 v) {
    m_Opacity = v;
    if (transitionManager)
        transitionManager->OnPropertyChanged("Opacity");  // must come first
    OnFusionPropertyModified("Opacity");
}

// Transition setter — called only by FTransitionManager's tween each frame
void Transition_Opacity(f32 v) {
    m_Opacity = v;
    OnFusionPropertyModified("Opacity");  // fires normally so layout/render stays in sync
    // does NOT call transitionManager->OnPropertyChanged — avoids infinite recursion
}
```

### Call Order

When `Opacity(0.5f)` is called:

```
1. m_Opacity = 0.5f                              // backing var = new target
2. transitionManager->OnPropertyChanged("Opacity")
       → captures new target (0.5f) from m_Opacity
       → writes stored animated value back to m_Opacity   // revert to current visual
       → starts/replaces tween: from=animated, to=0.5f
       → tween uses Transition_Opacity as its setter
3. OnFusionPropertyModified("Opacity")           // reads m_Opacity = current visual value ✓
```

`OnFusionPropertyModified` always sees the value the widget is currently displaying, whether or not a transition is in progress. As the tween ticks each frame it calls `Transition_Opacity(v)`, which fires `OnFusionPropertyModified` normally so layout and rendering stay in sync throughout the animation.

### API

```cpp
struct FTransitionSpec
{
    FIELD() f32          duration = 0.2f;
    FIELD() f32          delay    = 0.0f;
    FIELD() FEasingCurve easing;
};

class FTransitionManager
{
public:
    // Register a transition for a named property.
    // transitionSetter is the Transition_PropertyName setter — writes backing var
    // and fires OnFusionPropertyModified, but does NOT re-enter OnPropertyChanged.
    template<typename T>
    void RegisterTransition(Name propertyName, FTransitionSpec spec,
                            Delegate<void(const T&)> transitionSetter);

    // Called by the normal property setter (before OnFusionPropertyModified).
    // Captures the new target from the backing var, reverts backing var to the
    // current animated value, then starts or replaces the active tween.
    void OnPropertyChanged(Name propertyName);

    // Tick all active transition tweens. Called by the owning FWidget each frame.
    void Tick(f32 deltaTime);
};
```

`FWidget` holds an optional `FTransitionManager* transitionManager` (null by default — zero overhead for widgets that don't use transitions). The normal property setter checks `if (transitionManager)` before calling `OnPropertyChanged`.

### Interruptibility

When `OnPropertyChanged` fires for a property that already has an active transition, the manager's stored animated value is whatever the tween last wrote — the current in-motion position. The new tween starts from there toward the new target, so motion is always continuous with no jump.

---

## Layer 6 — `FAnimationService`

Manages all active root-level animations. Composition children (sequences, parallels) tick their own children — the service only holds top-level entries.

```cpp
CLASS()
class FAnimationService : public FService
{
    CE_CLASS(FAnimationService, FService)
public:
    Ref<FAnimation> Play(Ref<FAnimation> animation);  // Register and start
    void Stop(Ref<FAnimation> animation);             // Unregister and stop

    void TickService(FServiceTickPhase tickPhase) override;
    // Active during: PreUpdateSurfaces
};
```

Completed non-looping animations are removed automatically. Pending add/remove lists prevent mutation during the tick loop.

`FTransitionManager` ticks its animations independently (called through `FWidget`) and does not register with `FAnimationService` — transitions are widget-local resources.

---

## Fluent Builder — `FAnimate`

A static helper that provides a clean authoring API. Internally constructs the correct `FInterpolator<T>` or `FSpringState<T>` based on the types passed to the builder.

```cpp
// Tween — T inferred from from/to values
FAnimate::Tween([widget](f32 v){ widget->Opacity(v); }, 0.0f, 1.0f)
    .Duration(0.3f)
    .Easing(FEasingCurve(FEasingType::EaseOutCubic))
    .Play();

// Works identically for any animatable type — FGradient, FBrush, etc.
FAnimate::Tween([widget](FGradient g){ widget->Background(g); }, fromGradient, toGradient)
    .Duration(0.5f)
    .Play();

// Keyframes — builds a FKeyframeAnimation with a type-erased FKeyframeTrack<T>
FAnimate::Keyframes([widget](f32 v){ widget->Opacity(v); })
    .Key(0.0f, 0.0f)
    .Key(0.5f, 1.0f, FEasingCurve(FEasingType::EaseOutCubic))
    .Key(1.0f, 0.5f)
    .TotalDuration(1.0f)
    .Play();

// Sequence
FAnimate::Sequence()
    .Then(FAnimate::Tween([widget](f32 v){ widget->Opacity(v); }, 0.0f, 1.0f).Duration(0.2f))
    .Then(FAnimate::Tween([widget](Vec2 v){ widget->Position(v); }, Vec2{}, Vec2{100, 0}).Duration(0.4f))
    .Play();

// Parallel with stagger
FAnimate::Parallel()
    .Add(FAnimate::Tween([a](f32 v){ a->Opacity(v); }, 0.0f, 1.0f).Duration(0.3f))
    .Add(FAnimate::Tween([b](f32 v){ b->Opacity(v); }, 0.0f, 1.0f).Duration(0.3f).Delay(0.05f))
    .Add(FAnimate::Tween([c](f32 v){ c->Opacity(v); }, 0.0f, 1.0f).Duration(0.3f).Delay(0.10f))
    .Play();

// Spring
FAnimate::Spring([widget](Vec2 v){ widget->Position(v); },
                 [widget](){ return widget->Position(); },
                 targetPos)
    .Stiffness(300.0f)
    .Damping(25.0f)
    .Play();

// Transition (declared once at widget setup — fires automatically on property change)
// Transition_Opacity / Transition_Position are generated by FUSION_TRANSITION_PROPERTY
transitionManager->RegisterTransition("Opacity",
    FTransitionSpec{ 0.2f, 0.0f, FEasingCurve(FEasingType::EaseOutQuad) },
    [this](f32 v) { Transition_Opacity(v); });
transitionManager->RegisterTransition("Position",
    FTransitionSpec{ 0.35f },
    [this](Vec2 v) { Transition_Position(v); });

widget->Opacity(0.5f);      // automatically animates
widget->Position({100, 0}); // automatically animates
```

---

## Serialization

Serializable (decorated with `FIELD()`):

- `FEasingCurve` — `type`, `controlPoint1`, `controlPoint2`
- `FTweenAnimation` — base fields (`duration`, `delay`, `speed`, `loopMode`)
- `FSpringAnimation` — `stiffness`, `damping`, `settleEpsilon` + base fields
- `FSequenceAnimation` / `FParallelAnimation` — `children` array + base fields
- `FKeyframeAnimation` — base fields only (`duration`, `delay`, `speed`, `loopMode`)

Not serialized (runtime-only):

- `from`, `to`, `easing`, `setter` — live inside `FInterpolator<T>`
- Keyframe array and `setter` — live inside `FKeyframeTrack<T>`
- Spring runtime state (`current`, `velocity`, `target`) and `setter` — live inside `FSpringState<T>`
- Active tween handles in `FTransitionManager`

Serialization of keyframe data for asset pipelines is the responsibility of higher-level engine modules. An asset layer stores the typed keyframe arrays and reconstructs the `FKeyframeAnimation` at load time by calling `SetTrack<T>()` and `AddKey()` to reattach the track and setter.

---

## File Layout

```
Engine/Source/FusionCore2/
  Public/Animation/
    FAnimatable.h          — template interpolation trait (non-Object)
    FEasingCurve.h         — FEasingType enum, FEasingCurve struct
    FAnimation.h           — abstract base class, FAnimationLoopMode, FAnimationState
    FTweenAnimation.h      — IFInterpolator, FInterpolator<T>, FTweenAnimation
    FKeyframeAnimation.h   — IFKeyframeTrack, FKeyframeTrack<T>, FKeyframeAnimation
    FSpringAnimation.h     — IFSpringState, FSpringState<T>, FSpringAnimation
    FSequenceAnimation.h   — FSequenceAnimation
    FParallelAnimation.h   — FParallelAnimation
    FTransitionManager.h   — FTransitionSpec, FTransitionManager
    FAnimate.h             — fluent builder helpers

  Private/Animation/
    FAnimation.cpp
    FEasingCurve.cpp       — all easing function implementations
    FTweenAnimation.cpp
    FKeyframeAnimation.cpp
    FSpringAnimation.cpp   — spring integration math
    FSequenceAnimation.cpp
    FParallelAnimation.cpp
    FTransitionManager.cpp
```
