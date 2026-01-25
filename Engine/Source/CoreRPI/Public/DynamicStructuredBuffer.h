#pragma once

namespace CE::RPI
{
    
	/**
     * @brief A structured buffer that can be dynamically resized with CPU data access.
     */
    template <typename TStruct>
    class DynamicStructuredBuffer
    {
    public:
        
        CE_NO_COPY(DynamicStructuredBuffer);

        static constexpr u32 MaxImageCount = RHI::Limits::MaxSwapChainImageCount;
        static constexpr u64 StructSize = sizeof(TStruct);

        DynamicStructuredBuffer();

        void Init(const Name& name, u64 initialNumElements, u32 imageCount = MaxImageCount);

        void Shutdown();

        RHI::Buffer* GetBuffer(u32 imageIndex) const;

        u64 GetElementCount() const;

        //! @brief Grows the buffer to fit exactly 'totalElementCount' number of total elements.
        void GrowToFit(u32 totalElementCount);

        bool IsInitialized() const
        {
            return initialNumElements > 0;
        }

        void Map(u32 imageIndex, u64 startOffset, u64 size, void** outPtr);
        void Unmap(u32 imageIndex);


        MultiCastDelegate<void(void)> onResizeCallback;

    private:

        StaticArray<RHI::Buffer*, MaxImageCount> buffers{};
        Name name{};
        u64 initialNumElements = 0;
        u32 imageCount = 0;

    };

    template <typename TStruct>
    DynamicStructuredBuffer<TStruct>::DynamicStructuredBuffer()
    {}

    template <typename TStruct>
    void DynamicStructuredBuffer<TStruct>::Init(const Name& name, u64 initialNumElements, u32 imageCount)
    {
        if (IsInitialized())
            return;

        this->name = name;
        this->initialNumElements = initialNumElements;
        imageCount = Math::Clamp<u32>(imageCount, 0, MaxImageCount);
        this->imageCount = imageCount;

        if (this->initialNumElements > 0)
        {
            RHI::BufferDescriptor bufferDesc{};
            bufferDesc.name = name;
            bufferDesc.bindFlags = BufferBindFlags::StructuredBuffer;
            bufferDesc.bufferSize = initialNumElements * StructSize;
            bufferDesc.defaultHeapType = MemoryHeapType::Upload;
            bufferDesc.structureByteStride = StructSize;

            for (int i = 0; i < imageCount; ++i)
            {
                buffers[i] = RHI::gDynamicRHI->CreateBuffer(bufferDesc);
            }
        }
    }

    template <typename TStruct>
    void DynamicStructuredBuffer<TStruct>::Shutdown()
    {
        if (!IsInitialized())
            return;

	    for (int i = 0; i < buffers.GetSize(); ++i)
	    {
            if (buffers[i] == nullptr)
                continue;

		    if (RPISystem::Get().IsInitialized())
		    {
                RPISystem::Get().QueueDestroy(buffers[i]);
		    }
            else
            {
                delete buffers[i];
            }
            buffers[i] = nullptr;
	    }

        initialNumElements = 0;
        imageCount = 0;
    }

    template <typename TStruct>
    RHI::Buffer* DynamicStructuredBuffer<TStruct>::GetBuffer(u32 imageIndex) const
    {
        for (int i = (int)imageIndex; i >= 0; --i)
        {
            if (buffers[i] != nullptr)
                return buffers[i];
        }
        return nullptr;
    }

    template <typename TStruct>
    u64 DynamicStructuredBuffer<TStruct>::GetElementCount() const
    {
        auto buffer = GetBuffer(0);
        if (buffer == nullptr)
            return 0;
        return buffer->GetBufferSize() / StructSize;
    }

    template <typename TStruct>
    void DynamicStructuredBuffer<TStruct>::GrowToFit(u32 totalElementCount)
    {
        if (totalElementCount <= GetElementCount())
            return;
        if (!IsInitialized())
            return;

        RHI::Buffer* originalBuffer = GetBuffer(0);
        if (!originalBuffer)
            return;

        RHI::BufferDescriptor bufferDesc{};
        bufferDesc.name = name;
        bufferDesc.bindFlags = BufferBindFlags::StructuredBuffer;
        bufferDesc.bufferSize = totalElementCount * StructSize;
        bufferDesc.defaultHeapType = MemoryHeapType::Upload;
        bufferDesc.structureByteStride = StructSize;

        StaticArray<RHI::Buffer*, MaxImageCount> newBuffers{};

        void* data;
        originalBuffer->Map(0, originalBuffer->GetBufferSize(), &data);

        for (int i = 0; i < newBuffers.GetSize(); ++i)
        {
            newBuffers[i] = gDynamicRHI->CreateBuffer(bufferDesc);

            newBuffers[i]->UploadData(data, originalBuffer->GetBufferSize(), 0);
        }

        originalBuffer->Unmap();

        for (int i = 0; i < buffers.GetSize(); ++i)
        {
            RPISystem::Get().QueueDestroy(buffers[i]);
            buffers[i] = newBuffers[i];
        }

        onResizeCallback.Broadcast();
    }

    template <typename TStruct>
    void DynamicStructuredBuffer<TStruct>::Map(u32 imageIndex, u64 startOffset, u64 size, void** outPtr)
    {
        buffers[imageIndex]->Map(startOffset, size, outPtr);
    }

    template <typename TStruct>
    void DynamicStructuredBuffer<TStruct>::Unmap(u32 imageIndex)
    {
        buffers[imageIndex]->Unmap();
    }

} // namespace CE::RPI
