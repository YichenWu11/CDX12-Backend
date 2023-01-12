#pragma once

/*
    A Buffer is the simplest GPU resource, 
    which is essentially a piece of linear memory on the GPU.

               |---- DefaultBuffer     
    Buffer --------- UploadBuffer
              |----- ReadbackBuffer
*/

#include "Resource.h"
#include "BufferView.h"

namespace Chen::CDX12 {
    class Buffer : public Resource{
    public:
        Buffer(ID3D12Device* device);
        virtual uint64_t GetByteSize() const = 0;
        virtual ~Buffer();
        Buffer(Buffer&&) = default;
    };
}
