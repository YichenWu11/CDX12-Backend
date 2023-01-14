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

    // template <typename T>
    // class UploadBuffer final : public Buffer {
    // public:
    //     UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
    //         Buffer(device), mIsConstantBuffer(isConstantBuffer) {
    //         mElementByteSize = sizeof(T);

    //         if (isConstantBuffer)
    //             mElementByteSize = DXUtil::CalcConstantBufferByteSize(sizeof(T));

    //         ThrowIfFailed(device->CreateCommittedResource(
    //             get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
    //             D3D12_HEAP_FLAG_NONE,
    //             get_rvalue_ptr(CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount)),
    //             D3D12_RESOURCE_STATE_GENERIC_READ,
    //             nullptr,
    //             IID_PPV_ARGS(&mUploadBuffer)));

    //         ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));
    //     }

    //     UploadBuffer(const UploadBuffer& rhs)            = delete;
    //     UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

    //     ID3D12Resource*       GetResource() const override { return mUploadBuffer.Get(); }
    //     D3D12_RESOURCE_STATES GetInitState() const override { return D3D12_RESOURCE_STATE_GENERIC_READ; }
    //     uint64_t              GetByteSize() const override { return mElementByteSize; }

    //     ~UploadBuffer() {
    //         if (mUploadBuffer != nullptr)
    //             mUploadBuffer->Unmap(0, nullptr);

    //         mMappedData = nullptr;
    //     }

    //     void CopyData(int elementIndex, const T& data) {
    //         memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
    //     }

    // private:
    //     Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
    //     BYTE*                                  mMappedData = nullptr; // map media

    //     UINT mElementByteSize  = 0;
    //     bool mIsConstantBuffer = false;
    // };
} // namespace Chen::CDX12
