#pragma once

/*
    To mark a buffer.
    As same sa the std::string_view, just record an offset and a size.
*/

#include <CDX12/DXUtil.h>

namespace Chen::CDX12 {
    class Buffer;

    struct BufferView {
        Buffer const* buffer   = nullptr;
        uint64_t      byteSize = 0;
        uint64_t      offset   = 0;

        BufferView() {}
        BufferView(Buffer const* buffer);
        BufferView(
            Buffer const* buffer,
            uint64_t      offset,
            uint64_t      byteSize);
        bool operator==(BufferView const& a) const {
            return memcmp(this, &a, sizeof(BufferView)) == 0;
        }
        bool operator!=(BufferView const& a) const {
            return !operator==(a);
        }
    };
} // namespace Chen::CDX12
