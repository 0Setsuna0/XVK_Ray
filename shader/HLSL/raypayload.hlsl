struct [raypayload] RayPayload
{
    float4 ColorAndDistance; // 允许TraceRay返回该值
    float4 ScatterDirection; // 允许TraceRay返回该值
    uint RandomSeed; // 允许TraceRay返回该值
    float3 Position;
    float3 Normal;
    float2 UV;
    uint MaterialIndex;
    float3 SkyColor;
};