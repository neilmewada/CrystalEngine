#pragma once

namespace CE
{
    class FShader;

    CLASS(Abstract)
    class FUSIONCORE_API FRenderService : public FService
    {
        CE_CLASS(FRenderService, FService)
    protected:

        FRenderService();
        
    public:

        static constexpr u32 MaxTextureCount = 100'000;

        const RHI::ShaderResourceGroupLayout& GetSubPassSrgLayout() const { return subPassSrgLayout; }

        const RHI::ShaderResourceGroupLayout& GetObjectSrgLayout() const { return objectSrgLayout; }
        
        RHI::ShaderResourceGroup* GetSceneSrg() const { return sceneSrg; }

        // - Lifecycle -

        void OnStart() override;

        void OnShutdown() override;

        void TickService(FServiceTickPhase tickPhase) override;

        // - Rendering -

        virtual void MarkFrameGraphDirty() = 0;

        virtual void RenderPrepare() = 0;

        virtual bool BeginRender() = 0;
        virtual void EndRender() = 0;

        virtual int GetCurrentFrameIndex() = 0;

        Ref<FShader> GetMainShader() const { return mainShader; }

        // - Textures -

        int RegisterTexture(RHI::Texture* rhiTexture);

        void DeregisterTexture(int slot);

    protected:

        struct TextureImpl;

        int RegisterTexture(const TextureImpl& texture);

        void FlushTextures(int frameIndex);

        void UpdateDrawListMask(RHI::DrawListMask& drawListMask);

    protected:

        // - Shader -

        Ref<FShader> mainShader;

        // - Srgs -

        RHI::ShaderResourceGroup* sceneSrg = nullptr;

        RHI::ShaderResourceGroupLayout subPassSrgLayout{};

        RHI::ShaderResourceGroupLayout objectSrgLayout{};

        // - Sampler -

        struct FSampleState
        {
            RHI::FilterMode filterMode;
            RHI::SamplerAddressMode addressU, addressV;
            RHI::SamplerBorderColor borderColor;

            SIZE_T GetHash() const
            {
                SIZE_T hash = CE::GetHash(filterMode);
                CombineHash(hash, addressU);
                CombineHash(hash, addressV);

                if (addressU == SamplerAddressMode::ClampToBorder || addressV == SamplerAddressMode::ClampToBorder)
                {
                    CombineHash(hash, borderColor);
                }

                return hash;
            }

            bool operator==(const FSampleState& other) const
            {
                return GetHash() == other.GetHash();
            }

            bool operator!=(const FSampleState& other) const
            {
                return !operator==(other);
            }
        };

        HashMap<FSampleState, int> samplerIndicesByState;

        // - Texture -

        using FTextureArray = StableDynamicArray<RHI::Texture*, 256, false>;
        using FSlotArray = StableDynamicArray<int, 256, false>;

        FTextureArray texturesBySlot;
        FSlotArray freeSlots;
        int totalTextures = 0;
        BitSet<RHI::Limits::MaxSwapChainImageCount> textureDirty;

    };
    
} // namespace CE

#include "FRenderService.rtti.h"
