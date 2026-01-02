#include "RayPayload.hlsl"

[shader("miss")]
void main(inout RayPayload payload : SV_RayPayload)
{
    payload.MaterialIndex = -1;
    payload.HitT = -1.0;
}