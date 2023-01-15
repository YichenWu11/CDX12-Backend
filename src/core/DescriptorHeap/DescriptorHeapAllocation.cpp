#include "CDX12/DescripitorHeap/IDescriptorAllocator.h"
#include <CDX12/DescripitorHeap/DescriptorHeapAllocation.h>
#include <memory>

using namespace Chen::CDX12;

DescriptorHeapAllocation::DescriptorHeapAllocation(
    IDescriptorAllocator*       pAllocator,
    ID3D12DescriptorHeap*       pHeap,
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle,
    uint32_t                    NHandles,
    uint16_t                    AllocationManagerId) :
    m_FirstCpuHandle{CpuHandle},
    m_FirstGpuHandle{GpuHandle},
    m_pAllocator{pAllocator},
    m_pDescriptorHeap{pHeap},
    m_NumHandles{NHandles},
    m_AllocationManagerId{AllocationManagerId} {
    assert(m_pAllocator != nullptr && m_pDescriptorHeap != nullptr);
    auto DescriptorSize = m_pAllocator->GetDescriptorSize();
    m_DescriptorSize    = static_cast<uint16_t>(DescriptorSize);
}

DescriptorHeapAllocation::DescriptorHeapAllocation(DescriptorHeapAllocation&& Allocation) noexcept :
    m_FirstCpuHandle{std::move(Allocation.m_FirstCpuHandle)},
    m_FirstGpuHandle{std::move(Allocation.m_FirstGpuHandle)},
    m_pAllocator{std::move(Allocation.m_pAllocator)},
    m_pDescriptorHeap{std::move(Allocation.m_pDescriptorHeap)},
    m_NumHandles{std::move(Allocation.m_NumHandles)},
    m_AllocationManagerId{std::move(Allocation.m_AllocationManagerId)},
    m_DescriptorSize{std::move(Allocation.m_DescriptorSize)} {
    Allocation.Reset();
}

DescriptorHeapAllocation& DescriptorHeapAllocation::operator=(DescriptorHeapAllocation&& Allocation) noexcept {
    m_FirstCpuHandle      = std::move(Allocation.m_FirstCpuHandle);
    m_FirstGpuHandle      = std::move(Allocation.m_FirstGpuHandle);
    m_pAllocator          = std::move(Allocation.m_pAllocator);
    m_pDescriptorHeap     = std::move(Allocation.m_pDescriptorHeap);
    m_NumHandles          = std::move(Allocation.m_NumHandles);
    m_AllocationManagerId = std::move(Allocation.m_AllocationManagerId);
    m_DescriptorSize      = std::move(Allocation.m_DescriptorSize);

    Allocation.Reset();

    return *this;
}

DescriptorHeapAllocation::~DescriptorHeapAllocation() {
    if (!IsNull() && m_pAllocator)
        m_pAllocator->Free(std::move(*this));
    // Allocation must have been disposed by the allocator
    assert("Non-null descriptor is being destroyed" && IsNull());
}

void DescriptorHeapAllocation::Reset() noexcept {
    m_FirstCpuHandle.ptr  = 0;
    m_FirstGpuHandle.ptr  = 0;
    m_pAllocator          = nullptr;
    m_pDescriptorHeap     = nullptr;
    m_NumHandles          = 0;
    m_AllocationManagerId = static_cast<uint16_t>(-1);
    m_DescriptorSize      = 0;
}
