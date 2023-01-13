#pragma once

#include <deque>
#include <map>

namespace Chen::CDX12 {
    // The class handles free memory block management to accommodate variable-size allocation requests.
    // It keeps track of free blocks only and does not record allocation sizes. The class uses two ordered maps
    // to facilitate operations. The first map keeps blocks sorted by their offsets. The second multimap keeps blocks
    // sorted by their sizes. The elements of the two maps reference each other, which enables efficient block
    // insertion, removal and merging.
    //
    //   8                 32                       64                           104
    //   |<---16--->|       |<-----24------>|        |<---16--->|                 |<-----32----->|
    //
    //
    //          size2freeblock         offset2freeblock
    //           size->offset            offset->size
    //
    //                16 ------------------>   8 ---------->  {size = 16, &size2freeblock[0]}
    //
    //                16 ------.   .------->  32 ---------->  {size = 24, &size2freeblock[2]}
    //                          '.'
    //                24 -------' '-------->  64 ---------->  {size = 16, &size2freeblock[1]}
    //
    //                32 ------------------> 104 ---------->  {size = 32, &size2freeblock[3]}
    //

    // 可变长度分配器Manager
    class VarSizeAllocMngr {
    public:
        using OffsetType = size_t;

    private:
        struct FreeBlockInfo; // as its name

        // Type of the map that keeps memory blocks sorted by their offsets
        using TFreeBlocksByOffsetMap = std::map<OffsetType, FreeBlockInfo>;

        // Type of the map that keeps memory blocks sorted by their sizes
        using TFreeBlocksBySizeMap = std::multimap<OffsetType, TFreeBlocksByOffsetMap::iterator>;

        struct FreeBlockInfo {
            // Block size (no reserved space for the size of allocation)
            // actually Size == OrderBySizeIt->first
            OffsetType Size;
            // Iterator referencing this block in the multimap sorted by the block size
            TFreeBlocksBySizeMap::iterator OrderBySizeIt;

            FreeBlockInfo(OffsetType _Size) :
                Size(_Size) {}
        };

    public:
        VarSizeAllocMngr(OffsetType capacity);

        VarSizeAllocMngr(VarSizeAllocMngr&&) noexcept;

        VarSizeAllocMngr& operator=(VarSizeAllocMngr&&) noexcept;

        VarSizeAllocMngr(const VarSizeAllocMngr&)            = delete;
        VarSizeAllocMngr& operator=(const VarSizeAllocMngr&) = delete;

        void AddNewBlock(OffsetType Offset, OffsetType Size);

        OffsetType Allocate(OffsetType Size);

        void Free(OffsetType Offset, OffsetType Size);

        OffsetType GetFreeSize() const { return freeSize; }
        OffsetType GetCapacity() const { return capacity; }

        // InvalidOffset For Allocate
        static OffsetType InvalidOffset;

    private:
        TFreeBlocksByOffsetMap m_FreeBlocksByOffset;
        TFreeBlocksBySizeMap   m_FreeBlocksBySize;

        OffsetType capacity = 0;
        OffsetType freeSize = 0;
    };

    // Variable Size GPU Allocations Manager
    class VarSizeGPUAllocMngr : public VarSizeAllocMngr {
    private:
        struct FreedAllocationInfo {
            FreedAllocationInfo(OffsetType offset, OffsetType size, uint64_t num) :
                Offset(offset), Size(size), FrameNumber(num) {}

            OffsetType Offset;
            OffsetType Size;
            uint64_t   FrameNumber;
        };

    public:
        VarSizeGPUAllocMngr(OffsetType capacity);

        ~VarSizeGPUAllocMngr() = default;

        VarSizeGPUAllocMngr(VarSizeGPUAllocMngr&& rhs);

        VarSizeGPUAllocMngr& operator=(VarSizeGPUAllocMngr&& rhs)  = default;
        VarSizeGPUAllocMngr(const VarSizeGPUAllocMngr&)            = delete;
        VarSizeGPUAllocMngr& operator=(const VarSizeGPUAllocMngr&) = delete;

        void Free(OffsetType Offset, OffsetType Size, uint64_t FrameNumber) {
            // Do not release the block immediately, but add
            // it to the queue instead
            m_StaleAllocations.emplace_back(Offset, Size, FrameNumber);
        }

        void ReleaseCompletedFrames(uint64_t NumCompletedFrames) {
            // Free all allocations from the beginning of the queue that belong to completed frames
            while (!m_StaleAllocations.empty() && m_StaleAllocations.front().FrameNumber < NumCompletedFrames) {
                auto& OldestAllocation = m_StaleAllocations.front();
                VarSizeAllocMngr::Free(OldestAllocation.Offset, OldestAllocation.Size);
                m_StaleAllocations.pop_front();
            }
        }

        static OffsetType InvalidOffset;

    private:
        // record the Allocation Blocks that will be freed
        std::deque<FreedAllocationInfo> m_StaleAllocations;
    };
} // namespace Chen::CDX12
