#include <CDX12/VarSizeAllocMngr.h>

#include <cassert>
#include <utility>


using namespace Chen::CDX12;

VarSizeAllocMngr::OffsetType VarSizeAllocMngr::InvalidOffset = -1;

VarSizeAllocMngr::VarSizeAllocMngr(size_t capacity) :
    capacity(capacity),
    freeSize(capacity) {
    // Insert single maximum-size block
    AddNewBlock(0, capacity);
}

VarSizeAllocMngr::VarSizeAllocMngr(VarSizeAllocMngr&& rhs) noexcept :
    m_FreeBlocksByOffset{std::move(rhs.m_FreeBlocksByOffset)},
    m_FreeBlocksBySize{std::move(rhs.m_FreeBlocksBySize)},
    capacity{rhs.capacity},
    freeSize{rhs.freeSize} {
    rhs.capacity = 0;
    rhs.freeSize = 0;
}

VarSizeAllocMngr& VarSizeAllocMngr::operator=(VarSizeAllocMngr&& rhs) noexcept {
    m_FreeBlocksByOffset = std::move(rhs.m_FreeBlocksByOffset);
    m_FreeBlocksBySize   = std::move(rhs.m_FreeBlocksBySize);
    capacity             = rhs.capacity;
    freeSize             = rhs.freeSize;

    rhs.capacity = 0;
    rhs.freeSize = 0;

    return *this;
}

void VarSizeAllocMngr::AddNewBlock(OffsetType Offset, OffsetType Size) {
    auto NewBlockIt = m_FreeBlocksByOffset.emplace(Offset, Size);
    auto OrderIt    = m_FreeBlocksBySize.emplace(Size, NewBlockIt.first);
    // set the FreeBlockInfo
    NewBlockIt.first->second.OrderBySizeIt = OrderIt;
}

VarSizeAllocMngr::OffsetType VarSizeAllocMngr::Allocate(OffsetType Size) {
    if (freeSize < Size)
        return InvalidOffset;

    // Get the first block that is large enough to encompass Size bytes
    auto SmallestBlockItIt = m_FreeBlocksBySize.lower_bound(Size);
    if (SmallestBlockItIt == m_FreeBlocksBySize.end())
        return InvalidOffset;

    auto SmallestBlockIt = SmallestBlockItIt->second;
    auto Offset          = SmallestBlockIt->first;
    auto NewOffset       = Offset + Size;
    auto NewSize         = SmallestBlockIt->second.Size - Size;
    m_FreeBlocksBySize.erase(SmallestBlockItIt);
    m_FreeBlocksByOffset.erase(SmallestBlockIt);
    if (NewSize > 0)
        AddNewBlock(NewOffset, NewSize);

    freeSize -= Size;
    return Offset;
}

void VarSizeAllocMngr::Free(OffsetType Offset, OffsetType Size) {
    // Find the first element whose offset is greater than the specified offset
    auto NextBlockIt = m_FreeBlocksByOffset.upper_bound(Offset);
    auto PrevBlockIt = NextBlockIt;
    if (PrevBlockIt != m_FreeBlocksByOffset.begin())
        --PrevBlockIt;
    else
        PrevBlockIt = m_FreeBlocksByOffset.end();
    OffsetType NewSize, NewOffset;
    if (PrevBlockIt != m_FreeBlocksByOffset.end() && Offset == PrevBlockIt->first + PrevBlockIt->second.Size) {
        // PrevBlock.Offset           Offset
        // |                          |
        // |<-----PrevBlock.Size----->|<------Size-------->|
        //
        NewSize   = PrevBlockIt->second.Size + Size;
        NewOffset = PrevBlockIt->first;

        if (NextBlockIt != m_FreeBlocksByOffset.end() && Offset + Size == NextBlockIt->first) {
            // PrevBlock.Offset           Offset               NextBlock.Offset
            // |                          |                    |
            // |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|
            //
            NewSize += NextBlockIt->second.Size;
            m_FreeBlocksBySize.erase(PrevBlockIt->second.OrderBySizeIt);
            m_FreeBlocksBySize.erase(NextBlockIt->second.OrderBySizeIt);
            // Delete the range of two blocks
            ++NextBlockIt;
            m_FreeBlocksByOffset.erase(PrevBlockIt, NextBlockIt);
        }
        else {
            // PrevBlock.Offset           Offset                       NextBlock.Offset
            // |                          |                            |
            // |<-----PrevBlock.Size----->|<------Size-------->| ~ ~ ~ |<-----NextBlock.Size----->|
            //
            m_FreeBlocksBySize.erase(PrevBlockIt->second.OrderBySizeIt);
            m_FreeBlocksByOffset.erase(PrevBlockIt);
        }
    }
    else if (NextBlockIt != m_FreeBlocksByOffset.end() && Offset + Size == NextBlockIt->first) {
        // PrevBlock.Offset                   Offset               NextBlock.Offset
        // |                                  |                    |
        // |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->|<-----NextBlock.Size----->|
        //
        NewSize   = Size + NextBlockIt->second.Size;
        NewOffset = Offset;
        m_FreeBlocksBySize.erase(NextBlockIt->second.OrderBySizeIt);
        m_FreeBlocksByOffset.erase(NextBlockIt);
    }
    else {
        // PrevBlock.Offset                   Offset                       NextBlock.Offset
        // |                                  |                            |
        // |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->| ~ ~ ~ |<-----NextBlock.Size----->|
        //
        NewSize   = Size;
        NewOffset = Offset;
    }

    AddNewBlock(NewOffset, NewSize);

    freeSize += Size;
}

VarSizeGPUAllocMngr::OffsetType VarSizeGPUAllocMngr::InvalidOffset = -1;

VarSizeGPUAllocMngr::VarSizeGPUAllocMngr(OffsetType capacity) :
    VarSizeAllocMngr(capacity) {}

VarSizeGPUAllocMngr::VarSizeGPUAllocMngr(VarSizeGPUAllocMngr&& rhs) :
    VarSizeAllocMngr(std::move(rhs)),
    m_StaleAllocations(std::move(rhs.m_StaleAllocations)) {
}
