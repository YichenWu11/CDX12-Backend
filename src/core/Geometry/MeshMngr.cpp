#include <CDX12/DXUtil.h>
#include <CDX12/Geometry/MeshMngr.h>
#include <CDX12/Math/MathHelper.h>

// void MeshMngr::BuildOBJModelGeometryFromFile(
//     const char* path,
//     const char* geoName) {
//     tinyobj::ObjReaderConfig reader_config;
//     reader_config.mtl_search_path = "./"; // Path to material files

//     tinyobj::ObjReader reader;

//     if (!reader.ParseFromFile(std::string(path), reader_config))
//         if (!reader.Error().empty()) return;

//     if (!reader.Warning().empty())
//         OutputDebugString(AnsiToWString(reader.Warning()).c_str());

//     auto& attrib    = reader.GetAttrib();
//     auto& shapes    = reader.GetShapes();
//     auto& materials = reader.GetMaterials();

//     UINT vcount = 0; // vertex count
//     UINT tcount = 0; // triangle(face) count
//     for (size_t s = 0; s < shapes.size(); s++) tcount += shapes[s].mesh.indices.size();

//     std::vector<Vertex>       vertices(tcount);
//     std::vector<std::int32_t> indices(tcount);

//     auto geo  = std::make_unique<MeshGeometry>();
//     geo->Name = geoName;

//     // Loop over shapes
//     int idxInVertices = 0;
//     for (size_t s = 0; s < shapes.size(); s++) {
//         BoundingBox bounds;

//         XMFLOAT3 vMinf3(+Math::MathHelper::Infinity, +Math::MathHelper::Infinity, +Math::MathHelper::Infinity);
//         XMFLOAT3 vMaxf3(-Math::MathHelper::Infinity, -Math::MathHelper::Infinity, -Math::MathHelper::Infinity);

//         XMVECTOR vMin = XMLoadFloat3(&vMinf3);
//         XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

//         // Loop over faces(polygon)
//         size_t index_offset = 0;
//         for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
//             size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
//             // Loop over vertices in the face.
//             for (size_t v = 0; v < fv; v++) {
//                 // access to vertex
//                 tinyobj::index_t idx          = shapes[s].mesh.indices[index_offset + v];
//                 vertices[idxInVertices].Pos.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
//                 vertices[idxInVertices].Pos.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
//                 vertices[idxInVertices].Pos.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

//                 // Check if `normal_index` is zero or positive. negative = no normal data
//                 if (idx.normal_index >= 0) {
//                     vertices[idxInVertices].Normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
//                     vertices[idxInVertices].Normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
//                     vertices[idxInVertices].Normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
//                 }

//                 // Check if `texcoord_index` is zero or positive. negative = no texcoord data
//                 if (idx.texcoord_index >= 0) {
//                     vertices[idxInVertices].TexC.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
//                     vertices[idxInVertices].TexC.y = 1.0f - attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
//                 }

//                 XMVECTOR P = XMLoadFloat3(&vertices[idxInVertices].Pos);

//                 XMVECTOR N = XMLoadFloat3(&vertices[idxInVertices].Normal);

//                 XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//                 if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f) {
//                     XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
//                     XMStoreFloat3(&vertices[idxInVertices].TangentU, T);
//                 }
//                 else {
//                     up         = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
//                     XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
//                     XMStoreFloat3(&vertices[idxInVertices].TangentU, T);
//                 }

//                 vMin = XMVectorMin(vMin, P);
//                 vMax = XMVectorMax(vMax, P);

//                 ++idxInVertices;
//             }
//             index_offset += fv;
//         }
//         XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
//         XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

//         SubmeshGeometry submesh;
//         submesh.IndexCount = (UINT)shapes[s].mesh.indices.size();
//         if (s == 0) {
//             submesh.StartIndexLocation = 0;
//             submesh.BaseVertexLocation = 0;
//         }
//         else {
//             submesh.StartIndexLocation = shapes[s - 1].mesh.indices.size();
//             submesh.BaseVertexLocation = shapes[s - 1].mesh.indices.size();
//         }

//         submesh.Bounds = bounds;

//         geo->DrawArgs[shapes[s].name] = submesh;
//     }

//     for (UINT i = 0; i < tcount; ++i) {
//         indices[i] = i;
//     }

//     const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//     const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

//     ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
//     CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

//     ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
//     CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

//     geo->VertexBufferGPU = DXUtil::CreateDefaultBuffer(device,
//                                                        cmdList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

//     geo->IndexBufferGPU = DXUtil::CreateDefaultBuffer(device,
//                                                       cmdList, indices.data(), ibByteSize, geo->IndexBufferUploader);

//     geo->VertexByteStride     = sizeof(Vertex);
//     geo->VertexBufferByteSize = vbByteSize;
//     geo->IndexFormat          = DXGI_FORMAT_R32_UINT;
//     geo->IndexBufferByteSize  = ibByteSize;

//     mGeometries[geo->Name] = std::move(geo);
// }
