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

        ShapeSettings(Ref<PhysicsScene> scene);

        FIELD()
    	WeakRef<PhysicsScene> ownerScene;

        FIELD(EditAnywhere)
        Vec3 position;

        FIELD(EditAnywhere)
        Vec3 rotation;

        FIELD(EditAnywhere)
        Vec3 scale = Vec3(1, 1, 1);

    };

    CLASS(Abstract)
    class COREPHYSICS_API PhysicsShape : public Object
    {
        CE_CLASS(PhysicsShape, Object)
    protected:

        PhysicsShape();
        
    public:

        virtual JPH::Shape* CreateJoltShape() const = 0;

        FIELD()
        Ref<PhysicsMaterial> physicsMaterial;

    };

    // - Box Shape -

    STRUCT()
    struct COREPHYSICS_API BoxShapeSettings : ShapeSettings
    {
		CE_STRUCT(BoxShapeSettings, ShapeSettings)
    public:
		BoxShapeSettings() = default;

        BoxShapeSettings(const Vec3& halfExtents, Ref<PhysicsScene> scene) : Super(scene), halfExtents(halfExtents)
        {
		}

        FIELD(EditAnywhere)
        Vec3 halfExtents = Vec3(1, 1, 1) * 0.5f;
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

        SphereShapeSettings(float radius, Ref<PhysicsScene> scene) : Super(scene), radius(radius)
        {
        }

        FIELD(EditAnywhere)
		float radius = 0.5f;
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

        CapsuleShapeSettings(float radius, float halfHeight, Ref<PhysicsScene> scene) : Super(scene), radius(radius), halfHeight(halfHeight)
        {
		}

        FIELD(EditAnywhere)
        float radius = 0.5f;

        FIELD(EditAnywhere)
		float halfHeight = 0.5f; // Half the height of the capsule, so the full height is 2 * halfHeight
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

    // - Cylinder Shape -

    STRUCT()
    struct COREPHYSICS_API CylinderShapeSettings : ShapeSettings
    {
        CE_STRUCT(CylinderShapeSettings, ShapeSettings)
    public:

        CylinderShapeSettings() = default;

        CylinderShapeSettings(float radius, float halfHeight, Ref<PhysicsScene> scene) : Super(scene), radius(radius), halfHeight(halfHeight)
        {
        }

        FIELD(EditAnywhere)
        float radius = 1;

        FIELD(EditAnywhere)
		float halfHeight = 1; // Half the height of the capsule, so the full height is 2 * halfHeight
    };

    CLASS()
    class COREPHYSICS_API CylinderShape : public PhysicsShape
    {
        CE_CLASS(CylinderShape, PhysicsShape)
    public:

		static Ref<CylinderShape> Create(const CylinderShapeSettings& settings, Ref<Object> outer = nullptr);

        JPH::Shape* CreateJoltShape() const override;

    protected:

        void OnBeginDestroy() override;

        FIELD()
        CylinderShapeSettings settings;

        
    };

    // - Static Compound Shape -

    STRUCT()
    struct COREPHYSICS_API StaticCompoundShapeSettings : ShapeSettings
    {
        CE_STRUCT(StaticCompoundShapeSettings, ShapeSettings)
    public:

        StaticCompoundShapeSettings() = default;

        FIELD()
        Array<Ref<PhysicsShape>> shapes;

        FIELD()
        Array<Vec3> shapePositions;

        FIELD()
        Array<Quat> shapeRotations;

        FIELD()
        Array<Vec3> shapeScales;
    };

    CLASS()
    class COREPHYSICS_API StaticCompoundShape : public PhysicsShape
    {
        CE_CLASS(StaticCompoundShape, PhysicsShape)
    public:

        static Ref<StaticCompoundShape> Create(const StaticCompoundShapeSettings& settings, Ref<Object> outer = nullptr);

        JPH::Shape* CreateJoltShape() const override;

    protected:

        void OnBeginDestroy() override;

        FIELD()
        StaticCompoundShapeSettings settings;

    };

} // namespace CE

#include "PhysicsShape.rtti.h"
