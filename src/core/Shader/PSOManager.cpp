#include <CDX12/Shader/PSOManager.h>
#include <CDX12/GeneralDesc.h>

using namespace Chen::CDX12;

void PSOManager::CreatePipelineState(
    std::string name,
    ID3D12Device* device,
    BasicShader* shader,
    UINT rtNum,
    DXGI_FORMAT mBackBufferFormat,
    DXGI_FORMAT mDepthStencilFormat,
    bool transparent,
    bool shadowMap)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc
        = Desc::PSO::MRT(
            shader->RootSig(),
            shader->mInputLayout.data(),
            (UINT)shader->mInputLayout.size(),
            shader->vsShader.Get(),
            shader->psShader.Get(),
            rtNum,
            mBackBufferFormat,
            mDepthStencilFormat);
    psoDesc.RasterizerState = shader->rasterizerState;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.DepthStencilState = shader->depthStencilState;
    if (!transparent)
        psoDesc.BlendState = shader->blendState;
    else
    {
        /* transparent classic */
        D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
        transparencyBlendDesc.BlendEnable = true;
        transparencyBlendDesc.LogicOpEnable = false;

        transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;

        transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
        transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
        transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

        transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
        transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        psoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    }
    
    if (shadowMap) 
    { 
        psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
        psoDesc.NumRenderTargets = 0;
        psoDesc.RasterizerState.DepthBias = 100000;
        psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
        psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    }

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs[name])));
    nameList.push_back(name);
}
