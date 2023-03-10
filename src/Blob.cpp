#include "CDX12/Blob.h"

namespace CDX12 {
    void Blob::Create(SIZE_T size) {
        ThrowIfFailed(D3DCreateBlob(size, &raw));
    }

    void Blob::Copy(const void* data, SIZE_T size) {
        CopyMemory(raw->GetBufferPointer(), data, size);
    }

    void Blob::Create(const void* data, SIZE_T size) {
        Create(size);
        Copy(data, size);
    }
} // namespace CDX12