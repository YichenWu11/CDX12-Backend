#include <CDX12/Material/TextureMngr.h>
#include <CDX12/DescriptorHeapMngr.h>
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

using namespace Chen::CDX12;

TextureMngr::TextureMngr(ID3D12Device* device, int size) : resourceUpload(device)
{
    textureSrvAllocation = DescriptorHeapMngr::GetInstance().GetCSUGpuDH()->Allocate(size);
}

TextureMngr::~TextureMngr()
{
    if (!textureSrvAllocation.IsNull())
        DescriptorHeapMngr::GetInstance().GetCSUGpuDH()->Free(std::move(textureSrvAllocation));
}

size_t TextureMngr::CreateTextureFromFile(
    ID3D12Device* device,
    ID3D12CommandQueue* cmdQueue,
    const std::wstring& path,
    const std::string& name,
    TexFileFormat format,
    TextureDimension dimension)
{
    if (name2index.find(name) != name2index.end()) return InvalidIndex;  // name_only
    resourceUpload.Begin();
    auto tex = std::make_unique<Texture>(dimension);
    tex->Filename = path;
    tex->Name = name;

    if (format == TexFileFormat::WIC)
    {
        ThrowIfFailed(DirectX::CreateWICTextureFromFile(
            device,
            resourceUpload,
            path.c_str(),
            tex->Resource.ReleaseAndGetAddressOf()));
    }
    else if (format == TexFileFormat::DDS)
    {
        ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
            device,
            resourceUpload,
            path.c_str(),
            tex->Resource.ReleaseAndGetAddressOf()));
    }
    else
    {
        return InvalidIndex;
    }

    name2index[name] = mTextures.size();
    mTextures[name] = std::move(tex);
    nameList.push_back(name);

    D3D12_SHADER_RESOURCE_VIEW_DESC desc = mTextures[name]->GetTexSrvDesc();
    device->CreateShaderResourceView(
        mTextures[name]->Resource.Get(),
        &desc,
        textureSrvAllocation.GetCpuHandle(name2index[name])
    );

    auto uploadResourcesFinished = resourceUpload.End(cmdQueue);
    uploadResourcesFinished.wait();

    return name2index[name];
}
