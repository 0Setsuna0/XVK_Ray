#include "RayPayload.hlsl"
#include "UBO.hlsl"

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};


[shader("miss")]
void main(inout RayPayload payload : SV_RayPayload)
{
    if (uniformBufferObject.hasSkyBox)
    {
        float t = 0.5 * (normalize(WorldRayDirection()).y + 1.0);
        float3 skyColor = lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), t);

        payload.SkyColor = float4(skyColor, -1.0);
    }
    else
    {
        payload.SkyColor = float4(0.0, 0.0, 0.0, -1.0);
    }
    payload.MaterialIndex = -1;
}