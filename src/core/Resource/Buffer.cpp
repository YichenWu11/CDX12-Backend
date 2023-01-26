#include <CDX12/Resource/Buffer.h>

using namespace Chen::CDX12;

BufferView::BufferView(Buffer const* buffer) :
    buffer(buffer),
    byteSize(buffer ? buffer->GetByteSize() : 0),
    offset(0) {}

BufferView::BufferView(
    Buffer const* buffer,
    uint64_t      offset,
    uint64_t      byteSize) :
    buffer(buffer),
    byteSize(byteSize),
    offset(offset) {}

Buffer::Buffer(ID3D12Device* device) :
    Resource(device) {}

Buffer::~Buffer() {}
