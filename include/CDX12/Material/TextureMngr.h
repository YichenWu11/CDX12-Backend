#pragma once

#include <ResourceUploadBatch.h>

#include <CDX12/DescripitorHeap/DescriptorHeapAllocation.h>
#include <CDX12/Material/Texture.h>

namespace Chen::CDX12 {
    struct TextureCreateInfo {
        const std::wstring&   path;
        const std::string&    name;
        uint                  width;
        uint                  height;
        DXGI_FORMAT           format;
        TextureDimension      dimension;
        uint                  depth;
        uint                  mip;
        Texture::TextureUsage usage;
        D3D12_RESOURCE_STATES resourceState;
    };

    class TextureMngr {
    public:
        enum class TexFileFormat : uint8_t { DDS = 0,
                                             WIC };

        TextureMngr(ID3D12Device* device, int size = 168);
        ~TextureMngr();
        TextureMngr(const TextureMngr&)            = delete;
        TextureMngr& operator=(const TextureMngr&) = delete;

        size_t CreateTextureFromFile(ID3D12Device*       device,
                                     ID3D12CommandQueue* cmdQueue,
                                     TextureCreateInfo   create_info,
                                     TexFileFormat       file_format);

        size_t GetTextureIndex(const std::string& name) const {
            return (texname2index.find(name) != texname2index.end()) ? texname2index.at(name) : InvalidIndex;
        }

        Texture* GetTexture(const std::string& name) {
            return (mTextures.find(name) != mTextures.end()) ? mTextures.at(name).get() : nullptr;
        }

        size_t GetTexNum() { return mTextures.size(); }

        DescriptorHeapAllocation& GetTexAllocation() { return textureSrvAllocation; }

        std::string GetTexNameFromIdx(int id) {
            for (auto& iter : texname2index) {
                if (iter.second == id)
                    return iter.first;
            }
            return std::string();
        }

    private:
        size_t InvalidIndex = -1;

        std::unordered_map<std::string, size_t>         texname2index;
        std::map<std::string, std::unique_ptr<Texture>> mTextures;

        DirectX::ResourceUploadBatch resourceUpload;
        DescriptorHeapAllocation     textureSrvAllocation;
    };
} // namespace Chen::CDX12
