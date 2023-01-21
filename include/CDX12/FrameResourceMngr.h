#pragma once

#include <CDX12/FrameResource.h>

namespace Chen::CDX12 {
    class FrameResourceMngr {
    public:
        FrameResourceMngr(size_t numFrame, ID3D12Device*);
        ~FrameResourceMngr();
        size_t         GetNumFrame() const noexcept { return frameResources.size(); }
        FrameResource* GetCurrentFrameResource() noexcept;
        size_t         GetCurrentCpuFence() const noexcept { return cpuFence; };
        size_t         GetCurrentIndex() const noexcept { return (cpuFence % frameResources.size()); }

        void BeginFrame();
        void EndFrame(ID3D12CommandQueue*);
        void Execute(ID3D12CommandQueue* queue);

        void CleanUp();

    private:
        HANDLE                                      eventHandle;
        std::vector<std::unique_ptr<FrameResource>> frameResources;
        size_t                                      cpuFence{0};
        Microsoft::WRL::ComPtr<ID3D12Fence>         gpuFence;
    };
} // namespace Chen::CDX12
