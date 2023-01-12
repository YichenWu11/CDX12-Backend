#pragma once

#include <ResourceUploadBatch.h>

#include "../DescripitorHeap/DescriptorHeapAllocation.h"
#include "Texture.h"

namespace Chen::CDX12 {
class TextureMngr {
public:
  enum class TexFileFormat : uint8_t { DDS = 0, WIC };

  TextureMngr(ID3D12Device *device, int size = 168);
  ~TextureMngr();
  TextureMngr(const TextureMngr &) = delete;
  TextureMngr &operator=(const TextureMngr &) = delete;

  size_t
  CreateTextureFromFile(ID3D12Device *device, ID3D12CommandQueue *cmdQueue,
                        const std::wstring &path, const std::string &name,
                        TexFileFormat format,
                        TextureDimension dimension = TextureDimension::Tex2D);

  size_t GetTextureIndex(const std::string &name) const {
    return (name2index.find(name) != name2index.end()) ? name2index.at(name)
                                                       : InvalidIndex;
  }

  Texture *GetTexture(const std::string &name) {
    return (mTextures.find(name) != mTextures.end()) ? mTextures.at(name).get()
                                                     : nullptr;
  }

  size_t GetTexNum() { return mTextures.size(); }

  std::vector<std::string> &GetTexNameList() { return nameList; }

  DescriptorHeapAllocation &GetTexAllocation() { return textureSrvAllocation; }

  void SetCubeIndex(size_t idx) { cubeIndex = idx; }
  size_t GetCubeIndex() { return cubeIndex; }

  void SetSMIndex(size_t idx) { shadowMapIndex = idx; }
  size_t GetSMIndex() { return shadowMapIndex; }

  void SetSSAOIdxStart(size_t idx) { mSsaoHeapIndexStart = idx; }
  size_t GetSSAOIdxStart() { return mSsaoHeapIndexStart; }

  void SetNullCubeIdx(size_t idx) { mNullCubeSrvIndex = idx; }
  size_t GetNullCubeIdx() { return mNullCubeSrvIndex; }

  void SetCGLutIndex(size_t idx) { cgLutIndex = idx; }
  size_t GetCGLutIndex() { return cgLutIndex; }

  std::string GetTexNameFromID(int id) {
    for (auto &iter : name2index) {
      if (iter.second == id)
        return iter.first;
    }
    return std::string();
  }

private:
  size_t InvalidIndex = -1;
  size_t cubeIndex = 0;
  size_t shadowMapIndex = 0;
  size_t cgLutIndex = 0;

  size_t mSsaoHeapIndexStart = 0;
  size_t mNullCubeSrvIndex = 0;

  std::unordered_map<std::string, size_t> name2index;
  std::map<std::string, std::unique_ptr<Texture>> mTextures;
  std::vector<std::string> nameList;

  DirectX::ResourceUploadBatch resourceUpload;
  DescriptorHeapAllocation textureSrvAllocation;
};
} // namespace Chen::CDX12
