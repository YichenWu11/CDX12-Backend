/*
    RAII For CommandList
*/
#pragma once

#include <d3d12.h>

#include <CDX12/DXUtil.h>

namespace Chen::CDX12 {
    class CmdListHandle {
        ID3D12GraphicsCommandList* cmdList;

    public:
        CmdListHandle(CmdListHandle&&) noexcept;

        CmdListHandle(CmdListHandle const&) = delete;

        ID3D12GraphicsCommandList* CmdList() const { return cmdList; }

        CmdListHandle(
            ID3D12CommandAllocator*    allocator,
            ID3D12GraphicsCommandList* cmdList);

        ~CmdListHandle();
    };

} // namespace Chen::CDX12
