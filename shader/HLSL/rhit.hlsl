// 启用 Vulkan 光线追踪扩展
#pragma enable_dxr

// 包含头文件
#include "rscatter.hlsl"
#include "vertex.hlsl"

//// 资源绑定
//[[vk::binding(4, 0)]] 
//StructuredBuffer<Vertex> VertexArray; // 顶点数据
[[vk::binding(5, 0)]] 
StructuredBuffer<uint> IndexArray; // 索引数据
[[vk::binding(6, 0)]] 
StructuredBuffer<Material> Materials; // 材质数据
[[vk::binding(7, 0)]] 
StructuredBuffer<uint2> Offsets; // 偏移数据
//[[vk::binding(8, 0)]] 
//Texture2D<float4> TextureSamplers[]; // 纹理采样器

// 命中属性
struct HitAttributes
{
    float2 Barycentrics;
};

// 混合函数
float2 Mix(float2 a, float2 b, float2 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float3 Mix(float3 a, float3 b, float3 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

// 最近命中着色器
[shader("closesthit")]
void main(inout RayPayload payload : SV_RayPayload, in HitAttributes attribs : SV_HitAttributes)
{
    // 获取材质
    uint2 offsets = Offsets[InstanceIndex()];
    uint indexOffset = offsets.x;
    uint vertexOffset = offsets.y;

    uint primitiveIndex = PrimitiveIndex();
    int i0 = IndexArray[indexOffset + primitiveIndex * 3 + 0]; // 直接访问索引数据
    int i1 = IndexArray[indexOffset + primitiveIndex * 3 + 1];
    int i2 = IndexArray[indexOffset + primitiveIndex * 3 + 2];

    Vertex v0 = UnpackVertex(vertexOffset + i0); // 直接访问顶点数据
    Vertex v1 = UnpackVertex(vertexOffset + i1);
    Vertex v2 = UnpackVertex(vertexOffset + i2);

    Material material = Materials[v0.MaterialIndex];

    // 计算命中点属性
    float3 barycentrics = float3(1.0 - attribs.Barycentrics.x - attribs.Barycentrics.y, attribs.Barycentrics.x, attribs.Barycentrics.y);
    float3 normal = v0.Normal;
    float2 texCoord = Mix(v0.TexCoord, v1.TexCoord, v2.TexCoord, barycentrics);

    // 散射计算
    payload = Scatter(material, WorldRayDirection(), normal, texCoord, RayTCurrent(), payload.RandomSeed);
}