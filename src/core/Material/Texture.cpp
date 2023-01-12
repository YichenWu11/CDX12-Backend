#include <CDX12/Material/Texture.h>
#include <CDX12/GeneralDesc.h>

using namespace Chen::CDX12;

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetTexSrvDesc() 
{
    D3D12_SHADER_RESOURCE_VIEW_DESC texDesc;
    memset(&texDesc, 0, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
    switch (dimension) {
        case TextureDimension::Cubemap:
            texDesc = Desc::SRV::TexCube(Resource->GetDesc().Format);
            break;
        case TextureDimension::Tex2D:
            texDesc = Desc::SRV::Tex2D(Resource->GetDesc().Format);
            break;
        case TextureDimension::Tex3D:
            texDesc = Desc::SRV::Tex3D(Resource->GetDesc().Format);
            break;
        case TextureDimension::Tex1D: 
        {
            texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            texDesc.Format = Resource->GetDesc().Format;
            texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            texDesc.Texture1D.MostDetailedMip = 0;
	        texDesc.Texture1D.ResourceMinLODClamp = 0.0f;
            break;
        }
        case TextureDimension::Tex2DArray: 
        {
            texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            texDesc.Format = Resource->GetDesc().Format;
            texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            texDesc.Texture2DArray.MostDetailedMip = 0;
	        texDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            break;
        }
    }
    return texDesc;
}
