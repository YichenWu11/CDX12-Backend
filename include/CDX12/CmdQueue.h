#pragma once

#include "DXUtil.h"

namespace Chen::CDX12 {
	struct CmdQueue : ComPtrHolder<ID3D12CommandQueue> {
		using ComPtrHolder<ID3D12CommandQueue>::ComPtrHolder;
		void Execute(ID3D12GraphicsCommandList* list);
	};
}
