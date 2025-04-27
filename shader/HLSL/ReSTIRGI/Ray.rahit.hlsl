#include "RayPayload.hlsl"

struct HitAttributes
{
    float2 Barycentrics;
};

[shader("anyhit")]
void main(inout AnyHitPayload payload : SV_RayPayload, in HitAttributes attribs : SV_HitAttributes)
{
    AcceptHitAndEndSearch();
}