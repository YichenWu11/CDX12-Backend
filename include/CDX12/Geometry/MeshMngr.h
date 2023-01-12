#pragma once

#include "Mesh.h"
#include "../_deps/tinyobjloader/tinyobjloader.h"
#include <map>

namespace Chen::CDX12 {

    // vertex
    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TexC;
        DirectX::XMFLOAT3 TangentU;
    };

    class MeshMngr 
    {
    public:
        enum class MeshFileFormat : uint8_t
        {
            txt = 0,
        };

        MeshMngr(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
        ~MeshMngr() = default;
        MeshMngr(const MeshMngr&) = delete;
        MeshMngr& operator=(const MeshMngr&) = delete;

        MeshGeometry* GetMeshGeo(const std::string& name) {
            bool is = mGeometries.find(name) != mGeometries.end();
            return (mGeometries.find(name) != mGeometries.end()) ? mGeometries.at(name).get() : nullptr;
        }

        std::map<std::string, std::unique_ptr<MeshGeometry>>& GetAllGeos()
        {
            return mGeometries;
        }

        void BuildBasicGeo();

        void BuildTXTModelGeometryFromFile(
            const char* path, 
            const char* geoName, 
            const char* subName, 
            bool        is_normal, 
            bool        is_uv);

        void BuildOBJModelGeometryFromFile(
            const char* path, 
            const char* geoName);

    private:
        ID3D12Device* device;
        ID3D12GraphicsCommandList* cmdList;
        std::map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    };
}
