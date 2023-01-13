#include <CDX12/GCmdList.h>

#include <DirectXHelpers.h>

using namespace Chen::CDX12;

void GCmdList::Reset(ID3D12CommandAllocator* pAllocator) {
    ThrowIfFailed(raw->Reset(pAllocator, nullptr));
}

void GCmdList::ResourceBarrierTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
    DirectX::TransitionResource(raw.Get(), resource, from, to);
}

void GCmdList::RSSetViewport(D3D12_VIEWPORT viewport) {
    raw->RSSetViewports(1, &viewport);
}

void GCmdList::RSSetScissorRect(D3D12_RECT rect) {
    raw->RSSetScissorRects(1, &rect);
}

void GCmdList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT color[4]) {
    raw->ClearRenderTargetView(RenderTargetView, color, 0, nullptr);
}

void GCmdList::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView) {
    raw->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
}

void GCmdList::OMSetRenderTarget(
    D3D12_CPU_DESCRIPTOR_HANDLE rendertarget,
    D3D12_CPU_DESCRIPTOR_HANDLE depthstencil) {
    raw->OMSetRenderTargets(1, &rendertarget, true, &depthstencil);
}

void GCmdList::DrawIndexed(
    UINT IndexCount,
    UINT StartIndexLocation,
    INT  BaseVertexLocation) {
    raw->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
}
