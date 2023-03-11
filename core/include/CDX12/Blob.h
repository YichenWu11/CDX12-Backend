#pragma once

#include "DXUtil.h"

namespace CDX12 {
    struct Blob : ComPtrHolder<ID3DBlob> {
        using ComPtrHolder<ID3DBlob>::ComPtrHolder;

        void Create(SIZE_T size);
        void Copy(const void* data, SIZE_T size);
        void Create(const void* data, SIZE_T size);
    };
} // namespace CDX12