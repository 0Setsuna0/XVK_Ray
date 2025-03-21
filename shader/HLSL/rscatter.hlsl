#include "random.hlsl"
#include "raypayload.hlsl"
#include "material.hlsl"

//todo: add more kinds of material models
Texture2D TextureSamplers[] : register(t8);
SamplerState LinearSampler : register(s8);

// Polynomial approximation by Christophe Schlick
float Schlick(float cosine, float refractionIndex)
{
	float r0 = (1 - refractionIndex) / (1 + refractionIndex);
	r0 *= r0;
	return r0 + (1 - r0) * pow(1 - cosine, 5);
}

// Lambertian
RayPayload ScatterLambertian(Material m, float3 direction, float3 normal, float2 texCoord, float t, inout uint seed)
{
	bool isScattered = dot(direction, normal) < 0;
    float4 texColor = (m.baseColorTextureIndex >= 0) ? TextureSamplers[m.baseColorTextureIndex].SampleLevel(LinearSampler, texCoord, 0) : float4(1, 1, 1, 1);
    float4 colorAndDistance = float4(m.baseColor.rgb * texColor.rgb, t);
	float4 scatter = float4(normal + RandomInUnitSphere(seed), isScattered ? 1 : 0);

	RayPayload rayPayload;
	rayPayload.ColorAndDistance = colorAndDistance;
	rayPayload.ScatterDirection = scatter;
	rayPayload.RandomSeed = seed;
	return rayPayload;
}

// Metallic
RayPayload ScatterMetallic(Material m, float3 direction, float3 normal, float2 texCoord, float t, inout uint seed)
{
	float3 reflected = reflect(direction, normal);
	bool isScattered = dot(reflected, normal) > 0;
	
    float4 texColor = (m.baseColorTextureIndex >= 0) ? TextureSamplers[m.baseColorTextureIndex].SampleLevel(LinearSampler, texCoord, 0) : float4(1, 1, 1, 1);
	float4 colorAndDistance = float4(m.baseColor.rgb * texColor.rgb, t);
	float4 scatter = float4(reflected + m.fuzziness * RandomInUnitSphere(seed), isScattered ? 1 : 0);
	
	RayPayload rayPayload;
	rayPayload.ColorAndDistance = colorAndDistance;
	rayPayload.ScatterDirection = scatter;
	rayPayload.RandomSeed = seed;
	return rayPayload;
}

// Dielectric
RayPayload ScatterDieletric(Material m, float3 direction, float3 normal, float2 texCoord, float t, inout uint seed)
{
	float dotProduct = dot(direction, normal);
	float3 outwardNormal = dotProduct > 0 ? -normal : normal;
	float niOverNt = dotProduct > 0 ? m.refractionIndex : 1.0 / m.refractionIndex;
	float cosine = dotProduct > 0 ? m.refractionIndex * dotProduct : -dotProduct;

	float3 refracted = refract(direction, outwardNormal, niOverNt);
	float reflectProb = any(refracted) ? Schlick(cosine, m.refractionIndex) : 1.0;

    float4 texColor = (m.baseColorTextureIndex >= 0) ? TextureSamplers[m.baseColorTextureIndex].SampleLevel(LinearSampler, texCoord, 0) : float4(1, 1, 1, 1);
    
	float4 colorAndDistance = float4(texColor.rgb, t);
	float4 scatter = RandomFloat(seed) < reflectProb ? float4(reflect(direction, normal), 1) : float4(refracted, 1);
	
	RayPayload rayPayload;
	rayPayload.ColorAndDistance = colorAndDistance;
	rayPayload.ScatterDirection = scatter;
	rayPayload.RandomSeed = seed;
	return rayPayload;
}

// Diffuse Light
RayPayload ScatterDiffuseLight(Material m, float t, inout uint seed)
{
	float4 colorAndDistance = float4(m.baseColor.rgb, t);
	float4 scatter = float4(1, 0, 0, 0);

	RayPayload rayPayload;
	rayPayload.ColorAndDistance = colorAndDistance;
	rayPayload.ScatterDirection = scatter;
	rayPayload.RandomSeed = seed;
	return rayPayload;
}

RayPayload Scatter(Material m, float3 direction, float3 normal, float2 texCoord, float t, inout uint seed)
{
	float3 normDirection = normalize(direction);

	switch (m.materialModel)
	{
		case MaterialLambertian:
			return ScatterLambertian(m, normDirection, normal, texCoord, t, seed);
		case MaterialMetallic:
			return ScatterMetallic(m, normDirection, normal, texCoord, t, seed);
		case MaterialDielectric:
			return ScatterDieletric(m, normDirection, normal, texCoord, t, seed);
		case MaterialDiffuseLight:
			return ScatterDiffuseLight(m, t, seed);
	}
	
	RayPayload rayPayload;
	rayPayload.ColorAndDistance = float4(0, 0, 0, 0);
	rayPayload.ScatterDirection = float4(0, 0, 0, 0);
	rayPayload.RandomSeed = seed;
	return rayPayload;
}