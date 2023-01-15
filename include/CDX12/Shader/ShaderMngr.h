#include <CDX12/Shader/Shader.h>

namespace Chen::CDX12 {
    class ShaderMngr {
    public:
        ShaderMngr(ID3D12Device* _device);
        ~ShaderMngr() = default;

        void CreateShader(
            const std::string&                                        shader_name,
            std::span<std::pair<std::string, Shader::Property> const> properties,
            std::span<D3D12_STATIC_SAMPLER_DESC>                      samplers = GlobalSamplers::GetSamplers());

        void CreateShader(
            const std::string&                                        shader_name,
            std::span<std::pair<std::string, Shader::Property> const> properties,
            ComPtr<ID3D12RootSignature>&&                             rootSig);

        Shader* GetShader(const std::string& shader_name);

    private:
        ID3D12Device* device;

        std::map<std::string, std::unique_ptr<Shader>> m_shader_map;
    };
} // namespace Chen::CDX12