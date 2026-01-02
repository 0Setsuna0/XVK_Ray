#ifndef RAY_PAYLOAD_H
#define RAY_PAYLOAD_H

struct [raypayload] RayPayload
{
    float3 Position : read(caller) : write(caller, closesthit);
    float3 Normal : read(caller) : write(caller, closesthit);
    float3 GeometryNormal : read(caller) : write(caller, closesthit);
    float2 UV : read(caller) : write(caller, closesthit);
    int MaterialIndex : read(caller) : write(caller, closesthit, miss);
    float HitT : read(caller) : write(caller, closesthit, miss);
};
struct [raypayload] AnyHitPayload
{
	int hit;
};	
#endif