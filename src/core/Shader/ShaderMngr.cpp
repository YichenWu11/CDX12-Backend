#include <CDX12/Shader/ShaderMngr.h>

using namespace Chen::CDX12;

void ShaderMngr::CreateShader(
	const std::string& name,
	std::span<std::pair<std::string, Shader::Property> const> properties,
	const wchar_t* vsPath,
	const wchar_t* psPath,
	const wchar_t* hsPath,
	const wchar_t* dsPath,
	const std::vector<D3D_SHADER_MACRO> shaderDefines,
	std::span<D3D12_STATIC_SAMPLER_DESC> samplers)
{
	auto shader = std::make_unique<BasicShader>(properties, device, samplers);
	shader->SetVsShader(vsPath, shaderDefines);
	shader->SetPsShader(psPath, shaderDefines);
	shader->SetHsShader(hsPath, shaderDefines);
	shader->SetDsShader(dsPath, shaderDefines);

	mShaders[name] = std::move(shader);
	nameList.push_back(name);
}

void ShaderMngr::CreateShader(
	const std::string& name,
	std::span<std::pair<std::string, Shader::Property> const> properties,
	ComPtr<ID3D12RootSignature>&& rootSig,
	const wchar_t* vsPath,
	const wchar_t* psPath,
	const wchar_t* hsPath,
	const wchar_t* dsPath,
	const std::vector<D3D_SHADER_MACRO> shaderDefines)
{
	auto shader = std::make_unique<BasicShader>(properties, std::forward<ComPtr<ID3D12RootSignature>&&>(rootSig));
	shader->SetVsShader(vsPath, shaderDefines);
	shader->SetPsShader(psPath, shaderDefines);
	shader->SetHsShader(hsPath, shaderDefines);
	shader->SetDsShader(dsPath, shaderDefines);

	mShaders[name] = std::move(shader);
	nameList.push_back(name);	
}

ShaderMngr::~ShaderMngr() {}
