#include <CDX12/Material/MaterialMngr.h>
#include <CDX12/Material/TextureMngr.h>
#include <CDX12/RenderResourceMngr.h>
#include <CDX12/Shader/PSOManager.h>
#include <CDX12/Shader/ShaderMngr.h>

using namespace Chen::CDX12;

void RenderResourceMngr::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    assert(!isInit);

    matMngr    = new MaterialMngr();
    texMngr    = new TextureMngr(device);
    shaderMngr = new ShaderMngr(device);
    psoMngr    = new PSOManager(device);

    isInit = true;
}

void RenderResourceMngr::Clear() {
    isInit = false;

    delete matMngr;
    delete texMngr;
    delete shaderMngr;
    delete psoMngr;

    matMngr    = nullptr;
    texMngr    = nullptr;
    shaderMngr = nullptr;
    psoMngr    = nullptr;
}

RenderResourceMngr::~RenderResourceMngr() {
    Clear();
}
