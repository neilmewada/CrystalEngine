#pragma once

namespace CE::Metal
{
    
    class MemoryHeap : public RHI::MemoryHeap
    {
    public:
        MemoryHeap(Device* device, const RHI::MemoryHeapDescriptor& desc);
        
        virtual ~MemoryHeap();
        
        id<MTLHeap> GetMtlHeap() const { return mtlHeap; }
        
        MTLStorageMode GetStorageMode() const { return storageMode; }
        
        MTLCPUCacheMode GetCpuCacheMode() const { return cpuCacheMode; }
        
        MTLHazardTrackingMode GetHazardTrackingMode() const { return hazardTrackingMode; }
        
    private:

        Device* device = nullptr;
        
        MTLStorageMode storageMode;
        MTLCPUCacheMode cpuCacheMode;
        MTLHazardTrackingMode hazardTrackingMode;
        
        // - ObjC -
        
        id<MTLHeap> mtlHeap = nil;
    };

} // namespace CE::Metal
