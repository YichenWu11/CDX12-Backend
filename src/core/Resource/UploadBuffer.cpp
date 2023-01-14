#include <CDX12/FrameResource.h>
#include <CDX12/Resource/UploadBuffer.h>

namespace Chen::CDX12 {
    UploadBuffer::UploadBuffer(
        ID3D12Device* device,
        uint64        byteSize) :
        Buffer(device),
        byteSize(byteSize) {
        auto prop   = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        ThrowIfFailed(device->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&resource)));
    }

    UploadBuffer::~UploadBuffer() {}

    void UploadBuffer::CopyData(uint64 offset, std::span<uint8_t const> data) const {
        void*       mappedPtr;
        D3D12_RANGE range;
        range.Begin = offset;
        range.End   = (byteSize < offset + data.size()) ? byteSize : offset + data.size();

        ThrowIfFailed(resource->Map(0, &range, reinterpret_cast<void**>(&mappedPtr)));
        memcpy(reinterpret_cast<uint8_t*>(mappedPtr) + offset, data.data(), range.End - range.Begin);
        resource->Unmap(0, &range);
    }

    void UploadBuffer::DelayDispose(FrameResource* frameRes) const {
        frameRes->AddDelayDisposeResource(resource);
    }
} // namespace Chen::CDX12
