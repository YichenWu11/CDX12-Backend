#include <CDX12/Material/MaterialMngr.h>
#include <CDX12/Material/TextureMngr.h>
#include <CDX12/RenderResourceMngr.h>
#include <CDX12/Resource/MeshMngr.h>
#include <CDX12/Shader/PSOManager.h>
#include <CDX12/Shader/ShaderMngr.h>

using namespace Chen::CDX12;

void RenderResourceMngr::Init(ID3D12Device* device) {
    assert(!isInit);

    matMngr    = new MaterialMngr();
    texMngr    = new TextureMngr(device);
    psoMngr    = new PSOManager(device);
    meshMngr   = new MeshMngr(device);
    shaderMngr = new ShaderMngr(device);

    isInit = true;
}

void RenderResourceMngr::Clear() {
    isInit = false;

    delete matMngr;
    delete texMngr;
    delete psoMngr;
    delete meshMngr;
    delete shaderMngr;

    matMngr    = nullptr;
    texMngr    = nullptr;
    psoMngr    = nullptr;
    meshMngr   = nullptr;
    shaderMngr = nullptr;
}

RenderResourceMngr::~RenderResourceMngr() {
    Clear();
}
