#pragma once

#include "../VarSizeAllocMngr.h"
#include "DescriptorHeapAllocation.h"
#include "IDescriptorAllocator.h"

#include <atlbase.h>
#include <d3d12.h>
#include <mutex>

namespace Chen::CDX12 {
    /*
        Every allocation that the class creates is represented by an instance of DescriptorHeapAllocation class.
        The list of free descriptors is managed by m_FreeBlocksManager member
    */
    class DescriptorHeapAllocationMngr {
    public:
        // Creates a new D3D12 descriptor heap
        DescriptorHeapAllocationMngr(
            ID3D12Device*                     pDevice,
            IDescriptorAllocator&             pParentAllocator,
            size_t                            ThisManagerId,
            const D3D12_DESCRIPTOR_HEAP_DESC& HeapDesc);

        // Uses subrange of descriptors in the existing D3D12 descriptor heap
        // that starts at offset FirstDescriptor and uses NumDescriptors descriptors
        DescriptorHeapAllocationMngr(
            ID3D12Device*         pDevice,
            IDescriptorAllocator& ParentAllocator,
            size_t                ThisManagerId,
            ID3D12DescriptorHeap* pd3d12Descriptor,
            uint32_t              FirstDescriptor,
            uint32_t              NumDescriptors);

        // Move constructor
        DescriptorHeapAllocationMngr(DescriptorHeapAllocationMngr&& rhs);

        DescriptorHeapAllocationMngr& operator=(DescriptorHeapAllocationMngr&& rhs)  = delete;
        DescriptorHeapAllocationMngr(const DescriptorHeapAllocationMngr&)            = delete;
        DescriptorHeapAllocationMngr& operator=(const DescriptorHeapAllocationMngr&) = delete;

        ~DescriptorHeapAllocationMngr() = default;

        // Allocates Count descriptors
        DescriptorHeapAllocation Allocate(uint32_t Count);

        // Releases descriptor heap allocation. Note
        // that the allocation is not released immediately, but
        // added to the release queue in the allocations manager
        void Free(DescriptorHeapAllocation&& Allocation);

        void FreeAllocation(DescriptorHeapAllocation&& Allocation);

        size_t GetNumAvailableDescriptors() const { return m_FreeBlockManager.GetFreeSize(); }

        uint32_t GetMaxDescriptors() const { return m_NumDescriptorsInAllocation; }

    private:
        std::mutex            m_AllocationMutex;
        ID3D12Device*         m_pDevice = nullptr;
        IDescriptorAllocator& m_ParentAllocator;

        // External ID assigned to this descriptor allocations manager
        size_t m_ThisManagerId = static_cast<size_t>(-1);

        // Heap description
        D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

        UINT m_DescriptorSize = 0;

        // Number of descriptors in the all allocation.
        // If this manager was initialized as a subrange in the existing heap,
        // this value may be different from m_HeapDesc.NumDescriptors
        uint32_t m_NumDescriptorsInAllocation = 0;

        // First CPU descriptor handle in the available descriptor range
        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = {0};

        // First GPU descriptor handle in the available descriptor range
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = {0};

        // Allocations manager used to handle descriptor allocations within the heap
        VarSizeAllocMngr m_FreeBlockManager;

        // Strong reference to D3D12 descriptor heap object
        CComPtr<ID3D12DescriptorHeap> m_pd3d12DescriptorHeap;
    };
} // namespace Chen::CDX12
