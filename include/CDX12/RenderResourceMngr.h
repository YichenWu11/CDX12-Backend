#pragma once

#include "./Geometry/MeshMngr.h"
#include "./Material/MaterialMngr.h"
#include "./Material/TextureMngr.h"
#include "./Shader/PSOManager.h"
#include "./Shader/ShaderMngr.h"

namespace Chen::CDX12 {
    class RenderResourceMngr {
    public:
        static RenderResourceMngr& GetInstance() noexcept {
            static RenderResourceMngr instance;
            return instance;
        }

        void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

        MaterialMngr* GetMatMngr() { return matMngr; }
        TextureMngr*  GetTexMngr() { return texMngr; }
        ShaderMngr*   GetShaderMngr() { return shaderMngr; }
        PSOManager*   GetPSOMngr() { return psoMngr; }

        void Clear();

    private:
        RenderResourceMngr() = default;
        ~RenderResourceMngr();

        bool isInit{false};

        MaterialMngr* matMngr{nullptr};
        TextureMngr*  texMngr{nullptr};
        ShaderMngr*   shaderMngr{nullptr};
        PSOManager*   psoMngr{nullptr};
    };
} // namespace Chen::CDX12
