#pragma once

#include "../DXUtil.h"

using Microsoft::WRL::ComPtr;

namespace Chen::CDX12 {
    class Resource {
    protected:
        ID3D12Device* device;

    public:
        ID3D12Device* GetDevice() const { return device; }
        Resource(ID3D12Device* device) : device(device) {}

        Resource(Resource&&) = default;
        Resource(Resource const&) = delete;
        virtual ~Resource() = default;
        virtual ID3D12Resource* GetResource() const { return nullptr; }
        virtual D3D12_RESOURCE_STATES GetInitState() const { return D3D12_RESOURCE_STATE_COMMON; }
    };
}