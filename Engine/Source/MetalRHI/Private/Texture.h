#pragma once

namespace CE::Metal
{
    class MemoryHeap;

    class Texture : public RHI::Texture
    {
    public:
        
        Texture(Device* device, const RHI::TextureDescriptor& desc);
        Texture(Device* device, const RHI::TextureDescriptor& desc, const RHI::ResourceMemoryDescriptor& memoryDesc);
        
        id<MTLTexture> GetMtlTexture() const { return mtlTexture; }
        
        void* GetHandle() override
        {
            return (void*)mtlTexture;
        }
        
        u32 GetNumberOfChannels() override;
        
        u32 GetBitsPerPixel() override;

    private:
        
        void Init(const RHI::TextureDescriptor& desc);
    
        Device* device = nullptr;
        
        MemoryHeap* memoryHeap = nullptr;
        u64 memoryOffset = 0;
        
        // - ObjC -
        MTLTextureDescriptor* textureDesc = nil;
        
        id<MTLTexture> mtlTexture = nil;
        
    };

} // namespace CE::Metal
