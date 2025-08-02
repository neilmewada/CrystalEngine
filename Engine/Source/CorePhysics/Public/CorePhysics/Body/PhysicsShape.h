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

        virtual JPH::Shape* CreateJoltShape() const = 0;

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

        FIELD(EditAnywhere)
        Vec3 halfExtents;
	};

    CLASS()
    class COREPHYSICS_API BoxShape : public PhysicsShape
    {
        CE_CLASS(BoxShape, PhysicsShape)
    public:

        static Ref<BoxShape> Create(const BoxShapeSettings& settings, Ref<Object> outer = nullptr);

        JPH::Shape* CreateJoltShape() const override;

    protected:

        void OnBeginDestroy() override;

        FIELD()
        BoxShapeSettings settings;

    private:

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

        FIELD(EditAnywhere)
		float radius = 0;
	};

	CLASS()
    class COREPHYSICS_API SphereShape : public PhysicsShape
    {
        CE_CLASS(SphereShape, PhysicsShape)
    public:

		static Ref<SphereShape> Create(const SphereShapeSettings& settings, Ref<Object> outer = nullptr);

        JPH::Shape* CreateJoltShape() const override;

	protected:

        void OnBeginDestroy() override;

        FIELD()
		SphereShapeSettings settings;

	private:

    };

	// - Capsule Shape -

    STRUCT()
    struct COREPHYSICS_API CapsuleShapeSettings : ShapeSettings
    {
        CE_STRUCT(CapsuleShapeSettings, ShapeSettings)
    public:

		CapsuleShapeSettings() = default;

        CapsuleShapeSettings(float radius, float halfHeight) 
            : radius(radius), halfHeight(halfHeight)
        {
		}

        FIELD(EditAnywhere)
        float radius = 1;

        FIELD(EditAnywhere)
		float halfHeight = 1; // Half the height of the capsule, so the full height is 2 * halfHeight
    };

    CLASS()
    class COREPHYSICS_API CapsuleShape : public PhysicsShape
    {
        CE_CLASS(CapsuleShape, PhysicsShape)
    public:

		static Ref<CapsuleShape> Create(const CapsuleShapeSettings& settings, Ref<Object> outer = nullptr);

        JPH::Shape* CreateJoltShape() const override;

    protected:

        void OnBeginDestroy() override;

        FIELD()
        CapsuleShapeSettings settings;


    };

} // namespace CE

#include "PhysicsShape.rtti.h"
