/*
    RAII For CommandList
*/
#pragma once

#include <d3d12.h>

#include "DXUtil.h"

class CmdListHandle {
	ID3D12GraphicsCommandList* cmdList;

public:
	CmdListHandle(CmdListHandle&&);

	CmdListHandle(CmdListHandle const&) = delete;

	ID3D12GraphicsCommandList* CmdList() const { return cmdList; }

	CmdListHandle(
		ID3D12CommandAllocator* allocator,
		ID3D12GraphicsCommandList* cmdList);

	~CmdListHandle();
};
