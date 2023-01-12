#include <CDX12/FrameResource.h>

using namespace Chen::CDX12;

void FrameResource::Signal(ID3D12CommandQueue* cmdQueue, UINT64 cpuFence) {
    this->cpuFence = cpuFence;
    cmdQueue->Signal(gpuFence, cpuFence);
}

void FrameResource::BeginFrame(HANDLE sharedEventHandle) {
    if (gpuFence->GetCompletedValue() < cpuFence) {
        ThrowIfFailed(gpuFence->SetEventOnCompletion(cpuFence, sharedEventHandle));
        WaitForSingleObject(sharedEventHandle, INFINITE);
    }
}

FrameResource& FrameResource::UnregisterResource(std::string_view name) {
    assert(HaveResource(name));

    resourceMap.erase(resourceMap.find(name));

    return *this;
}
