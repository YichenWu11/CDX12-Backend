#pragma once

#include "../DXUtil.h"
#include "./BasicShader.h"

using Microsoft::WRL::ComPtr;

namespace Chen::CDX12 {
    class PSOManager {
    public:
        PSOManager() = default;

        void CreatePipelineState(
            std::string name,
            ID3D12Device* device,
            BasicShader* shader,
            UINT rtNum,
            DXGI_FORMAT mBackBufferFormat,
            DXGI_FORMAT mDepthStencilFormat,
            bool transparent = false,
            bool shadowMap = false);

        ID3D12PipelineState* GetPipelineState(const std::string& name) const {
            if (mPSOs.find(name) != mPSOs.end()) {
                return mPSOs.at(name).Get();
            }
            else {
                return nullptr;
            }
        }

        std::vector<std::string>& GetPSONameList() { return nameList; }

    private:
        std::map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
        std::vector<std::string> nameList;
    };
}
