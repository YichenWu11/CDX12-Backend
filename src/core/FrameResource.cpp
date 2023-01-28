#include <CDX12/FrameResource.h>

namespace Chen::CDX12 {
    FrameResource::FrameResource(UINT64 cpuFence, ID3D12Fence* gpuFence, ID3D12Device* device) :
        ubAlloc(TEMP_SIZE, &tempUBVisitor),
        rbAlloc(TEMP_SIZE, &tempRBVisitor),
        dbAlloc(TEMP_SIZE, &tempVisitor),
        device(device),
        cpuFence{cpuFence},
        gpuFence{gpuFence} {
        tempUBVisitor.self = this;
        tempRBVisitor.self = this;
        tempVisitor.self   = this;
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(cmdAllocator.GetAddressOf())));

        ThrowIfFailed(device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(cmdList.raw.GetAddressOf())));

        ThrowIfFailed(cmdList->Close());
    }

    FrameResource::~FrameResource() {
    }

    void FrameResource::Execute(ID3D12CommandQueue* queue) {
        if (!populated) return;
        ID3D12CommandList* ppCommandLists[] = {cmdList.Get()};
        queue->ExecuteCommandLists(array_count(ppCommandLists), ppCommandLists);
    }

    void FrameResource::Signal(ID3D12CommandQueue* cmdQueue, UINT64 cpuFence) {
        this->cpuFence = cpuFence;
        cmdQueue->Signal(gpuFence, cpuFence);
    }

    void FrameResource::BeginFrame(HANDLE sharedEventHandle) {
        if (!populated || cpuFence == 0) return;
        if (gpuFence->GetCompletedValue() < cpuFence) {
            LPCWSTR falseValue  = 0;
            HANDLE  eventHandle = CreateEventEx(nullptr, falseValue, false, EVENT_ALL_ACCESS);
            ThrowIfFailed(gpuFence->SetEventOnCompletion(cpuFence, eventHandle));
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }

        delayDisposeResources.clear();

        for (auto&& i : afterSyncEvents) {
            i();
        }

        afterSyncEvents.clear();
        ubAlloc.Clear();
        dbAlloc.Clear();
        rbAlloc.Clear();
    }

    CmdListHandle FrameResource::Command() {
        populated = true;
        return {cmdAllocator.Get(), cmdList.Get()};
    }

    void FrameResource::AddDelayDisposeResource(ComPtr<ID3D12Resource> const& ptr) {
        delayDisposeResources.push_back(ptr);
    }

    template <typename T>
    uint64 FrameResource::Visitor<T>::Allocate(uint64 size) {
        auto packPtr = new T(
            self->device,
            size);
        return reinterpret_cast<uint64>(static_cast<Buffer*>(packPtr));
    }

    template <typename T>
    void FrameResource::Visitor<T>::DeAllocate(uint64 handle) {
        delete reinterpret_cast<T*>(handle);
    }

    BufferView FrameResource::GetTempBuffer(size_t size, size_t align, StackAllocator& alloc) {
        auto chunk = [&] {
            if (align <= 1) {
                return alloc.Allocate(size);
            }
            return alloc.Allocate(size, align);
        }();

        auto package = reinterpret_cast<UploadBuffer*>(chunk.handle);
        return {
            package,
            chunk.offset,
            size};
    }

    void FrameResource::Upload(BufferView const& buffer, void const* src) {
        auto uBuffer = GetTempBuffer(buffer.byteSize, 0, ubAlloc);
        static_cast<UploadBuffer const*>(uBuffer.buffer)
            ->CopyData(
                uBuffer.offset,
                {reinterpret_cast<vbyte const*>(src), size_t(uBuffer.byteSize)});
        CopyBuffer(
            uBuffer.buffer,
            buffer.buffer,
            uBuffer.offset,
            buffer.offset,
            buffer.byteSize);
    }

    BufferView FrameResource::AllocateConstBuffer(std::span<uint8_t const> data) {
        auto tempBuffer = GetTempBuffer(data.size(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, dbAlloc);
        Upload(tempBuffer, data.data());
        return tempBuffer;
    }

    void FrameResource::Download(BufferView const& buffer, void* dst) {
        auto rBuffer = GetTempBuffer(buffer.byteSize, 0, rbAlloc);
        afterSyncEvents.emplace_back([rBuffer, dst] {
            static_cast<ReadbackBuffer const*>(rBuffer.buffer)
                ->CopyData(
                    rBuffer.offset,
                    {reinterpret_cast<vbyte*>(dst), size_t(rBuffer.byteSize)});
        });
    }

    void FrameResource::CopyBuffer(
        Buffer const* src,
        Buffer const* dst,
        uint64        srcOffset,
        uint64        dstOffset,
        uint64        byteSize) {
        auto c = cmdList.Get();
        c->CopyBufferRegion(
            dst->GetResource(),
            dstOffset,
            src->GetResource(),
            srcOffset,
            byteSize);
    }

    void FrameResource::SetRenderTarget(
        Texture const*                       tex,
        CD3DX12_CPU_DESCRIPTOR_HANDLE const* rtvHandle,
        CD3DX12_CPU_DESCRIPTOR_HANDLE const* dsvHandle) {
        auto             desc = tex->GetResource()->GetDesc();
        CD3DX12_VIEWPORT viewPort(0.0f, 0.0f, desc.Width, desc.Height);
        CD3DX12_RECT     scissorRect(0, 0, desc.Width, desc.Height);
        cmdList->RSSetViewports(1, &viewPort);
        cmdList->RSSetScissorRects(1, &scissorRect);
        cmdList->OMSetRenderTargets(1, rtvHandle, FALSE, dsvHandle);
    }

    void FrameResource::ClearRTV(CD3DX12_CPU_DESCRIPTOR_HANDLE const& rtv) {
        cmdList.ClearRenderTargetView(rtv, Texture::CLEAR_COLOR);
    }

    void FrameResource::ClearDSV(CD3DX12_CPU_DESCRIPTOR_HANDLE const& dsv) {
        cmdList.ClearDepthStencilView(dsv);
    }

    void FrameResource::DrawMesh(
        BasicShader const*      shader,
        PSOManager*             psoManager,
        Mesh*                   mesh,
        DXGI_FORMAT             colorFormat,
        DXGI_FORMAT             depthFormat,
        std::span<BindProperty> properties) {
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->SetPipelineState(
            psoManager->GetPipelineState(
                mesh->Layout(),
                shader,
                {&colorFormat, 1},
                depthFormat));
        mesh->GetVertexBufferView(vertexBufferView);
        // Set vertex & index buffer
        cmdList->IASetVertexBuffers(0, vertexBufferView.size(), vertexBufferView.data());
        D3D12_INDEX_BUFFER_VIEW indexBufferView = mesh->GetIndexBufferView();
        cmdList->IASetIndexBuffer(&indexBufferView);

        struct PropertyBinder {
            ID3D12GraphicsCommandList* cmdList;
            Shader const*              shader;
            std::string const*         name;
            void                       operator()(BufferView const& bfView) const {
                                      if (!shader->SetResource(
                                              *name,
                                              cmdList,
                                              bfView)) {
                                          throw "Invalid shader binding";
                }
            }
            void operator()(DescriptorHeapAllocView const& descView) const {
                if (!shader->SetResource(
                        *name,
                        cmdList,
                        descView)) {
                    throw "Invalid shader binding";
                }
            }
        };

        cmdList->SetGraphicsRootSignature(shader->RootSig());
        PropertyBinder binder{
            .cmdList = cmdList.Get(),
            .shader  = shader};

        for (auto&& i : properties) {
            binder.name = &i.name;
            std::visit(binder, i.prop);
        }
        cmdList->DrawIndexedInstanced(
            mesh->IndexBuffer().GetByteSize() / 4,
            1,
            0,
            0,
            0);
    }

    IndirectDrawCommand FrameResource::getIndirectArguments(Mesh* mesh, D3D12_GPU_VIRTUAL_ADDRESS obj_cb_addr, uint32_t offset, uint32_t per_size) {
        UINT cb_byte_size = DXUtil::CalcConstantBufferByteSize(per_size);

        IndirectDrawCommand command;
        command.object_cbuffer_address = obj_cb_addr + cb_byte_size * offset;
        mesh->GetVertexBufferView(vertexBufferView);
        // TODO:
        command.vertex_buffer                   = vertexBufferView[0];
        command.index_buffer                    = mesh->GetIndexBufferView();
        command.draw_args.BaseVertexLocation    = 0;
        command.draw_args.IndexCountPerInstance = mesh->IndexBuffer().GetByteSize() / 4;
        command.draw_args.InstanceCount         = 1;
        command.draw_args.StartIndexLocation    = 0;
        command.draw_args.StartInstanceLocation = 0;

        return command;
    }

    void FrameResource::DrawMeshIndirect(
        BasicShader const*                        shader,
        PSOManager*                               psoManager,
        std::span<D3D12_INPUT_ELEMENT_DESC const> layout,
        DXGI_FORMAT                               colorFormat,
        DXGI_FORMAT                               depthFormat,
        std::span<BindProperty>                   properties,
        UploadBuffer*                             indirectDrawBuffer,
        uint32_t                                  draw_count,
        ID3D12CommandSignature*                   command_sign) {
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->SetPipelineState(
            psoManager->GetPipelineState(
                layout,
                shader,
                {&colorFormat, 1},
                depthFormat));

        struct PropertyBinder {
            ID3D12GraphicsCommandList* cmdList;
            Shader const*              shader;
            std::string const*         name;
            void                       operator()(BufferView const& bfView) const {
                                      if (!shader->SetResource(
                                              *name,
                                              cmdList,
                                              bfView)) {
                                          throw "Invalid shader binding";
                }
            }
            void operator()(DescriptorHeapAllocView const& descView) const {
                if (!shader->SetResource(
                        *name,
                        cmdList,
                        descView)) {
                    throw "Invalid shader binding";
                }
            }
        };

        cmdList->SetGraphicsRootSignature(shader->RootSig());
        PropertyBinder binder{
            .cmdList = cmdList.Get(),
            .shader  = shader};

        for (auto&& i : properties) {
            binder.name = &i.name;
            std::visit(binder, i.prop);
        }

        cmdList->ExecuteIndirect(
            command_sign,
            draw_count,
            indirectDrawBuffer->GetResource(),
            0,
            nullptr,
            0);
    }
} // namespace Chen::CDX12
