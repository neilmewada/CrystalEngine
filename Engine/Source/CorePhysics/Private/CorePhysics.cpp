
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
        }
    };
}

CE_IMPLEMENT_MODULE(CorePhysics, CE::CorePhysicsModule)
