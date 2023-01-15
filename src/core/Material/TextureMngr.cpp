#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

#include <CDX12/DescriptorHeapMngr.h>
#include <CDX12/Material/TextureMngr.h>

using namespace Chen::CDX12;

TextureMngr::TextureMngr(ID3D12Device* device, int size) :
    resourceUpload(device) {
    textureSrvAllocation = DescriptorHeapMngr::GetInstance().GetCSUGpuDH()->Allocate(size);
}

TextureMngr::~TextureMngr() {
    if (!textureSrvAllocation.IsNull())
        DescriptorHeapMngr::GetInstance().GetCSUGpuDH()->Free(std::move(textureSrvAllocation));
}

size_t TextureMngr::CreateTextureFromFile(
    ID3D12Device*       device,
    ID3D12CommandQueue* cmdQueue,
    TextureCreateInfo   create_info,
    TexFileFormat       file_format) {
    if (texname2index.find(create_info.name) != texname2index.end())
        return InvalidIndex;

    resourceUpload.Begin();
    auto tex = std::make_unique<Texture>(
        device,
        create_info.width,
        create_info.height,
        create_info.format,
        create_info.dimension,
        create_info.depth,
        create_info.mip,
        create_info.usage,
        create_info.resourceState);

    switch (file_format) {
        case TexFileFormat::DDS:
            ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
                device,
                resourceUpload,
                create_info.path.c_str(),
                get_rvalue_ptr(tex->GetResource())));
            break;
        case TexFileFormat::WIC:
            ThrowIfFailed(DirectX::CreateWICTextureFromFile(
                device,
                resourceUpload,
                create_info.path.c_str(),
                get_rvalue_ptr(tex->GetResource())));
            break;
        default:
            return InvalidIndex;
    }

    texname2index[create_info.name] = mTextures.size();
    mTextures[create_info.name]     = std::move(tex);

    D3D12_SHADER_RESOURCE_VIEW_DESC desc = mTextures[create_info.name]->GetColorSrvDesc(0);
    device->CreateShaderResourceView(
        mTextures[create_info.name]->GetResource(),
        &desc,
        textureSrvAllocation.GetCpuHandle(texname2index[create_info.name]));

    auto uploadResourcesFinished = resourceUpload.End(cmdQueue);
    uploadResourcesFinished.wait();

    return texname2index[create_info.name];
}
