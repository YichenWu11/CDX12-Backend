#pragma once

#include "../Math/MathHelper.h"
#include "../DXUtil.h"

namespace Chen::CDX12 {
    // The Top Material class
    struct BasicMaterial
    {
        std::string Name;             // Unique material name for lookup.
        int MatIndex = -1;          // Index into constant buffer corresponding to this material.
        int DiffuseSrvHeapIndex = -1; // Index into SRV heap for diffuse texture.
        int NormalSrvHeapIndex = -1;  // Index into SRV heap for normal texture.
        DirectX::XMFLOAT4 DiffuseAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 FresnelR0{ 0.01f, 0.01f, 0.01f };
        float Roughness = 0.25f;
        DirectX::XMFLOAT4X4 MatTransform{Math::MathHelper::Identity4x4()};
    };

    // structed buffer content (will be passed to shader)
    struct BasicMaterialData
    {
        DirectX::XMFLOAT4 DiffuseAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 FresnelR0{ 0.01f, 0.01f, 0.01f };
        float Roughness = 0.5f;
        DirectX::XMFLOAT4X4 MatTransform{Math::MathHelper::Identity4x4()};
        UINT DiffuseMapIndex = 0;
        UINT NormalMapIndex = 0;
        /* data for alignment */
        UINT MaterialPad1;
        UINT MaterialPad2;
    };

    class MaterialMngr 
    {
        public:
            MaterialMngr()  = default;
            ~MaterialMngr() = default;
            MaterialMngr(const MaterialMngr&)            = delete;
            MaterialMngr& operator=(const MaterialMngr&) = delete;

            void CreateMaterial(
                const std::string& name,
                int DiffuseSrvHeapIndex,
                DirectX::XMFLOAT4 DiffuseAlbedo,
                DirectX::XMFLOAT3 FresnelR0,
                float Roughness,
                int NormalSrvHeapIndex = -1);

	        BasicMaterial* GetMaterial(const std::string &name) const;

            std::vector<std::string>& GetMatNameList() { return nameList; }

            std::map<std::string, std::unique_ptr<BasicMaterial>>& GetAllMats() { return mMaterials; }

        private:
            std::map<std::string, std::unique_ptr<BasicMaterial>> mMaterials;
            std::vector<std::string> nameList;
    };
}
