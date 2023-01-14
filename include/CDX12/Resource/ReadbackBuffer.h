#pragma once

#include <CDX12/Resource/Buffer.h>

namespace Chen::CDX12 {
    class ReadbackBuffer final : public Buffer {
    private:
        ComPtr<ID3D12Resource> resource;
        uint64_t               byteSize;

    public:
        ID3D12Resource*           GetResource() const override { return resource.Get(); }
        D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const override { return resource->GetGPUVirtualAddress(); }
        uint64_t                  GetByteSize() const override { return byteSize; }

        ReadbackBuffer(
            ID3D12Device* device,
            uint64_t      byteSize);
        ~ReadbackBuffer();

        void CopyData(uint64_t offset, std::span<uint8_t> data) const;

        D3D12_RESOURCE_STATES GetInitState() const override {
            return D3D12_RESOURCE_STATE_COPY_DEST;
        }
        ReadbackBuffer(ReadbackBuffer&&)      = default;
        ReadbackBuffer(ReadbackBuffer const&) = delete;
        void DelayDispose(FrameResource* frameRes) const override;
    };
} // namespace Chen::CDX12
