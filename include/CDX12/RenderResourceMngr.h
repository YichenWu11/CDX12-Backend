#pragma once

#include <CDX12/DXUtil.h>

namespace Chen::CDX12 {
    class MaterialMngr;
    class TextureMngr;
    class PSOManager;
    class MeshMngr;
    class ShaderMngr;

    class RenderResourceMngr {
    public:
        RenderResourceMngr() = default;
        ~RenderResourceMngr();

        void Init(ID3D12Device* device);

        MaterialMngr* GetMatMngr() { return matMngr; }
        TextureMngr*  GetTexMngr() { return texMngr; }
        PSOManager*   GetPSOMngr() { return psoMngr; }
        MeshMngr*     GetMeshMngr() { return meshMngr; }
        ShaderMngr*   GetShaderMngr() { return shaderMngr; }

        void Clear();

    private:
        bool isInit{false};

        MaterialMngr* matMngr{nullptr};
        TextureMngr*  texMngr{nullptr};
        PSOManager*   psoMngr{nullptr};
        MeshMngr*     meshMngr{nullptr};
        ShaderMngr*   shaderMngr{nullptr};
    };
} // namespace Chen::CDX12
