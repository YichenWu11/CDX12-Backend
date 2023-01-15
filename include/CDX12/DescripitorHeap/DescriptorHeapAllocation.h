/*
    DescriptorHeapAllocation represents a descriptor heap allocation.
    It can be initialized as a single descriptor or as a continuous range of descriptors in the specified heap.

    [
        * A descriptor is a small block of data that fully describes
        an object to the GPU, in a GPU specific opaque format.
        * Descriptor heap is essentially an array of descriptors.
        * Every pipeline state incorporates a root signature that defines how shader
        registers are mapped to the descriptors in the bound descriptor heaps.
    ]

    [
        Resource binding is a two-stage process
            * shader register is first mapped to the descriptor in a descriptor heap
            as defined by the root signature.
            * The descriptor (which may be SRV, UAV, CBV or Sampler) then references
            the resource in GPU memory.
    ]
*/

#pragma once

#include <d3d12.h>

#include "IDescriptorAllocator.h"
#include <CDX12/Resource/Resource.h>

namespace Chen::CDX12 {
    class Resource;

    // contains the first CPU handle in CPU virtual address space, and,
    // if the heap is shader-visible, the first GPU handle in GPU virtual address space

    // The class represents descriptor heap allocation (continuous descriptor range in a descriptor heap)
    //
    //                  m_FirstCpuHandle
    //                   |
    //  | ~  ~  ~  ~  ~  X  X  X  X  X  X  X  ~  ~  ~  ~  ~  ~ |  D3D12 Descriptor Heap
    //                   |
    //                  m_FirstGpuHandle
    //

    // TODO: DescriptorHeapAllocation is derived form Resource, but here the `device` of base class is `nullptr`
    class DescriptorHeapAllocation : public Resource {
    public:
        // Creates null allocation
        DescriptorHeapAllocation() = default;

        // Initializes non-null allocation
        DescriptorHeapAllocation(
            IDescriptorAllocator*       pAllocator,
            ID3D12DescriptorHeap*       pHeap,
            D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle,
            uint32_t                    NHandles,
            uint16_t                    AllocationManagerId);

        // Move constructor
        DescriptorHeapAllocation(DescriptorHeapAllocation&& Allocation) noexcept;

        // Move assignment
        DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& Allocation) noexcept;

        // copy/copy assignment is not allowed.
        DescriptorHeapAllocation(const DescriptorHeapAllocation&)            = delete;
        DescriptorHeapAllocation& operator=(const DescriptorHeapAllocation&) = delete;

        ~DescriptorHeapAllocation();

        void Reset() noexcept;

        // Returns CPU descriptor handle at the specified offset
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32_t Offset = 0) const {
            D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_FirstCpuHandle;
            if (Offset != 0)
                CPUHandle.ptr += m_DescriptorSize * Offset;
            return CPUHandle;
        }

        // Returns GPU descriptor handle at the specified offset
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32_t Offset = 0) const {
            D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle = m_FirstGpuHandle;
            if (Offset != 0)
                GPUHandle.ptr += m_DescriptorSize * Offset;
            return GPUHandle;
        }

        // Returns pointer to the descriptor heap that contains this allocation
        ID3D12DescriptorHeap* GetDescriptorHeap() { return m_pDescriptorHeap; }

        size_t GetNumHandles() const { return m_NumHandles; }

        bool IsNull() const { return m_FirstCpuHandle.ptr == 0; }
        bool IsShaderVisible() const { return m_FirstGpuHandle.ptr != 0; }
        // bool

        size_t GetAllocationManagerId() { return m_AllocationManagerId; }
        UINT   GetDescriptorSize() const { return m_DescriptorSize; }

    private:
        // First CPU descriptor handle in this allocation
        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCpuHandle{0};

        // First GPU descriptor handle in this allocation
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGpuHandle{0};

        // Pointer to the descriptor heap allocator that created this allocation
        IDescriptorAllocator* m_pAllocator{nullptr};

        // Pointer to the D3D12 descriptor heap that contains descriptors in this allocation
        ID3D12DescriptorHeap* m_pDescriptorHeap{nullptr};

        // Number of descriptors in the allocation
        uint32_t m_NumHandles = 0;

        // Allocation manager ID
        // Indicates which AllocMngr this allocation is included in.
        uint16_t m_AllocationManagerId = static_cast<uint16_t>(-1);

        // Descriptor size
        uint16_t m_DescriptorSize = 0;
    };
} // namespace Chen::CDX12
