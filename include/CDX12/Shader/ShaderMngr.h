#pragma once

#include "BasicShader.h"

namespace Chen::CDX12 {
	class ShaderMngr
	{
	public:
		ShaderMngr(ID3D12Device* device) : device(device) {}
		~ShaderMngr();
		ShaderMngr(const ShaderMngr&) = delete;
		ShaderMngr& operator=(const ShaderMngr&) = delete;

		std::vector<std::string>& GetShaderNameList() { return nameList; }

		size_t GetShaderNum() { return mShaders.size(); }

		void CreateShader(
			const std::string& name,
			std::span<std::pair<std::string, Shader::Property> const> properties,
			const wchar_t* vsPath,
			const wchar_t* psPath,
			const wchar_t* hsPath = nullptr,
			const wchar_t* dsPath = nullptr,
			const std::vector<D3D_SHADER_MACRO> shaderDefines = std::vector<D3D_SHADER_MACRO>(),
			std::span<D3D12_STATIC_SAMPLER_DESC> samplers = GlobalSamplers::GetSamplers());

		void CreateShader(
			const std::string& name,
			std::span<std::pair<std::string, Shader::Property> const> properties,
			ComPtr<ID3D12RootSignature>&& rootSig,
			const wchar_t* vsPath,
			const wchar_t* psPath,
			const wchar_t* hsPath = nullptr,
			const wchar_t* dsPath = nullptr,
			const std::vector<D3D_SHADER_MACRO> shaderDefines = std::vector<D3D_SHADER_MACRO>());

		BasicShader* GetShader(std::string name) 
		{
			return (mShaders.find(name) != mShaders.end()) ? mShaders.at(name).get() : nullptr;
		}

	private:
		ID3D12Device* device;
		std::unordered_map<std::string, std::unique_ptr<BasicShader>> mShaders;
		std::vector<std::string> nameList;
	};
}
