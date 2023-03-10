#pragma once

#include "../DXUtil.h"

namespace CDX12 {
    enum class ShaderVariableType : uint8_t {
        ConstantBuffer,
        CBVDescriptorHeap,
        SRVDescriptorHeap,
        BindlessSRVDescriptorHeap,
        UAVDescriptorHeap,
        StructuredBuffer,
        RWStructuredBuffer
    };

    struct ShaderVariable {
        std::string        name;
        ShaderVariableType type;
        uint32_t           tableSize;
        uint32_t           registerPos;
        uint32_t           space;
        ShaderVariable() {}
        ShaderVariable(
            const std::string& name,
            ShaderVariableType type,
            uint32_t           tableSize,
            uint32_t           registerPos,
            uint32_t           space) :
            name(name),
            type(type),
            tableSize(tableSize),
            registerPos(registerPos),
            space(space) {}
    };
} // namespace CDX12
