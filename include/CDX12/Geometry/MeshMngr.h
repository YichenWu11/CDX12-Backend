#pragma once

#include <map>

#include <CDX12/_deps/tinyobjloader/tinyobjloader.h>

#include <CDX12/Geometry/Mesh.h>

namespace Chen::CDX12 {
    struct Vertex : public rtti::Struct {
        rtti::Var<DirectX::XMFLOAT3> position  = "POSITION";
        rtti::Var<DirectX::XMFLOAT3> normal    = "NORMAL";
        rtti::Var<DirectX::XMFLOAT2> tex_coord = "TEXCOORD";
        rtti::Var<DirectX::XMFLOAT3> tangent_u = "TANGENT";
    };

} // namespace Chen::CDX12
