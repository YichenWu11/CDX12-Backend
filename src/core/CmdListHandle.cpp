#include <CDX12/CmdListHandle.h>

namespace Chen::CDX12 {
    CmdListHandle::CmdListHandle(CmdListHandle&& v) noexcept :
        cmdList(v.cmdList) {
        v.cmdList = nullptr;
    }

    CmdListHandle::CmdListHandle(
        ID3D12CommandAllocator*    allocator,
        ID3D12GraphicsCommandList* cmdList) :
        cmdList(cmdList) {
        ThrowIfFailed(allocator->Reset());
        ThrowIfFailed(cmdList->Reset(allocator, nullptr));
    }

    CmdListHandle::~CmdListHandle() {
        if (cmdList)
            cmdList->Close();
    }

} // namespace Chen::CDX12
