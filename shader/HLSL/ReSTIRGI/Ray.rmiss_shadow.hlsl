#include "RayPayload.hlsl"
#include "UBO.hlsl"

[shader("miss")]
void main(inout AnyHitPayload payload : SV_RayPayload)
{
    payload.hit = 0;
}