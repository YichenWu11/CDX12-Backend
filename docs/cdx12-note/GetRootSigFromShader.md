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

