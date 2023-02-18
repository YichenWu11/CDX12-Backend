#pragma once

#include <functional>

#include <CDX12/CmdListHandle.h>
#include <CDX12/DXUtil.h>
#include <CDX12/GCmdList.h>
#include <CDX12/Math/MathHelper.h>
#include <CDX12/Resource/DefaultBuffer.h>
#include <CDX12/Resource/Mesh.h>
#include <CDX12/Resource/ReadbackBuffer.h>
#include <CDX12/Resource/Texture.h>
#include <CDX12/Resource/UploadBuffer.h>
#include <CDX12/Shader/BasicShader.h>
#include <CDX12/Shader/PSOManager.h>
#include <CDX12/Util/BindProperty.h>
#include <CDX12/Util/StackAllocator.h>


namespace Chen::CDX12 {
    struct IndirectDrawCommand {
        D3D12_GPU_VIRTUAL_ADDRESS    object_cbuffer_address; // Object Constant Buffer Address
        D3D12_VERTEX_BUFFER_VIEW     vertex_buffer;          // Vertex Buffer Address
        D3D12_INDEX_BUFFER_VIEW      index_buffer;           // Index Buffer Address
        D3D12_DRAW_INDEXED_ARGUMENTS draw_args;              // Draw Arguments
    };

    class FrameResource {
        friend class FrameResourceMngr;

    public:
        FrameResource(UINT64 cpuFence, ID3D12Fence* gpuFence, ID3D12Device* device);
        ~FrameResource();

        GCmdList      GetCmdList() { return cmdList; }
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

        void DrawMeshIndirect(
            BasicShader const*                        shader,
            PSOManager*                               psoManager,
            std::span<D3D12_INPUT_ELEMENT_DESC const> layout,
            DXGI_FORMAT                               colorFormat,
            DXGI_FORMAT                               depthFormat,
            std::span<BindProperty>                   properties,
            UploadBuffer*                             indirectDrawBuffer,
            uint32_t                                  mesh_count,
            ID3D12CommandSignature*                   command_sign);

        Chen::CDX12::IndirectDrawCommand getIndirectArguments(Chen::CDX12::Mesh* mesh, D3D12_GPU_VIRTUAL_ADDRESS, uint32_t offset, uint32_t per_size);

    private:
        static constexpr size_t TEMP_SIZE = 1024ull * 1024ull;

        template <typename T>
        class Visitor : public IStackAllocVisitor {
        public:
            FrameResource* self;
            uint64         Allocate(uint64 size) override;
            void           DeAllocate(uint64 handle) override;
        };

        BufferView GetTempBuffer(size_t size, size_t align, StackAllocator& alloc);

        // cpu at frame [cpuFence] use the resources
        // when the frame complete (GPU instructions complete), gpuFence <- cpuFence
        void Signal(ID3D12CommandQueue* cmdQueue, UINT64 cpuFence);

        // at the cpu frame beginning, you should wait the frame complete
        // block cpu
        // run delay updator and unregister
        void BeginFrame(HANDLE sharedEventHandle);

    private:
        Visitor<UploadBuffer>                 tempUBVisitor;
        Visitor<DefaultBuffer>                tempVisitor;
        Visitor<ReadbackBuffer>               tempRBVisitor;
        StackAllocator                        ubAlloc;
        StackAllocator                        rbAlloc;
        StackAllocator                        dbAlloc;
        ID3D12Device*                         device;
        std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;

        bool populated = false;

        GCmdList                                       cmdList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;

        UINT64       cpuFence;
        ID3D12Fence* gpuFence;

        std::vector<ComPtr<ID3D12Resource>> delayDisposeResources;

        std::vector<std::function<void()>> afterSyncEvents;
    };
} // namespace Chen::CDX12