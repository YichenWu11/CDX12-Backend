#include <CDX12/DescripitorHeap/GPUDescriptorHeap.h>

using namespace Chen::CDX12;

GPUDescriptorHeap::GPUDescriptorHeap(
    ID3D12Device*               device,
    uint32_t                    NumDescriptorsInHeap,
    uint32_t                    NumDynamicDescriptors,
    D3D12_DESCRIPTOR_HEAP_TYPE  Type,
    D3D12_DESCRIPTOR_HEAP_FLAGS Flags) :
    m_Device{device},
    m_HeapDesc{
        Type,
        NumDescriptorsInHeap + NumDynamicDescriptors,
        Flags,
        1 // UINT NodeMask;
    },
    m_pd3d12DescriptorHeap{
        [&] {
    CComPtr<ID3D12DescriptorHeap> pHeap;
    device->CreateDescriptorHeap(&m_HeapDesc, __uuidof(pHeap), reinterpret_cast<void**>(&pHeap));
    return pHeap;
    }()},
m_DescriptorSize{device->GetDescriptorHandleIncrementSize(Type)},
m_HeapAllocationManager{device, *this, StaticHeapAllocatonManagerID, m_pd3d12DescriptorHeap, 0, NumDescriptorsInHeap},
m_DynamicAllocationsManager{device, *this, DynamicHeapAllocatonManagerID, m_pd3d12DescriptorHeap, NumDescriptorsInHeap, NumDynamicDescriptors} {
}

void GPUDescriptorHeap::Free(DescriptorHeapAllocation&& Allocation) {
    auto MgrId = Allocation.GetAllocationManagerId();
    if (MgrId == 0) {
        std::lock_guard<std::mutex> LockGuard(m_AllocMutex);
        m_HeapAllocationManager.FreeAllocation(std::move(Allocation));
    }
    else {
        std::lock_guard<std::mutex> LockGuard(m_DynAllocMutex);
        m_DynamicAllocationsManager.FreeAllocation(std::move(Allocation));
    }
}
