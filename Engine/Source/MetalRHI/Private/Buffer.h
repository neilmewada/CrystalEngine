#pragma once

namespace CE::Metal
{

    class Buffer : public RHI::Buffer
    {
    public:
        
        Buffer(Device* device, const RHI::BufferDescriptor& desc);
        Buffer(Device* device, const RHI::BufferDescriptor& desc, const RHI::ResourceMemoryDescriptor& memoryDesc);
        
        virtual ~Buffer();
        
        void* GetHandle() override;
        
        bool IsHostAccessible() const override;
        
        void UploadData(const BufferData& data) override;
        
        bool Map(u64 offset, u64 size, void** outPtr) override;
        
        bool Unmap() override;
        
        void ReadData(u8** outData, u64* outDataSize) override;
        
        void ReadData(void* data) override;
        
        id<MTLBuffer> GetMtlBuffer() const { return mtlBuffer; }

    private:
        
        void UploadDataToGPU(const BufferData& data);
        
        void ReadDataFromGPU(u8** outData, u64* outDataSize);
        
        void ReadDataFromGPU(void* data);
        
        Device* device = nullptr;
        
        RHI::BufferDescriptor desc;
        MTLStorageMode storageMode;
        
        u64 mappedOffset;
        u64 mappedSize;
        
        // External memory heap
        MemoryHeap* memoryHeap = nullptr;
        u64 memoryOffset = 0;
        
        // - ObjC -
        
        id<MTLBuffer> mtlBuffer = nil;
    };
    
} // namespace CE::Metal
