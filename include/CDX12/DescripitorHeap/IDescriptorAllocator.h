#pragma once

#include "../DXUtil.h"

namespace Chen::CDX12 {
    class DescriptorHeapAllocation;
    class DescriptorHeapAllocationMngr;
    class IDescriptorAllocator {
    public:
        // Allocate Count descriptors
        virtual DescriptorHeapAllocation Allocate(uint32_t Count)                    = 0;
        virtual void                     Free(DescriptorHeapAllocation&& Allocation) = 0;
        virtual uint32_t                 GetDescriptorSize() const                   = 0;
    };
} // namespace Chen::CDX12
