#include "raypayload.hlsl"
#include "uniformBufferObject.hlsl"

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};

[shader("miss")]
void main(inout RayPayload payload : SV_RayPayload)
{
    payload.MaterialIndex = -1; // 标记为未命中
    payload.HitT = -1.0;
}