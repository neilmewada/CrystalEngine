#pragma once

namespace CE
{
    STRUCT()
    struct COREPHYSICS_API ShapeSettings
    {
        CE_STRUCT(ShapeSettings)
    public:
        ShapeSettings() = default;

		virtual ~ShapeSettings() = default;
    };

    CLASS(Abstract)
    class COREPHYSICS_API PhysicsShape : public Object
    {
        CE_CLASS(PhysicsShape, Object)
    protected:

        PhysicsShape();
        
    public:


    };

    // - Box Shape -

    STRUCT()
    struct COREPHYSICS_API BoxShapeSettings : ShapeSettings
    {
		CE_STRUCT(BoxShapeSettings, ShapeSettings)
    public:
		BoxShapeSettings() = default;

        BoxShapeSettings(const Vec3& halfExtents) : halfExtents(halfExtents)
        {
		}

        FIELD()
        Vec3 halfExtents;
	};

    CLASS()
    class COREPHYSICS_API BoxShape : public PhysicsShape
    {
        CE_CLASS(BoxShape, PhysicsShape)
    public:

        static Ref<BoxShape> Create(const BoxShapeSettings& settings, Ref<Object> outer = nullptr);

    protected:

        void OnBeginDestroy() override;

        FIELD()
        BoxShapeSettings settings;

    private:

        struct Impl;
		Impl* impl = nullptr;
    };

	// - Sphere Shape -

    STRUCT()
    struct COREPHYSICS_API SphereShapeSettings : ShapeSettings
    {
        CE_STRUCT(SphereShapeSettings, ShapeSettings)
    public:

		SphereShapeSettings() = default;

        SphereShapeSettings(float radius) : radius(radius)
        {
        }

        FIELD()
		float radius = 0;
	};

	CLASS()
    class COREPHYSICS_API SphereShape : public PhysicsShape
    {
        CE_CLASS(SphereShape, PhysicsShape)
    public:

		static Ref<SphereShape> Create(const SphereShapeSettings& settings, Ref<Object> outer = nullptr);

	protected:

        void OnBeginDestroy() override;

        FIELD()
		SphereShapeSettings settings;

	private:

        struct Impl;
        Impl* impl = nullptr;
    };
    
} // namespace CE

#include "PhysicsShape.rtti.h"
