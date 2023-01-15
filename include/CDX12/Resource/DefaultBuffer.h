#pragma once

#include <CDX12/Resource/Buffer.h>

namespace Chen::CDX12 {

    class DefaultBuffer final : public Buffer {
    private:
        D3D12_RESOURCE_STATES  initState;
        uint64_t               byteSize;
        ComPtr<ID3D12Resource> resource;

    public:
        ID3D12Resource*           GetResource() const override { return resource.Get(); }
        D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const override { return resource->GetGPUVirtualAddress(); }
        uint64_t                  GetByteSize() const override { return byteSize; }
        DefaultBuffer(
            ID3D12Device*         device,
            uint64_t              byteSize,
            D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);

        ~DefaultBuffer();

        D3D12_RESOURCE_STATES GetInitState() const override {
            return initState;
        }

        DefaultBuffer(DefaultBuffer&&)      = default;
        DefaultBuffer(DefaultBuffer const&) = delete;
        void DelayDispose(FrameResource* frameRes) const override;
    };
} // namespace Chen::CDX12
