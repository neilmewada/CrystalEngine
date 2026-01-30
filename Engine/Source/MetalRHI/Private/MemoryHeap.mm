#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    MemoryHeap::MemoryHeap(Device* device, const RHI::MemoryHeapDescriptor& desc)
        : device(device)
    {
        debugName = desc.debugName;
        heapType = desc.heapType;
        heapSize = desc.allocationSize;
        usageFlags = desc.usageFlags;
        
        MTLHeapDescriptor* heapDesc = [[MTLHeapDescriptor alloc] init];
        heapDesc.size = desc.allocationSize;
        heapDesc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
#if CE_BUILD_DEBUG
        heapDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
#else
        heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
#endif
        
        switch (desc.heapType) {
            case RHI::MemoryHeapType::Upload:
                heapDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
            case RHI::MemoryHeapType::ReadBack:
                heapDesc.storageMode = MTLStorageModeShared;
                break;
            default:
                heapDesc.storageMode = MTLStorageModePrivate;
                break;
        }
        
        heapDesc.type = MTLHeapTypePlacement;
        
        cpuCacheMode = heapDesc.cpuCacheMode;
        storageMode = heapDesc.storageMode;
        hazardTrackingMode = heapDesc.hazardTrackingMode;
        
        mtlHeap = [device->GetHandle() newHeapWithDescriptor:heapDesc];
    }

    MemoryHeap::~MemoryHeap()
    {
        mtlHeap = nil;
    }
    
} // namespace CE::Metal
