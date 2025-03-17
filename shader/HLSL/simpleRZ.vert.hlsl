#include "uniformBufferObject.hlsl"
#include "material.hlsl"

struct VS_INPUT
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 normal : NORMAL;
    [[vk::location(2)]] float2 texCoord : TEXCOORD;
    [[vk::location(3)]] int materialIndex : MATERIALINDEX;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    [[vk::location(0)]] float3 FragColor : COLOR0;
    [[vk::location(1)]] float3 FragNormal : NORMAL;
    [[vk::location(2)]] float2 FragTexCoord : TEXCOORD0;
    [[vk::location(3)]] int FragMaterialIndex : MATERIALINDEX;
};

cbuffer UniformBuffer : register(b0)
{
	UniformBufferObject uniformBufferObject;
};

StructuredBuffer<Material> MaterialArray : register(t1);

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    Material m = MaterialArray[input.materialIndex];

    // 计算顶点位置
    output.Position = mul(uniformBufferObject.projection, mul(uniformBufferObject.modelView, float4(input.position, 1.0f)));

    // 传递颜色
    output.FragColor = m.baseColor.xyz;

    // 计算法线（注意：这里需要 ModelInverseTranspose 矩阵来正确变换法线）
    output.FragNormal = mul(uniformBufferObject.modelViewInverse, float4(input.normal, 0.0f)).xyz;

    // 传递纹理坐标
    output.FragTexCoord = input.texCoord;

    // 传递材质索引
    output.FragMaterialIndex = input.materialIndex;

    return output;
}