#include <CDX12/DescripitorHeap/DescriptorHeapAllocationMngr.h>

using namespace Chen::CDX12;

DescriptorHeapAllocationMngr::DescriptorHeapAllocationMngr(DescriptorHeapAllocationMngr&& rhs) :
    m_pDevice{rhs.m_pDevice},
    m_ParentAllocator{rhs.m_ParentAllocator},
    m_ThisManagerId{rhs.m_ThisManagerId},
    m_HeapDesc{rhs.m_HeapDesc},
    m_DescriptorSize{rhs.m_DescriptorSize},
    m_NumDescriptorsInAllocation{rhs.m_NumDescriptorsInAllocation},
    m_FirstCPUHandle{rhs.m_FirstCPUHandle},
    m_FirstGPUHandle{rhs.m_FirstGPUHandle},
    // Mutex is not movable
    // m_FreeBlockManagerMutex   { std::move(rhs.m_FreeBlockManagerMutex) },
    m_FreeBlockManager{std::move(rhs.m_FreeBlockManager)},
    m_pd3d12DescriptorHeap{std::move(rhs.m_pd3d12DescriptorHeap)} {
    rhs.m_NumDescriptorsInAllocation = 0; // Must be set to zero so that debug check in dtor passes
    rhs.m_ThisManagerId              = static_cast<size_t>(-1);
    rhs.m_FirstCPUHandle.ptr         = 0;
    rhs.m_FirstGPUHandle.ptr         = 0;
}

// Creates a new descriptor heap and reference the entire heap
DescriptorHeapAllocationMngr::DescriptorHeapAllocationMngr(
    ID3D12Device*                     pDevice,
    IDescriptorAllocator&             ParentAllocator,
    size_t                            ThisManagerId,
    const D3D12_DESCRIPTOR_HEAP_DESC& HeapDesc) :
    m_pDevice{pDevice},
    m_ParentAllocator{ParentAllocator},
    m_ThisManagerId{ThisManagerId},
    m_HeapDesc{HeapDesc},
    m_DescriptorSize{pDevice->GetDescriptorHandleIncrementSize(m_HeapDesc.Type)},
    m_NumDescriptorsInAllocation{HeapDesc.NumDescriptors},
    m_FreeBlockManager{HeapDesc.NumDescriptors} {
    m_FirstCPUHandle.ptr = 0;
    m_FirstGPUHandle.ptr = 0;

    pDevice->CreateDescriptorHeap(
        &m_HeapDesc,
        __uuidof(m_pd3d12DescriptorHeap),
        reinterpret_cast<void**>(static_cast<ID3D12DescriptorHeap**>(&m_pd3d12DescriptorHeap)));
    m_FirstCPUHandle = m_pd3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    // if shader visible
    if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        m_FirstGPUHandle = m_pd3d12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

// Uses subrange of descriptors in the existing D3D12 descriptor heap
// that starts at offset FirstDescriptor and uses NumDescriptors descriptors
DescriptorHeapAllocationMngr::DescriptorHeapAllocationMngr(
    ID3D12Device*         pDevice,
    IDescriptorAllocator& ParentAllocator,
    size_t                ThisManagerId,
    ID3D12DescriptorHeap* pd3d12DescriptorHeap,
    uint32_t              FirstDescriptor,
    uint32_t              NumDescriptors) :
    m_pDevice{pDevice},
    m_ParentAllocator{ParentAllocator},
    m_ThisManagerId{ThisManagerId},
    m_HeapDesc{pd3d12DescriptorHeap->GetDesc()},
    m_DescriptorSize{pDevice->GetDescriptorHandleIncrementSize(m_HeapDesc.Type)},
    m_NumDescriptorsInAllocation{NumDescriptors},
    m_FreeBlockManager{NumDescriptors},
    m_pd3d12DescriptorHeap{pd3d12DescriptorHeap} {
    m_FirstCPUHandle = pd3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_FirstCPUHandle.ptr += m_DescriptorSize * FirstDescriptor;
    // if shader visible
    if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
        m_FirstGPUHandle = pd3d12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        m_FirstGPUHandle.ptr += m_DescriptorSize * FirstDescriptor;
    }
}

DescriptorHeapAllocation DescriptorHeapAllocationMngr::Allocate(uint32_t Count) {
    assert(Count > 0);

    std::lock_guard<std::mutex> LockGuard(m_AllocationMutex);

    // Use variable-size GPU allocations manager to allocate the requested number of descriptors
    auto DescriptorHandleOffset = m_FreeBlockManager.Allocate(Count);
    // allocation failed.
    if (DescriptorHandleOffset == VarSizeGPUAllocMngr::InvalidOffset)
        return DescriptorHeapAllocation();

    // Compute the first CPU and GPU descriptor handles in the allocation by
    // offseting the first CPU and GPU descriptor handle in the range
    auto CPUHandle = m_FirstCPUHandle;
    CPUHandle.ptr += DescriptorHandleOffset * m_DescriptorSize;

    auto GPUHandle = m_FirstGPUHandle;
    // if shader visible
    if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        GPUHandle.ptr += DescriptorHandleOffset * m_DescriptorSize;

    return DescriptorHeapAllocation(
        &m_ParentAllocator,
        m_pd3d12DescriptorHeap,
        CPUHandle,
        GPUHandle,
        Count,
        static_cast<uint16_t>(m_ThisManagerId));
}

void DescriptorHeapAllocationMngr::FreeAllocation(DescriptorHeapAllocation&& allocation) {
    assert(allocation.GetAllocationManagerId() == m_ThisManagerId && "Invalid descriptor heap manager Id");

    if (allocation.IsNull())
        return;

    std::lock_guard<std::mutex> LockGuard(m_AllocationMutex);
    auto                        DescriptorOffset = (allocation.GetCpuHandle().ptr - m_FirstCPUHandle.ptr) / m_DescriptorSize;
    // Methods of VariableSizeAllocationsManager class are not thread safe!
    m_FreeBlockManager.Free(DescriptorOffset, allocation.GetNumHandles());

    // Clear the allocation
    allocation.Reset();
    allocation = DescriptorHeapAllocation();
}

void DescriptorHeapAllocationMngr::Free(DescriptorHeapAllocation&& Allocation) {
    std::lock_guard<std::mutex> LockGuard(m_AllocationMutex);
    auto                        DescriptorOffset = (Allocation.GetCpuHandle().ptr - m_FirstCPUHandle.ptr) / m_DescriptorSize;
    // Note that the allocation is not released immediately, but added to the release queue in the allocations manager
    m_FreeBlockManager.Free(DescriptorOffset, Allocation.GetNumHandles());
    // m_FreeBlockManager.Free(DescriptorOffset, Allocation.GetNumHandles(), m_pDeviceD3D12Impl->GetCurrentFrame());

    // Clear the allocation
    Allocation.Reset();
    Allocation = DescriptorHeapAllocation();
}
