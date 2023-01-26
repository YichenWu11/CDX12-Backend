#pragma once

#include <map>

#include <CDX12/DXUtil.h>
#include <CDX12/Resource/DefaultBuffer.h>
#include <CDX12/Resource/Resource.h>
#include <CDX12/Util/ReflactableStruct.h>

namespace Chen::CDX12 {
    class Mesh : public Resource {
        uint32_t                              vertexCount;
        std::span<rtti::Struct const*>        vertexStructs;
        uint32_t                              indexCount;
        DefaultBuffer                         indexBuffer;
        std::vector<DefaultBuffer>            vertexBuffers;
        std::vector<D3D12_INPUT_ELEMENT_DESC> layout;

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

        void DelayDispose(FrameResource* frameres);

        DirectX::BoundingBox& GetBoundingBox() { return bounding_box; }
        DirectX::BoundingBox  GetBoundingBox() const { return bounding_box; }

        DirectX::BoundingBox bounding_box;
    };
} // namespace Chen::CDX12
