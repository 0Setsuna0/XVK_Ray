#include "raypayload.hlsl"
#include "uniformBufferObject.hlsl"
#include "sampling.hlsl"
//resource bindings

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> AccumulationImage : register(u1);
RWTexture2D<float4> OutputImage : register(u2);
cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};
StructuredBuffer<Material> Materials : register(t6);
uint2 GetLaunchID()
{
    return DispatchRaysIndex().xy;
}
uint2 GetLaunchSize()
{
    return DispatchRaysDimensions().xy;
}
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}
[shader("raygeneration")]
void main()
{
    const uint2 launchID = GetLaunchID();
    const uint2 launchSize = GetLaunchSize();
    
    uint rngState = InitRandomSeed(launchID, launchSize, uniformBufferObject.currentFrame);

    float3 pixelColor = 0;
    
    // multiple sampling process
    for (uint i = 0; i < uniformBufferObject.spp; i++)
    {
        float2 pixelOffset = float2(
            RandomFloat(rngState),
            RandomFloat(rngState));
            
        float2 pixel = (float2) launchID + pixelOffset;
        float2 uv = (pixel / (float2) launchSize) * 2.0 - 1.0;

        float4 origin = mul(uniformBufferObject.modelViewInverse, float4(0, 0, 0, 1));
        float4 target = mul(uniformBufferObject.projectionInverse, float4(uv.x, uv.y, 1, 1));
        
        float3 direction = normalize(mul(uniformBufferObject.modelViewInverse, float4(normalize(target.xyz), 0)).xyz);

        float3 rayColor = 0;
        float3 throughput = 1.0;

        for (uint bounce = 0;; bounce++)
        {
            RayDesc ray;
            ray.Origin = origin.xyz;
            ray.Direction = direction;
            ray.TMin = 0.0001f;
            ray.TMax = 10000.0f;

            RayPayload rayPayload;
            rayPayload.MaterialIndex = -1;
            
            rayPayload.Position = float3(0, 0, 0);
            rayPayload.Normal = float3(0, 0, 0);
            rayPayload.UV = float2(0, 0);
            rayPayload.HitT = 0.0f;

            TraceRay(
                scene,
                RAY_FLAG_NONE,
                0xFF,
                0,
                0,
                0,
                ray,
                rayPayload);

            if (rayPayload.MaterialIndex == -1)
            {
                float t = 0.5 * (direction.y + 1.0);
                float3 skyColor = lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), t);
                
                rayColor += uniformBufferObject.hasSkyBox ? throughput * skyColor : 0;
                break;
            }
            
            Material mat = Materials[rayPayload.MaterialIndex];

            if (mat.materialModel == MaterialDiffuseLight)
            {
                rayColor += throughput * mat.baseColor.rgb * 4.0;
                break;
            }

            float pdf = 0;
            float cos_theta = 0;
            float3 wi = 0;
            float3 wo = -direction;
            
            // BSDF ²ÉÑù
            float3 f = SampleBSDF(rayPayload.Normal, rayPayload.UV, mat, wo,
                true, wi, pdf, cos_theta, rngState);

            if (pdf <= 1e-6)
                break;

            // Update Throughput
            throughput *= f * cos_theta / pdf;

            // Russian Roulette
            if (bounce > 3)
            {
                float p = max(throughput.r, max(throughput.g, throughput.b));
                if (RandomFloat(rngState) > p)
                    break;
                throughput *= 1.0 / p;
            }


            float3 hitPosition = ray.Origin + ray.Direction * rayPayload.HitT;

            float3 offsetDir = dot(wi, rayPayload.Normal) > 0 ? rayPayload.Normal : -rayPayload.Normal;
            origin.xyz = hitPosition + offsetDir * 0.001f; 

            direction = wi;
        }

        pixelColor += rayColor;
    }


    bool accumulate = uniformBufferObject.spp != uniformBufferObject.totalNumberOfSamples;
    float3 accumulatedColor = accumulate ?
        AccumulationImage[launchID].rgb :
        float3(0, 0, 0);
    accumulatedColor += pixelColor;

    float3 finalColor = accumulatedColor / uniformBufferObject.totalNumberOfSamples;
    finalColor = ACESFilm(finalColor);
    finalColor = pow(finalColor, 1.0 / 2.2);

    AccumulationImage[launchID] = float4(accumulatedColor, 0);
    OutputImage[launchID] = float4(finalColor, 0);
}