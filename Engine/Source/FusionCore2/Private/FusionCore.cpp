
#include "FusionCore.h"

#include "FusionCore.private.h"
#include "Resource.h"

namespace CE
{
    RawData Get_Shaders_Fusion_vert_spv()
    {
        return RawData{ .data = (u8*)Shaders_Fusion_vert_spv_Data, .dataSize = Shaders_Fusion_vert_spv_Length };
    }

    RawData Get_Shaders_Fusion_frag_spv()
    {
        return RawData{ .data = (u8*)Shaders_Fusion_frag_spv_Data, .dataSize = Shaders_Fusion_frag_spv_Length };
    }

    RawData Get_Shaders_Fusion_vert_msl()
    {
        return RawData{ .data = (u8*)Shaders_Fusion_vert_msl_Data, .dataSize = Shaders_Fusion_vert_msl_Length };
    }

    RawData Get_Shaders_Fusion_frag_msl()
    {
        return RawData{ .data = (u8*)Shaders_Fusion_frag_msl_Data, .dataSize = Shaders_Fusion_frag_msl_Length };
    }

    RawData Get_Shaders_Fusion_vert_json()
    {
        return RawData{ .data = (u8*)Shaders_Fusion_vert_json_Data, .dataSize = Shaders_Fusion_vert_json_Length };
    }

    RawData Get_Shaders_Fusion_frag_json()
    {
        return RawData{ .data = (u8*)Shaders_Fusion_frag_json_Data, .dataSize = Shaders_Fusion_frag_json_Length };
    }

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
