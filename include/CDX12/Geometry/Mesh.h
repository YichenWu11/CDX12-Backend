#pragma once

#include <map>

#include <CDX12/DXUtil.h>
#include <CDX12/Resource/DefaultBuffer.h>
#include <CDX12/Resource/Resource.h>
#include <CDX12/Util/ReflactableStruct.h>


namespace Chen::CDX12 {
    class Mesh : public Resource {
        std::vector<DefaultBuffer>            vertexBuffers;
        DefaultBuffer                         indexBuffer;
        std::span<rtti::Struct const*>        vertexStructs;
        std::vector<D3D12_INPUT_ELEMENT_DESC> layout;
        uint32_t                              vertexCount;
        uint32_t                              indexCount;

    public:
        std::span<DefaultBuffer const>            VertexBuffers() const { return vertexBuffers; }
        DefaultBuffer const&                      IndexBuffer() const { return indexBuffer; }
        std::span<D3D12_INPUT_ELEMENT_DESC const> Layout() const { return layout; }
        Mesh(
            ID3D12Device*                  device,
            std::span<rtti::Struct const*> vbStructs,
            uint32_t                       vertexCount,
            uint32_t                       indexCount);

        void                    GetVertexBufferView(std::vector<D3D12_VERTEX_BUFFER_VIEW>& result) const;
        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
    };
} // namespace Chen::CDX12
