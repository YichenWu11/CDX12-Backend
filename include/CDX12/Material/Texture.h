#pragma once

#include "../Resource/Resource.h"
#include "../DXUtil.h"

namespace Chen::CDX12 {
    enum class TextureDimension : uint8_t 
    {
        None = 0,
        Tex1D,
        Tex2D,
        Tex3D,
        Cubemap,
        Tex2DArray,
    };

    class Texture
    {
    public:
        // FIXME: Resource inheritance error
        Texture(TextureDimension _dimension = TextureDimension::Tex2D) : dimension(_dimension) {}

        D3D12_SHADER_RESOURCE_VIEW_DESC GetTexSrvDesc();
        ID3D12Resource* GetResource() const { return Resource.Get(); }
        D3D12_RESOURCE_STATES GetInitState() const { return D3D12_RESOURCE_STATE_COMMON; }
        ID3D12Resource* GetUploadHeap() const { return UploadHeap.Get(); }

        std::string Name; // Unique Texture name for lookup.
        std::wstring Filename;
        ComPtr<ID3D12Resource> Resource = nullptr;
        ComPtr<ID3D12Resource> UploadHeap = nullptr;

    private:
        TextureDimension dimension;
    };
}
