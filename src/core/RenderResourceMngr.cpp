#include <CDX12/RenderResourceMngr.h>

using namespace Chen::CDX12;

void RenderResourceMngr::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	assert(!isInit);

	matMngr = new MaterialMngr();
	texMngr = new TextureMngr(device);
	meshMngr = new MeshMngr(device, cmdList);
	shaderMngr = new ShaderMngr(device);
	psoMngr = new PSOManager();

	isInit = true;
}

void RenderResourceMngr::Clear()
{
	isInit = false;

	delete matMngr;
	delete texMngr;
	delete meshMngr;
	delete shaderMngr;
	delete psoMngr;

	matMngr = nullptr;
	texMngr = nullptr;
	meshMngr = nullptr;
	shaderMngr = nullptr;
	psoMngr = nullptr;
}

RenderResourceMngr::~RenderResourceMngr()
{
	Clear();
}
