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
    
    // 获取帧随机种子
    uint frameSeed = uniformBufferObject.currentFrame;
    
    // 初始化局部随机种子 (混合 像素坐标 + 帧索引)
    uint2 seed = uint2(
        InitRandomSeed(launchID.x, launchID.y) + frameSeed,
        InitRandomSeed(launchID.y, launchID.x)
    );

    float3 pixelColor = 0;
    
    // multiple sampling process
    for (uint i = 0; i <= uniformBufferObject.spp; i++)
    {
        // 1. 修正抗锯齿逻辑：使用局部 seed.x，而不是全局 pixelRandomSeed
        float2 pixelOffset = float2(
            RandomFloat(seed.x),
            RandomFloat(seed.y));
            
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
            ray.Direction = direction; // 现在类型匹配了 (float3)
            ray.TMin = 0.0001f;
            ray.TMax = 10000.0f;

            RayPayload rayPayload;
            rayPayload.MaterialIndex = -1;
            
            // 3. 显式初始化 Payload 所有成员 (防止 Validation Error)
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
            
            // 命中判断
            if (rayPayload.MaterialIndex == -1)
            {
                float t = 0.5 * (direction.y + 1.0);
                float3 skyColor = lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), t);
                
                rayColor += uniformBufferObject.hasSkyBox ? throughput * skyColor : 0;
                break;
            }
            
            Material mat = Materials[rayPayload.MaterialIndex];
            
            // 简单自发光处理
            if (mat.materialModel == MaterialDiffuseLight)
            {
                 // 假设强度为 1.0，根据需要调整
                rayColor += throughput * mat.baseColor.rgb * 4.0;
                break;
            }

            float pdf = 0;
            float cos_theta = 0;
            float3 wi = 0;
            float3 wo = -direction;
            
            // BSDF 采样
            float3 f = SampleBSDF(rayPayload.Normal, rayPayload.UV, mat, wo,
                true, wi, pdf, cos_theta, seed);

            if (pdf <= 1e-6)
                break;

            // Update Throughput
            throughput *= f * cos_theta / pdf;

            // Russian Roulette
            if (bounce > 3)
            {
                float p = max(throughput.r, max(throughput.g, throughput.b));
                if (RandomFloat(seed.x) > p)
                    break;
                throughput *= 1.0 / p;
            }

            // 更新下一跳
            float3 hitPosition = ray.Origin + ray.Direction * rayPayload.HitT;

            // 更新下一跳 Origin
            float3 offsetDir = dot(wi, rayPayload.Normal) > 0 ? rayPayload.Normal : -rayPayload.Normal;
            origin.xyz = hitPosition + offsetDir * 0.001f; // 这里的偏移量依然重要

            direction = wi;
        }

        pixelColor += rayColor;
    }

    // 累积逻辑
    bool accumulate = uniformBufferObject.spp != uniformBufferObject.totalNumberOfSamples;
    float3 accumulatedColor = accumulate ?
        AccumulationImage[launchID].rgb :
        float3(0, 0, 0);
    accumulatedColor += pixelColor;

    float3 finalColor = accumulatedColor / uniformBufferObject.totalNumberOfSamples;
    finalColor = sqrt(finalColor);
    finalColor = ACESFilm(finalColor);

    AccumulationImage[launchID] = float4(accumulatedColor, 0);
    OutputImage[launchID] = float4(finalColor, 0);
}