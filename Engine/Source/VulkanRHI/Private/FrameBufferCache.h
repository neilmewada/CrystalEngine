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

        struct Entry
        {
            SharedPtr<FrameBuffer> frameBuffer;
        };

    private:

        HashMap<Key, Entry> cache{};

    };

} // namespace CE::Vulkan
