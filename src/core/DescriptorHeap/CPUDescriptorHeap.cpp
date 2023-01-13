#include <CDX12/DescripitorHeap/CPUDescriptorHeap.h>
#include <CDX12/DescripitorHeap/DescriptorHeapAllocation.h>
#include <mutex>

using namespace Chen::CDX12;

CPUDescriptorHeap::CPUDescriptorHeap(
    ID3D12Device*               pDevice,
    uint32_t                    NumDescriptorsInHeap,
    D3D12_DESCRIPTOR_HEAP_TYPE  Type,
    D3D12_DESCRIPTOR_HEAP_FLAGS Flags) :
    m_pDevice{pDevice},
    m_HeapDesc{
        Type,
        NumDescriptorsInHeap,
        Flags,
        1 // NodeMask
    },
    m_DescriptorSize{pDevice->GetDescriptorHandleIncrementSize(Type)} {
    // Create one pool
    m_HeapPool.emplace_back(m_pDevice, *this, 0, m_HeapDesc);
    m_AvailableHeaps.insert(0);
}

CPUDescriptorHeap::~CPUDescriptorHeap() {
    assert(m_CurrentSize == 0 && "Not all allocations released");
    assert(m_AvailableHeaps.size() == m_HeapPool.size() && "Not all descriptor heap pools are released");

#ifndef NDEBUG
    for (const auto& Heap : m_HeapPool)
        assert(Heap.GetNumAvailableDescriptors() == Heap.GetMaxDescriptors() && "Not all descriptors in the descriptor pool are released");
#endif // !NDEBUG
}

DescriptorHeapAllocation CPUDescriptorHeap::Allocate(uint32_t Count) {
    std::lock_guard<std::mutex> LockGuard(m_HeapPoolMutex);
    DescriptorHeapAllocation    Allocation;
    // Go through all descriptor heap managers that have free descriptors
    for (auto AvailableHeapIt = m_AvailableHeaps.begin(); AvailableHeapIt != m_AvailableHeaps.end(); ++AvailableHeapIt) {
        // Try to allocate descriptors using the current descriptor heap manager
        Allocation = m_HeapPool[*AvailableHeapIt].Allocate(Count);
        // Remove the manager from the pool if it has no more available descriptors
        if (m_HeapPool[*AvailableHeapIt].GetNumAvailableDescriptors() == 0)
            m_AvailableHeaps.erase(*AvailableHeapIt);

        // Terminate the loop if descriptor was successfully allocated, otherwise
        // go to the next manager
        if (Allocation.GetCpuHandle().ptr != 0)
            break;
    }

    // If there were no available descriptor heap managers or no manager was able
    // to suffice the allocation request, create a new manager
    if (Allocation.GetCpuHandle().ptr == 0) {
        // Make sure the heap is large enough to accomodate the requested number of descriptors
        if (static_cast<UINT>(Count) > m_HeapDesc.NumDescriptors) m_HeapDesc.NumDescriptors = static_cast<UINT>(Count);
        // Create a new descriptor heap manager. Note that this constructor creates a new D3D12 descriptor
        // heap and references the entire heap. Pool index is used as manager ID
        m_HeapPool.emplace_back(m_pDevice, *this, m_HeapPool.size(), m_HeapDesc);
        auto NewHeapIt = m_AvailableHeaps.insert(m_HeapPool.size() - 1);

        // Use the new manager to allocate descriptor handles
        Allocation = m_HeapPool[*NewHeapIt.first].Allocate(Count);
    }

    m_CurrentSize += (Allocation.GetCpuHandle().ptr != 0) ? Count : 0;
    m_MaxSize = (m_CurrentSize > m_MaxSize) ? m_CurrentSize : m_MaxSize;

    return Allocation;
}

void CPUDescriptorHeap::Free(DescriptorHeapAllocation&& Allocation) {
    std::lock_guard<std::mutex> LockGuard(m_HeapPoolMutex);
    auto                        ManagerId = Allocation.GetAllocationManagerId();
    m_CurrentSize -= static_cast<uint32_t>(Allocation.GetNumHandles());
    m_HeapPool[ManagerId].Free(std::move(Allocation));
}
