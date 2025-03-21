#include "raypayload.hlsl"
#include "uniformBufferObject.hlsl"

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};



[shader("miss")]
void main(inout RayPayload payload : SV_RayPayload)
{
    if (uniformBufferObject.hasSkyBox)
    {
        // ¼ÆËãÌì¿ÕÑÕÉ«
        float t = 0.5 * (normalize(WorldRayDirection()).y + 1.0);
        float3 skyColor = lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), t);

        payload.ColorAndDistance = float4(skyColor, -1.0);
    }
    else
    {
        payload.ColorAndDistance = float4(0.0, 0.0, 0.0, -1.0);
    }
}