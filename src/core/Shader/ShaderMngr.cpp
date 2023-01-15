#include <CDX12/Shader/ShaderMngr.h>

namespace Chen::CDX12 {
    ShaderMngr::ShaderMngr(ID3D12Device* _device) :
        device(_device) {}

    void ShaderMngr::CreateShader(
        const std::string&                                        shader_name,
        std::span<std::pair<std::string, Shader::Property> const> properties,
        std::span<D3D12_STATIC_SAMPLER_DESC>                      samplers) {
        if (m_shader_map.contains(shader_name))
            return;

        auto shader = std::make_unique<Shader>(properties, device, samplers);

        m_shader_map[shader_name] = std::move(shader);
    }

    void ShaderMngr::CreateShader(
        const std::string&                                        shader_name,
        std::span<std::pair<std::string, Shader::Property> const> properties,
        ComPtr<ID3D12RootSignature>&&                             rootSig) {
        if (m_shader_map.contains(shader_name))
            return;

        auto shader = std::make_unique<Shader>(properties, std::forward<ComPtr<ID3D12RootSignature>&&>(rootSig));

        m_shader_map[shader_name] = std::move(shader);
    }

    Shader* ShaderMngr::GetShader(const std::string& shader_name) {
        if (m_shader_map.contains(shader_name))
            return m_shader_map[shader_name].get();
        return nullptr;
    }
} // namespace Chen::CDX12