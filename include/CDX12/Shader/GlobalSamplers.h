#pragma once

#include "../DXUtil.h"
#include "../_deps/d3dx12.h"

namespace Chen::CDX12 {
    class GlobalSamplers
    {
    public:
        static std::span<D3D12_STATIC_SAMPLER_DESC> GetSamplers();
        static std::span<D3D12_STATIC_SAMPLER_DESC> GetSSAOSamplers();
    };
}
