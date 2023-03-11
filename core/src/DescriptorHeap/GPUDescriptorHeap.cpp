#include "CDX12/DescripitorHeap/GPUDescriptorHeap.h"

namespace CDX12 {
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
        assert((MgrId == StaticHeapAllocatonManagerID || MgrId == DynamicHeapAllocatonManagerID)
               && "Unexpected allocation manager ID");

        if (MgrId == StaticHeapAllocatonManagerID)
            m_HeapAllocationManager.FreeAllocation(std::move(Allocation));
        else // MgrId == DynamicHeapAllocatonManagerID
            m_DynamicAllocationsManager.FreeAllocation(std::move(Allocation));
    }
} // namespace CDX12