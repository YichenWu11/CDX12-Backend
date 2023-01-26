#include <CDX12/Device.h>

using namespace Chen::CDX12;

void Device::CreateCommittedResource(
    D3D12_HEAP_TYPE  heap_type,
    SIZE_T           size,
    ID3D12Resource** resources) {
    // 默认堆
    const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto bufferDesc            = CD3DX12_RESOURCE_DESC::Buffer(size);
    ThrowIfFailed(raw->CreateCommittedResource(
        &defaultHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(resources)));
}

// create shader-visible Descriptor Heap
void Device::CreateDescriptorHeap(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap** pHeap) {
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = size;
    cbvHeapDesc.Type           = type;
    cbvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask       = 0;
    ThrowIfFailed(raw->CreateDescriptorHeap(&cbvHeapDesc,
                                            IID_PPV_ARGS(pHeap)));
}

// create RootSignature
ComPtr<ID3D12RootSignature> Device::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc) {
    Microsoft::WRL::ComPtr<ID3DBlob> pSigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
    HRESULT                          hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        return nullptr;
    }
    Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSig;
    ThrowIfFailed(raw->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig)));
    return pRootSig;
}

HRESULT Device::Create(IUnknown* pAdapter, D3D_FEATURE_LEVEL level) {
    HRESULT hardwareResult = D3D12CreateDevice(
        pAdapter, // default adapter
        level,
        IID_PPV_ARGS(raw.GetAddressOf()));

    return hardwareResult;
}
