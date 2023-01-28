#pragma once

#include <optional>

#include <CDX12/DescripitorHeap/DescriptorHeapAllocation.h>
#include <CDX12/Resource/BufferView.h>
#include <CDX12/Resource/DescHeapAllocView.h>
#include <CDX12/Shader/GlobalSamplers.h>
#include <CDX12/Shader/ShaderVariableType.h>

using Microsoft::WRL::ComPtr;

namespace Chen::CDX12 {
    class Shader {
    public:
        struct Property {
            ShaderVariableType type;
            uint32_t           spaceIndex;
            uint32_t           registerIndex;
            uint32_t           arrSize;
            Property(
                ShaderVariableType type,
                uint32_t           spaceIndex,
                uint32_t           registerIndex,
                uint32_t           arrSize) :
                type(type),
                spaceIndex(spaceIndex),
                registerIndex(registerIndex),
                arrSize(arrSize) {}
        };

    protected:
        struct InsideProperty : public Property {
            uint32_t rootSigPos;
            InsideProperty(Property const& p) :
                Property(p) {}
        };
        ComPtr<ID3D12RootSignature>                     rootSig;
        std::optional<InsideProperty>                   GetProperty(std::string_view str) const;
        std::unordered_map<std::string, InsideProperty> properties;

    public:
        Shader(
            std::span<std::pair<std::string, Property> const> properties,
            ID3D12Device*                                     device,
            std::span<D3D12_STATIC_SAMPLER_DESC>              samplers = GlobalSamplers::GetSamplers());

        Shader(
            std::span<std::pair<std::string, Property> const> properties,
            ComPtr<ID3D12RootSignature>&&                     rootSig);

        Shader(Shader&& v) = default;

        int32_t GetPropRootSigPos(std::string_view propertyName) const;

        ID3D12RootSignature* RootSig() const { return rootSig.Get(); }

        bool SetResource(
            std::string_view           propertyName,
            ID3D12GraphicsCommandList* cmdList,
            BufferView                 buffer) const;

        bool SetResource(
            std::string_view           propertyName,
            ID3D12GraphicsCommandList* cmdList,
            DescriptorHeapAllocView    view) const;

        virtual ~Shader() = default;
    };
} // namespace Chen::CDX12
