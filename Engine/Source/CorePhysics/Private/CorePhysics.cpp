
#include "CorePhysics.h"

#include "CorePhysics.private.h"


namespace CE
{
    class CorePhysicsModule : public CE::Module
    {
    public:
        virtual void StartupModule() override
        {

        }

        virtual void ShutdownModule() override
        {

        }

        virtual void RegisterTypes() override
        {
            CE_REGISTER_TYPES(PhysicsLayer);

            // Just a safety check to ensure that this type has SerializePOD and DeserializePOD functions implemented.
            static_assert(THasSerializePODFunction<PhysicsLayer>::Value);
            static_assert(THasDeserializePODFunction<PhysicsLayer>::Value);
        }
    };
}

CE_IMPLEMENT_MODULE(CorePhysics, CE::CorePhysicsModule)
