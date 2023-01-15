#include <iostream>

#include <CDX12/_deps/tinyobjloader/tinyobjloader.h>
#include <SimpleMath.h>

#include <CDX12/DXUtil.h>
#include <CDX12/Math/MathHelper.h>
#include <CDX12/Resource/MeshMngr.h>
#include <CDX12/Resource/UploadBuffer.h>

using namespace DirectX;

namespace Chen::CDX12 {
    MeshMngr::MeshMngr(ID3D12Device* _device) :
        device(_device) {
    }

    void MeshMngr::CreateMeshFromFile(
        ID3D12GraphicsCommandList* cmdlist,
        const std::string&         path,
        const std::string&         mesh_name,
        FileFormat                 file_format,
        uint8_t                    tag) {
        if (m_meshmap.contains(mesh_name))
            return;

        switch (file_format) {
            case FileFormat::OBJ:
                CreateMeshFromOBJFile(cmdlist, path, mesh_name, tag);
            default:
                break;
        }
    }

    static Vertex vertexSample;

    void MeshMngr::CreateMeshFromOBJFile(
        ID3D12GraphicsCommandList* cmdlist,
        const std::string&         path,
        const std::string&         mesh_name,
        uint8_t                    tag) {
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "./"; // Path to material files

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path, reader_config))
            if (!reader.Error().empty()) return;

        if (!reader.Warning().empty())
            // TODO: Log
            std::cout << reader.Warning() << std::endl;

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        // auto& materials = reader.GetMaterials();

        // TODO: optimize indexed

        uint32_t v_count = 0; // vertex count
        uint32_t i_count = 0; // indices count
        for (size_t s = 0; s < shapes.size(); s++) i_count += shapes[s].mesh.indices.size();
        v_count = i_count;

        std::vector<rtti::Struct const*> structs;
        structs.emplace_back(&vertexSample);

        auto mesh = std::make_unique<Mesh>(
            device,
            structs,
            v_count,
            i_count);

        std::vector<vbyte>        vertexData(vertexSample.structSize * v_count);
        std::vector<std::int32_t> indicesData(i_count);
        vbyte*                    vertexDataPtr = vertexData.data();

        BoundingBox bounds;

        XMFLOAT3 vMinf3(+Math::MathHelper::Infinity, +Math::MathHelper::Infinity, +Math::MathHelper::Infinity);
        XMFLOAT3 vMaxf3(-Math::MathHelper::Infinity, -Math::MathHelper::Infinity, -Math::MathHelper::Infinity);

        XMVECTOR vMin = XMLoadFloat3(&vMinf3);
        XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    if (tag & 0x1) {
                        vertexSample.position.Get(vertexDataPtr) =
                            XMFLOAT3(attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                                     attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                                     attrib.vertices[3 * size_t(idx.vertex_index) + 2]);
                    }

                    // Check if `normal_index` is zero or positive. negative = no normal data
                    if (idx.normal_index >= 0) {
                        if (tag & 0x2) {
                            vertexSample.normal.Get(vertexDataPtr) =
                                XMFLOAT3(attrib.normals[3 * size_t(idx.normal_index) + 0],
                                         attrib.normals[3 * size_t(idx.normal_index) + 1],
                                         attrib.normals[3 * size_t(idx.normal_index) + 2]);
                        }
                    }

                    // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                    if (idx.texcoord_index >= 0) {
                        if (tag & 0x4) {
                            vertexSample.tex_coord.Get(vertexDataPtr) =
                                XMFLOAT2(attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
                                         1.0f - attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
                        }
                    }

                    XMVECTOR P  = XMLoadFloat3(&vertexSample.position.Get(vertexDataPtr));
                    XMVECTOR N  = XMLoadFloat3(&vertexSample.normal.Get(vertexDataPtr));
                    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

                    if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f) {
                        XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
                        XMFLOAT3 tangent;
                        XMStoreFloat3(&tangent, T);
                        vertexSample.tangent.Get(vertexDataPtr) = tangent;
                    }
                    else {
                        up         = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
                        XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
                        XMFLOAT3 tangent;
                        XMStoreFloat3(&tangent, T);
                        vertexSample.tangent.Get(vertexDataPtr) = tangent;
                    }

                    vertexDataPtr += vertexSample.structSize;

                    vMin = XMVectorMin(vMin, P);
                    vMax = XMVectorMax(vMax, P);
                }
                index_offset += fv;
            }
        }
        XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));
        // set bounding_box
        mesh->GetBoundingBox() = bounds;

        for (UINT i = 0; i < i_count; ++i)
            indicesData[i] = i;

        auto vertexUpload = std::make_unique<UploadBuffer>(
            device,
            vertexData.size());
        vertexUpload->CopyData(0, vertexData);

        auto indexUpload = std::make_unique<UploadBuffer>(
            device,
            i_count * sizeof(uint32_t));
        indexUpload->CopyData(0, {reinterpret_cast<vbyte const*>(indicesData.data()), i_count * sizeof(std::uint32_t)});

        // Copy vertex buffer to mesh
        cmdlist->CopyBufferRegion(
            mesh->VertexBuffers()[0].GetResource(),
            0,
            vertexUpload->GetResource(),
            0,
            vertexUpload->GetByteSize());

        // Copy index buffer to mesh
        cmdlist->CopyBufferRegion(
            mesh->IndexBuffer().GetResource(),
            0,
            indexUpload->GetResource(),
            0,
            indexUpload->GetByteSize());

        m_meshmap[mesh_name] = std::move(mesh);
    }
} // namespace Chen::CDX12
