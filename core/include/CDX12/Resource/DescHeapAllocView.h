#pragma once

#include "CDX12/DescripitorHeap/DescriptorHeapAllocation.h"

namespace CDX12 {
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

} // namespace CDX12