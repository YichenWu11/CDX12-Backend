#pragma once

#include "../DXUtil.h"
#include <map>

namespace Chen::CDX12 {
    struct SubmeshGeometry
    {
        UINT IndexCount = 0;
        UINT StartIndexLocation = 0;
        INT BaseVertexLocation = 0;

        DirectX::BoundingBox Bounds;
    };

    struct MeshGeometry
    {
        std::string Name;

        Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

        // GPU End
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

        UINT VertexByteStride = 0;      // per vertex data size in bytes
        UINT VertexBufferByteSize = 0;  // vertex buffer total size in bytes
        DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT; // DXGI_FORMAT_R16_UINT / DXGI_FORMAT_R32_UINT
        UINT IndexBufferByteSize = 0;   // index buffer total size in bytes

        std::map<std::string, SubmeshGeometry> DrawArgs;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
        {
            D3D12_VERTEX_BUFFER_VIEW vbv;  
            vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
            vbv.StrideInBytes = VertexByteStride;   
            vbv.SizeInBytes = VertexBufferByteSize;

            return vbv;
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
        {
            D3D12_INDEX_BUFFER_VIEW ibv;  
            ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
            ibv.Format = IndexFormat;              
            ibv.SizeInBytes = IndexBufferByteSize;

            return ibv;
        }

        void DisposeUploader()
        {
            VertexBufferUploader = nullptr;
            IndexBufferUploader  = nullptr;
        }

        std::vector<std::string> GetSubMeshNameList()
        {
            std::vector<std::string> nameList;
            for (auto& subMesh : DrawArgs)
            {
                nameList.push_back(subMesh.first);
            }
            return nameList;
        }

        int GetIndex(std::string name)
        {
            return (DrawArgs.find(name) != DrawArgs.end()) ? std::distance(DrawArgs.find(name), DrawArgs.begin()) : -1;
        }
    };
}
