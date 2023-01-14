#pragma once

#include <CDX12/DescripitorHeap/DescriptorHeapAllocation.h>

namespace Chen::CDX12 {
    struct DescriptorHeapAllocView {
        DescriptorHeapAllocation const* heap;
        uint64                          index;

        DescriptorHeapAllocView(
            DescriptorHeapAllocation const* heap,
            uint64                          index) :
            heap(heap),
            index(index) {}

        DescriptorHeapAllocView(
            DescriptorHeapAllocation const* heap) :
            heap(heap),
            index(0) {}
    };

} // namespace Chen::CDX12