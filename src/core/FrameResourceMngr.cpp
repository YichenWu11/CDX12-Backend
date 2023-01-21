#include <CDX12/FrameResourceMngr.h>

using namespace Chen::CDX12;

FrameResourceMngr::FrameResourceMngr(size_t numFrame, ID3D12Device* device) {
    assert(numFrame > 0 && device);
    ThrowIfFailed(device->CreateFence(cpuFence, D3D12_FENCE_FLAG_NONE,
                                      IID_PPV_ARGS(&gpuFence)));
    for (size_t i = 0; i < numFrame; i++)
        frameResources.push_back(std::make_unique<FrameResource>(cpuFence, gpuFence.Get(), device));
    eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
}

FrameResourceMngr::~FrameResourceMngr() {
    CloseHandle(eventHandle);
}

FrameResource* FrameResourceMngr::GetCurrentFrameResource() noexcept {
    return frameResources[cpuFence % frameResources.size()].get();
}

void FrameResourceMngr::BeginFrame() {
    GetCurrentFrameResource()->BeginFrame(eventHandle);
}

void FrameResourceMngr::Execute(ID3D12CommandQueue* queue) {
    GetCurrentFrameResource()->Execute(queue);
}

void FrameResourceMngr::EndFrame(ID3D12CommandQueue* cmdQueue) {
    GetCurrentFrameResource()->Signal(cmdQueue, ++cpuFence);
}

void FrameResourceMngr::CleanUp() {
    for (auto&& i : frameResources)
        i->BeginFrame(eventHandle);
}
