#pragma enable_dxr
#include "RayPayload.hlsl"
#include "Vertex.hlsl"
#include "Material.hlsl"

//ByteAddressBuffer VertexArray : register(t4); already defined in Vertex.hlsl
StructuredBuffer<uint> IndexArray : register(t5);
StructuredBuffer<BSDFMaterial> Materials : register(t6);
StructuredBuffer<uint2> Offsets : register(t7);

struct HitAttributes
{
    float2 Barycentrics;
};

float2 Mix(float2 a, float2 b, float2 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float3 Mix(float3 a, float3 b, float3 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

[shader("closesthit")]
void main(inout RayPayload payload : SV_RayPayload, in HitAttributes attribs : SV_HitAttributes)
{
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
    
    float3 barycentrics = float3(1.0 - attribs.Barycentrics.x - attribs.Barycentrics.y, attribs.Barycentrics.x, attribs.Barycentrics.y);
    float3 normal = normalize(Mix(v0.Normal, v1.Normal, v2.Normal, barycentrics));
    float2 texCoord = Mix(v0.TexCoord, v1.TexCoord, v2.TexCoord, barycentrics);

    float3x4 objToWorld = ObjectToWorld3x4();
    float3 objPos = Mix(v0.Position, v1.Position, v2.Position, barycentrics);
    float3 worldPos = mul(objToWorld, float4(objPos, 1.0));
    float3 worldNrm = mul(float4(normal, 0.0), WorldToObject4x3());
    const float3 e0 = v2.Position - v0.Position;
    const float3 e1 = v1.Position - v0.Position;
    const float3 e0t = mul(ObjectToWorld3x4(), float4(e0, 0));
    const float3 e1t = mul(ObjectToWorld3x4(), float4(e1, 0));

    payload.GeometryNormal = normalize(mul(cross(e0, e1), WorldToObject3x4())).xyz;
    payload.Position = worldPos;
    payload.Normal = normalize(worldNrm);
    payload.UV = texCoord;
    payload.MaterialIndex = v0.MaterialIndex;
    payload.HitT = RayTCurrent();
}