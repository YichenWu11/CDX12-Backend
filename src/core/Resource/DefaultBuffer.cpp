#include <CDX12/FrameResource.h>
#include <CDX12/Resource/DefaultBuffer.h>

namespace Chen::CDX12 {
    DefaultBuffer::DefaultBuffer(
        ID3D12Device*         device,
        uint64_t              byteSize,
        D3D12_RESOURCE_STATES initState) :
        Buffer(device),
        initState(initState),
        byteSize(byteSize) {
        auto prop   = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto buffer = CD3DX12_RESOURCE_DESC::Buffer(byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        ThrowIfFailed(device->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            initState,
            nullptr,
            IID_PPV_ARGS(&resource)));
    }

    DefaultBuffer::~DefaultBuffer() {
    }

    void DefaultBuffer::DelayDispose(FrameResource* frameRes) const {
        frameRes->AddDelayDisposeResource(resource);
    }
} // namespace Chen::CDX12
