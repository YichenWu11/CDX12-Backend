# Resource Binding in HLSL

着色器模型 5 (SM5.0) 资源语法使用 register 关键字将有关资源的重要信息中继到 HLSL 编译器。 例如，以下语句声明在槽 t3、t4、t5 和 t6 中绑定的四个纹理的数组。 t3 是唯一显示在该语句中的寄存器槽，而它仅仅是四个寄存器的数组中的第一个。

```cpp
    Texture2D<float4> tex1[4] : register(t3)
```

HLSL 中的着色器模型 5.1 (SM5.1) 资源语法基于现有的寄存器资源语法，旨在更方便地完成移植。HLSL 中的 Direct3D 12 资源绑定到逻辑寄存器空间中的虚拟寄存器：

* t – 表示着色器资源视图 (SRV)
* s – 表示采样器
* u – 表示无序访问视图 (UAV)
* b – 表示常量缓冲区视图 (CBV)

---

## 描述符数组和纹理数组

自 DirectX 10 以来一直可以使用纹理数组。纹理数组需要一个描述符，但是，所有数组切片必须采用相同的格式、宽度、高度和 mip 计数。此外，该数组必须占用虚拟地址空间中的一个连续范围。以下代码演示了从着色器访问纹理数组的示例。

```cpp
    Texture2DArray<float4> myTex2DArray : register(t0); // t0
    float3 myCoord(1.0f,1.4f,2.2f); // 2.2f is array index (rounded to int)
    color = myTex2DArray.Sample(mySampler, myCoord);
```

---

## 常量缓冲区

与 SM5.0 相比，SM5.1 常量缓冲区 (cbuffer) 语法已发生更改，可让开发人员为常量缓冲区编制索引。 为了启用可编制索引的常量缓冲区，SM5.1 引入了 ConstantBuffer“模板”构造：

```cpp
    struct Foo
    {
        float4 a;
        int2 b;
    };
    ConstantBuffer<Foo> myCB1[2][3] : register(b2, space1);
    ConstantBuffer<Foo> myCB2 : register(b0, space1);
```

为实现向后兼容，SM5.1 支持标量 cbuffers 的旧式 cbuffer 概念, 但是，此类旧式 cbuffer 不可编制索引。

```cpp
    cbuffer : register(b1)
    {
        float4 a;
        int2 b;
    };
```

---

## HLSL example

```cpp
Texture2D foo[5] : register(t2);
Buffer bar : register(t7);
RWBuffer dataLog : register(u1);

Sampler samp : register(s0);

struct Data
{
    UINT index;
    float4 color;
};
ConstantBuffer<Data> myData : register(b0);

Texture2D terrain[] : register(t8); // Unbounded array
Texture2D misc[] : register(t0,space1); // Another unbounded array 
                                        // space1 avoids overlap with above t#

struct MoreData
{
    float4x4 xform;
};
ConstantBuffer<MoreData> myMoreData : register(b1);

struct Stuff
{
    float2 factor;
    UINT drawID;
};
ConstantBuffer<Stuff> myStuff[][3][8]  : register(b2, space3)
```
