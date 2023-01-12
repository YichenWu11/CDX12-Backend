#pragma once

#include "DXUtil.h"
#include <d3d12.h>
#include <unknwnbase.h>

namespace Chen::CDX12 {
    // raw : Microsoft::WRL::ComPtr<ID3D12Device>
    // .   : simple API
    // ->  : raw API
	struct Device : ComPtrHolder<ID3D12Device> {
		using ComPtrHolder<ID3D12Device>::ComPtrHolder;

        void CreateCommittedResource(
            D3D12_HEAP_TYPE heap_type,
            SIZE_T size,
            ID3D12Resource** resources);

        void CreateDescriptorHeap(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type,
            ID3D12DescriptorHeap** pHeap);

        HRESULT Create(IUnknown* pAdapter, D3D_FEATURE_LEVEL level);

        ComPtr<ID3D12RootSignature> CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc);
    };
}
