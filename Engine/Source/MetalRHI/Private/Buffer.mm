#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    Buffer::Buffer(Device* device, const RHI::BufferDescriptor& desc)
        : device(device), desc(desc)
    {
        name = desc.name;
        bindFlags = desc.bindFlags;
        bufferSize = desc.bufferSize;
        heapType = desc.defaultHeapType;
        structureByteStride = desc.structureByteStride;
        
        MTLResourceOptions options;
        
        switch (desc.defaultHeapType)
        {
            case RHI::MemoryHeapType::Default:
                options = MTLResourceStorageModePrivate;
                storageMode = MTLStorageModePrivate;
                break;
            case RHI::MemoryHeapType::Upload:
                options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;
                storageMode = MTLStorageModeShared;
                break;
            case RHI::MemoryHeapType::ReadBack:
                options = MTLResourceStorageModeShared;
                storageMode = MTLStorageModeShared;
                break;
        }
        
#if CE_BUILD_DEBUG
        options |= MTLResourceHazardTrackingModeTracked;
#endif
        
        mtlBuffer = [device->GetHandle() newBufferWithLength:bufferSize options:options];
    }

    Buffer::Buffer(Device* device, const RHI::BufferDescriptor& desc, const RHI::ResourceMemoryDescriptor& memoryDesc)
        : device(device), desc(desc)
    {
        memoryHeap = (Metal::MemoryHeap*)memoryDesc.memoryHeap;
        memoryOffset = memoryDesc.memoryOffset;
        
        name = desc.name;
        bindFlags = desc.bindFlags;
        bufferSize = desc.bufferSize;
        heapType = memoryHeap->GetHeapType();
        structureByteStride = desc.structureByteStride;
        
        storageMode = memoryHeap->GetStorageMode();
        
        MTLResourceOptions options = 0;
        
        switch (memoryHeap->GetStorageMode()) {
            case MTLStorageModeShared:
                options = MTLResourceStorageModeShared;
                break;
            case MTLStorageModeManaged:
                options = MTLResourceStorageModeManaged;
                break;
            case MTLStorageModePrivate:
                options = MTLResourceStorageModePrivate;
                break;
            case MTLStorageModeMemoryless:
                options = MTLResourceStorageModeMemoryless;
                break;
        }
        
        switch (memoryHeap->GetCpuCacheMode())
        {
            case MTLCPUCacheModeDefaultCache:
                options |= MTLResourceCPUCacheModeDefaultCache;
                break;
            case MTLCPUCacheModeWriteCombined:
                options |= MTLResourceCPUCacheModeWriteCombined;
                break;
        }
        
        switch (memoryHeap->GetHazardTrackingMode())
        {
            case MTLHazardTrackingModeDefault:
                options |= MTLResourceHazardTrackingModeDefault;
                break;
            case MTLHazardTrackingModeUntracked:
                options |= MTLResourceHazardTrackingModeUntracked;
                break;
            case MTLHazardTrackingModeTracked:
                options |= MTLResourceHazardTrackingModeTracked;
                break;
        }
        
        mtlBuffer = [memoryHeap->GetMtlHeap() newBufferWithLength:bufferSize options:options offset:memoryOffset];
    }

    Buffer::~Buffer()
    {
        mtlBuffer = nil;
    }

    void* Buffer::GetHandle()
    {
        return (void*)mtlBuffer;
    }

    bool Buffer::IsHostAccessible() const
    {
        return heapType == RHI::MemoryHeapType::Upload || heapType == RHI::MemoryHeapType::ReadBack;
    }

    void Buffer::UploadData(const BufferData& data)
    {
        if (IsHostAccessible())
        {
            void* ptr = [mtlBuffer contents];
            memcpy(ptr, data.data, data.dataSize);
        }
        else
        {
            UploadDataToGPU(data);
        }
    }

    bool Buffer::Map(u64 offset, u64 size, void** outPtr)
    {
        if (storageMode == MTLStorageModeShared || storageMode == MTLStorageModeManaged)
        {
            mappedOffset = offset;
            mappedSize = size;
            *outPtr = (u8*)[mtlBuffer contents] + offset;
            return true;
        }
        
        *outPtr = nullptr;
        return false;
    }

    bool Buffer::Unmap()
    {
        if (storageMode == MTLStorageModeManaged)
        {
            [mtlBuffer didModifyRange:NSMakeRange(mappedOffset, mappedSize)];
        }
        
        return true;
    }

    void Buffer::ReadData(u8** outData, u64* outDataSize)
    {
        if (!outData)
            return;
        
        if (IsHostAccessible())
        {
            if (outDataSize)
                *outDataSize = bufferSize;
            *outData = (u8*)malloc(bufferSize);
            memcpy(*outData, [mtlBuffer contents], bufferSize);
        }
        else
        {
            ReadDataFromGPU(outData, outDataSize);
        }
    }

    void Buffer::ReadData(void* data)
    {
        if (!data)
            return;
        
        if (IsHostAccessible())
        {
            memcpy(data, [mtlBuffer contents], bufferSize);
        }
        else
        {
            ReadDataFromGPU(data);
        }
    }

    void Buffer::UploadDataToGPU(const BufferData& data)
    {
        if (heapType != RHI::MemoryHeapType::Default)
            return;
        if (mtlBuffer == nil)
            return;
        
        id<MTLBuffer> stagingBuffer = [device->GetHandle() newBufferWithLength:data.dataSize options:MTLResourceStorageModeShared];
        
        memcpy([stagingBuffer contents], data.data, data.dataSize);
        
        CommandQueue* primaryQueue = device->GetPrimaryQueue();
        
        id<MTLCommandBuffer> cmdBuffer = [primaryQueue->GetMtlQueue() commandBuffer];
        id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
        
        [blitEncoder copyFromBuffer:stagingBuffer sourceOffset:0 toBuffer:mtlBuffer destinationOffset:data.startOffsetInBuffer size:data.dataSize];
        
        [blitEncoder endEncoding];
        [cmdBuffer commit];
        
        [cmdBuffer waitUntilCompleted];
    }

    void Buffer::ReadDataFromGPU(u8** outData, u64* outDataSize)
    {
        if (outData == nullptr)
            return;
        if (mtlBuffer == nil)
            return;
        
        id<MTLBuffer> stagingBuffer = [device->GetHandle() newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
        
        CommandQueue* primaryQueue = device->GetPrimaryQueue();
        
        id<MTLCommandBuffer> cmdBuffer = [primaryQueue->GetMtlQueue() commandBuffer];
        id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
        
        [blitEncoder copyFromBuffer:mtlBuffer sourceOffset:0 toBuffer:stagingBuffer destinationOffset:0 size:bufferSize];
        
        [blitEncoder endEncoding];
        [cmdBuffer commit];
        
        [cmdBuffer waitUntilCompleted];
        
        if (outDataSize)
            *outDataSize = bufferSize;
        
        memcpy(*outData, [stagingBuffer contents], bufferSize);
    }

    void Buffer::ReadDataFromGPU(void* data)
    {
        if (data == nullptr)
            return;
        if (mtlBuffer == nil)
            return;
        
        id<MTLBuffer> stagingBuffer = [device->GetHandle() newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
        
        CommandQueue* primaryQueue = device->GetPrimaryQueue();
        
        id<MTLCommandBuffer> cmdBuffer = [primaryQueue->GetMtlQueue() commandBuffer];
        id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
        
        [blitEncoder copyFromBuffer:mtlBuffer sourceOffset:0 toBuffer:stagingBuffer destinationOffset:0 size:bufferSize];
        
        [blitEncoder endEncoding];
        [cmdBuffer commit];
        
        [cmdBuffer waitUntilCompleted];
        
        memcpy(data, [stagingBuffer contents], bufferSize);
    }
    
} // namespace CE::Metal
