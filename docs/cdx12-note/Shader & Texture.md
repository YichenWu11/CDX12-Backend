# Shader

在 `Shader` 这个类中,不仅包含二进制 `vertex shader`, `pixel shader`，同时也包含光栅状态，深度测试状态等，这种设计的思想在于，把 `Shader` 作为一个完整的状态保存器，外层绘制时直接用它内部的数据获取 `PSO` 用于绘制.

后面可能会从 `Shader` 派生出其他类, 比如光线追踪的 `Shader`, 仅用于计算的 `ComputeShader` 等

# GetRootSigFromShader

```cpp
std::vector<std::pair<std::string, Shader::Property>> rootProperties = {
    std::make_pair<std::string, Shader::Property>(
    "ObjectCB", Shader::Property{ShaderVariableType::ConstantBuffer, 0, 0, 1}
    ),
    std::make_pair<std::string, Shader::Property>(
    "PassCB", Shader::Property{ShaderVariableType::ConstantBuffer, 0, 1, 1}
    )
};

shader = std::make_unique<BasicShader>(rootProperties, mDevice.Get());

mRootSignature = shader->RootSig();
```

# Texture

