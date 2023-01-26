#pragma once

#include <CDX12/Resource/Buffer.h>

namespace Chen::CDX12 {
    class UploadBuffer final : public Buffer {
    private:
        ComPtr<ID3D12Resource> resource;
        uint64                 byteSize;

    public:
        ID3D12Resource*           GetResource() const override { return resource.Get(); }
        D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const override { return resource->GetGPUVirtualAddress(); }
        uint64                    GetByteSize() const override { return byteSize; }

        // TODO: bytesize ---> is_constant_buffer?
        UploadBuffer(
            ID3D12Device* device,
            uint64        byteSize);
        ~UploadBuffer();
        UploadBuffer(UploadBuffer&&)      = default;
        UploadBuffer(UploadBuffer const&) = delete;

        void                  CopyData(uint64 offset, std::span<vbyte const> data) const;
        D3D12_RESOURCE_STATES GetInitState() const override {
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        void DelayDispose(FrameResource* frameRes) const override;
    };
} // namespace Chen::CDX12
