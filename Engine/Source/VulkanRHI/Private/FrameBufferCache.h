#pragma once

namespace CE::Vulkan
{
    
    class FrameBufferCache final
    {
    public:

        struct Key
        {
            SIZE_T renderPassHash = 0;
            Array<SIZE_T> attachmentHandles;
            u32 width = 0;
            u32 height = 0;
            u32 layers = 1;

            SIZE_T GetHash() const;
        };

        Ptr<FrameBuffer> FindOrCreate(Device* device, Scope* scope, u32 frameSlot, u32 imageIndex);

    private:

        struct Entry
        {
            Ptr<FrameBuffer> frameBuffer;
        };

        HashMap<Key, Entry> cachedFrameBuffers{};

    };

} // namespace CE::Vulkan
