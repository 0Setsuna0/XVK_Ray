struct [raypayload] RayPayload
{
    uint RandomSeed; 
    float3 Position;
    float3 Normal;
	float2 UV;
	uint MaterialIndex;
	uint TriangleIndex;
	uint InstanceIndex;
	float Area;
	float Distance;
	uint HitKind;
    float4 SkyColor;
};

struct [raypayload] AnyHitPayload
{
	int hit;
};	