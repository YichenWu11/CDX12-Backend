#pragma once

/*
    A Buffer is the simplest GPU resource,
    which is essentially a piece of linear memory on the GPU.

    Buffer:
        - DefaultBuffer
        - UploadBuffer
        - ReadbackBuffer
*/

#include <CDX12/Resource/BufferView.h>
#include <CDX12/Resource/Resource.h>

namespace Chen::CDX12 {
    class Buffer : public Resource {
    public:
        Buffer(ID3D12Device* device);
        Buffer(Buffer&&) = default;

        virtual uint64_t                  GetByteSize() const = 0;
        virtual D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const  = 0;
        virtual ~Buffer();
    };
} // namespace Chen::CDX12
