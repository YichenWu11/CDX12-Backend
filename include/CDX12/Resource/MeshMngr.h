#pragma once

#include <map>

#include <CDX12/Resource/Mesh.h>

namespace Chen::CDX12 {
    struct Vertex : public rtti::Struct {
        rtti::Var<DirectX::XMFLOAT3> position  = "POSITION";
        rtti::Var<DirectX::XMFLOAT3> normal    = "NORMAL";
        rtti::Var<DirectX::XMFLOAT2> tex_coord = "TEXCOORD";
        rtti::Var<DirectX::XMFLOAT3> tangent   = "TANGENT";
    };

    class MeshMngr {
    public:
        enum AttrTag : uint8_t {
            TAG_POSITION = 0x1,
            TAG_NORMAL   = 0x2,
            TAG_TEXCOORD = 0x4,
            // TODO: read material
            TAG_MATERIAL = 0x8
        };

        enum class FileFormat : uint8_t {
            OBJ,
        };

        MeshMngr(ID3D12Device* _device);
        ~MeshMngr()                          = default;
        MeshMngr(const MeshMngr&)            = delete;
        MeshMngr& operator=(const MeshMngr&) = delete;

        void CreateMeshFromFile(
            ID3D12GraphicsCommandList* cmdlist,
            const std::string&         path,
            const std::string&         mesh_name,
            FileFormat                 file_format = FileFormat::OBJ,
            uint8_t                    tag         = AttrTag::TAG_POSITION | AttrTag::TAG_NORMAL | AttrTag::TAG_TEXCOORD);

    private:
        void CreateMeshFromOBJFile(
            ID3D12GraphicsCommandList* cmdlist,
            const std::string&         path,
            const std::string&         mesh_name,
            uint8_t                    tag = AttrTag::TAG_POSITION | AttrTag::TAG_NORMAL | AttrTag::TAG_TEXCOORD);

        ID3D12Device* device;

        std::map<std::string, std::unique_ptr<Mesh>> m_meshmap;
    };

} // namespace Chen::CDX12
