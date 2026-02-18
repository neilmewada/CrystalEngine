
#include "FusionCore.h"

#include "FusionCore.private.h"
#include "Resource.h"

namespace CE
{
	

    class FusionCoreModule : public CE::Module
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
            CE_REGISTER_TYPES(FAffineTransform);
        }
    };
}

CE_IMPLEMENT_MODULE(FusionCore, CE::FusionCoreModule)
