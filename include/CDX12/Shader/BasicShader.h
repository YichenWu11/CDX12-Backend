#pragma once

#include "Shader.h"

using Microsoft::WRL::ComPtr;

namespace Chen::CDX12 {
    class BasicShader : public Shader {
    public:
        ComPtr<ID3DBlob>                      vsShader = nullptr;
        ComPtr<ID3DBlob>                      psShader = nullptr;
        ComPtr<ID3DBlob>                      hsShader = nullptr;
        ComPtr<ID3DBlob>                      dsShader = nullptr;
        D3D12_RASTERIZER_DESC                 rasterizerState{CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)};
        D3D12_DEPTH_STENCIL_DESC              depthStencilState{CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)};
        D3D12_BLEND_DESC                      blendState{CD3DX12_BLEND_DESC(D3D12_DEFAULT)};
        std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
        BasicShader(
            std::span<std::pair<std::string, Property> const> properties,
            ID3D12Device*                                     device,
            std::span<D3D12_STATIC_SAMPLER_DESC>              samplers = GlobalSamplers::GetSamplers());
        BasicShader(
            std::span<std::pair<std::string, Property> const> properties,
            ComPtr<ID3D12RootSignature>&&                     rootSig);

        void SetVsShader(
            const wchar_t*                       path,
            const std::vector<D3D_SHADER_MACRO>& shaderDefines = std::vector<D3D_SHADER_MACRO>()) {
            if (path != nullptr)
                vsShader = std::move(DXUtil::CompileShader(path, shaderDefines.data(), "VSMain", "vs_5_1"));
        }

        void SetPsShader(
            const wchar_t*                       path,
            const std::vector<D3D_SHADER_MACRO>& shaderDefines = std::vector<D3D_SHADER_MACRO>()) {
            if (path != nullptr)
                psShader = std::move(DXUtil::CompileShader(path, shaderDefines.data(), "PSMain", "ps_5_1"));
        }

        void SetHsShader(
            const wchar_t*                       path,
            const std::vector<D3D_SHADER_MACRO>& shaderDefines = std::vector<D3D_SHADER_MACRO>()) {
            if (path != nullptr)
                hsShader = std::move(DXUtil::CompileShader(path, shaderDefines.data(), "HSMain", "hs_5_1"));
        }

        void SetDsShader(
            const wchar_t*                       path,
            const std::vector<D3D_SHADER_MACRO>& shaderDefines = std::vector<D3D_SHADER_MACRO>()) {
            if (path != nullptr)
                dsShader = std::move(DXUtil::CompileShader(path, shaderDefines.data(), "DSMain", "ds_5_1"));
        }

        void SetRasterizerState(D3D12_RASTERIZER_DESC desc) { rasterizerState = desc; }
        void SetDepthStencilState(D3D12_DEPTH_STENCIL_DESC desc) { depthStencilState = desc; }
        void SetBlendState(D3D12_BLEND_DESC desc) { blendState = desc; }
    };
} // namespace Chen::CDX12
