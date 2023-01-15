#pragma once

#include <functional>

#include <CDX12/CmdListHandle.h>
#include <CDX12/DXUtil.h>
#include <CDX12/GCmdList.h>
#include <CDX12/Material/Texture.h>
#include <CDX12/Math/MathHelper.h>
#include <CDX12/Resource/DefaultBuffer.h>
#include <CDX12/Resource/Mesh.h>
#include <CDX12/Resource/ReadbackBuffer.h>
#include <CDX12/Resource/UploadBuffer.h>
#include <CDX12/Shader/BasicShader.h>
#include <CDX12/Shader/PSOManager.h>
#include <CDX12/Util/BindProperty.h>
#include <CDX12/Util/StackAllocator.h>

namespace Chen::CDX12 {
    class FrameResource {
    public:
        FrameResource(UINT64 cpuFence, ID3D12Fence* gpuFence, ID3D12Device* device);

        bool HaveResource(std::string_view name) const { return resourceMap.find(name) != resourceMap.end(); }

        template <typename T>
        FrameResource& RegisterResource(std::string name, T&& resource);

        FrameResource& UnregisterResource(std::string_view name);

        template <typename T>
        T& GetResource(std::string_view name);
        template <typename T>
        const T& GetResource(std::string_view name) const;

        GCmdList GetCmdList() { return cmdList; }

        // ***********************************************************************************

        CmdListHandle Command();
        void          AddDelayDisposeResource(ComPtr<ID3D12Resource> const& ptr);

        void Execute(ID3D12CommandQueue* queue);

        void       Upload(BufferView const& buffer, void const* src);
        void       Download(BufferView const& buffer, void* dst);
        BufferView AllocateConstBuffer(std::span<uint8_t const> data);
        void       CopyBuffer(
                  Buffer const* src,
                  Buffer const* dst,
                  uint64        srcOffset,
                  uint64        dstOffset,
                  uint64        byteSize);
        void SetRenderTarget(
            Texture const*                       tex,
            CD3DX12_CPU_DESCRIPTOR_HANDLE const* rtvHandle,
            CD3DX12_CPU_DESCRIPTOR_HANDLE const* dsvHandle = nullptr);
        void ClearRTV(CD3DX12_CPU_DESCRIPTOR_HANDLE const& rtv);
        void ClearDSV(CD3DX12_CPU_DESCRIPTOR_HANDLE const& dsv);

        void DrawMesh(
            BasicShader const*      shader,
            PSOManager*             psoManager,
            Mesh*                   mesh,
            DXGI_FORMAT             colorFormat,
            DXGI_FORMAT             depthFormat,
            std::span<BindProperty> properties);

    private:
        friend class FrameResourceMngr;

        static constexpr size_t TEMP_SIZE = 1024ull * 1024ull;

        template <typename T>
        class Visitor : public IStackAllocVisitor {
        public:
            FrameResource* self;
            uint64         Allocate(uint64 size) override;
            void           DeAllocate(uint64 handle) override;
        };
        Visitor<UploadBuffer>                 tempUBVisitor;
        Visitor<DefaultBuffer>                tempVisitor;
        Visitor<ReadbackBuffer>               tempRBVisitor;
        StackAllocator                        ubAlloc;
        StackAllocator                        rbAlloc;
        StackAllocator                        dbAlloc;
        ID3D12Device*                         device;
        std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;

        BufferView GetTempBuffer(size_t size, size_t align, StackAllocator& alloc);

        // cpu at frame [cpuFence] use the resources
        // when the frame complete (GPU instructions complete), gpuFence <- cpuFence
        void Signal(ID3D12CommandQueue* cmdQueue, UINT64 cpuFence);

        // at the cpu frame beginning, you should wait the frame complete
        // block cpu
        // run delay updator and unregister
        void BeginFrame(HANDLE sharedEventHandle);

        bool populated = false;

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
        GCmdList                                       cmdList;

        UINT64       cpuFence;
        ID3D12Fence* gpuFence;

        std::map<std::string, std::any, std::less<>> resourceMap;
        std::vector<ComPtr<ID3D12Resource>>          delayDisposeResources;

        std::vector<std::function<void()>> afterSyncEvents;
    };
} // namespace Chen::CDX12

#include "details/FrameResourceMngr.inl"