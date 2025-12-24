// 包含头文件
#include "vertex.hlsl"
#include "random.hlsl"
#include "raypayload.hlsl"
#include "material.hlsl"
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
    int i0 = IndexArray[indexOffset + primitiveIndex * 3 + 0]; 
    int i1 = IndexArray[indexOffset + primitiveIndex * 3 + 1];
    int i2 = IndexArray[indexOffset + primitiveIndex * 3 + 2];

    Vertex v0 = UnpackVertex(vertexOffset + i0); 
    Vertex v1 = UnpackVertex(vertexOffset + i1);
    Vertex v2 = UnpackVertex(vertexOffset + i2);
    
    // 计算命中点属性
    float3 barycentrics = float3(1.0 - attribs.Barycentrics.x - attribs.Barycentrics.y, attribs.Barycentrics.x, attribs.Barycentrics.y);
    float3 normal = normalize(Mix(v0.Normal, v1.Normal, v2.Normal, barycentrics));
    float2 texCoord = Mix(v0.TexCoord, v1.TexCoord, v2.TexCoord, barycentrics);

    // 散射计算
    float3x4 objToWorld = ObjectToWorld3x4();
    // 变换位置 (Position * Matrix)
    float3 objPos = Mix(v0.Position, v1.Position, v2.Position, barycentrics);
    float3 worldPos = mul(objToWorld, float4(objPos, 1.0));
    float3 worldNrm = mul(float4(normal, 0.0), WorldToObject4x3());
    // 变换法线 (Normal * Matrix)
    // 注意：参考代码使用了 gl_WorldToObjectEXT (逆矩阵) 来变换法线，这是对付非均匀缩放的严谨做法。
    // 如果你没有非均匀缩放，直接用 objToWorld 变换向量(w=0)也行：
    payload.Position = worldPos;
    payload.Normal = normalize(worldNrm);
    payload.UV = texCoord;
    payload.MaterialIndex = v0.MaterialIndex;
    payload.HitT = RayTCurrent();
}