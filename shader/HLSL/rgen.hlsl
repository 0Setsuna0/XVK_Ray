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

[shader("raygeneration")]
void main()
{
    const uint2 launchID = GetLaunchID();
    const uint2 launchSize = GetLaunchSize();
    
    uint pixelRandomSeed = uniformBufferObject.randomSeed;
    RayPayload rayPayload;
    //rayPayload.ColorAndDistance = float4(0, 0, 0, 0); // 初始化
    //rayPayload.ScatterDirection = float4(0, 0, 0, 0);
    rayPayload.RandomSeed = InitRandomSeed(
        InitRandomSeed(launchID.x, launchID.y),
        uniformBufferObject.reuse ? uniformBufferObject.currentFrame : uniformBufferObject.totalNumberOfSamples
    );

    float3 pixelColor = 0;
    
    //multiple sampling process
    for (uint i = 0; i < uniformBufferObject.spp; i++)
    {
        //random offset
        float2 pixelOffset = float2(
            RandomFloat(pixelRandomSeed),
            RandomFloat(pixelRandomSeed));
        float2 pixel = launchID + pixelOffset;
        float2 uv = (pixel / (float2)launchSize) * 2.0 - 1.0; //texture uv range[-1, 1]
       
        float2 offset = RandomInUnitDisk(rayPayload.RandomSeed);
        float4 origin = mul(uniformBufferObject.modelViewInverse, float4(0, 0, 0, 1)); 
        float4 target = mul(uniformBufferObject.projectionInverse, float4(uv.x, uv.y, 1, 1));
        float4 direction = mul(uniformBufferObject.modelViewInverse, float4(normalize(target.xyz), 0));

        float3 rayColor = 1;

        for (uint bounce = 0; bounce <= uniformBufferObject.numberOfBounces; bounce++)
        {
            if (bounce == uniformBufferObject.numberOfBounces)
            {
                rayColor = 0;
                break;
            }
            
            RayDesc ray;
            ray.Origin = origin.xyz;
            ray.Direction = direction.xyz;
            ray.TMin = 0.01f;
            ray.TMax = 1000.0f;

            // 调用TraceRay
            TraceRay(
                scene, // 加速结构
                RAY_FLAG_NONE, // 光线标志 (替换gl_RayFlagsOpaqueEXT)
                0xFF, // Instance掩码 (保持0xff)
                0, // SBT记录偏移
                0, // SBT记录步长
                0, // 未命中着色器索引
                ray, // 光线描述结构体
                rayPayload); // 直接传递载荷对象
            
            float3 hitColor = rayPayload.ColorAndDistance.rgb;
            float t = rayPayload.ColorAndDistance.w;
            bool isScattered = rayPayload.ScatterDirection.w > 0;

            rayColor *= hitColor;

            if (t < 0 || !isScattered)
                break;
            
            //if (rayPayload.MaterialIndex == -1)
            //{
            //    tempPixelColor += throughput * rayPayload.SkyColor;
            //    break;
            //}
            
            //Material mat = Materials[rayPayload.MaterialIndex];
            
            //float pdf = 0;
            //float cos_theta = 0;
            //float3 wi = 0;
            //float3 wo = -direction.xyz;
            //uint2 seedin = uint2(rayPayload.RandomSeed, rayPayload.RandomSeed);
            
            //float3 f = SampleBSDF(rayPayload.Normal, rayPayload.UV, mat, wo,
            //    true, wi, pdf, cos_theta, seedin);
            
            ////update throughput
            //if (mat.materialModel == MaterialDiffuseLight && bounce != 2)
            //{
            //    tempPixelColor += throughput * SampleDiffuseLight(mat, uv);
            //    break;
            //}
            //if (mat.materialModel != MaterialDiffuseLight)
            //{
            //    throughput *= f * cos_theta / pdf;
            //}
            //direction.xyz = wi;
            origin.xyz += t * direction.xyz;
            direction.xyz = rayPayload.ScatterDirection.xyz;

        }

        pixelColor += rayColor;
    }

    bool accumulate = uniformBufferObject.spp != uniformBufferObject.totalNumberOfSamples;
    float3 accumulatedColor = accumulate ?
        AccumulationImage[launchID].rgb :
        float3(0, 0, 0);
    accumulatedColor += pixelColor;

    float3 finalColor = accumulatedColor / uniformBufferObject.totalNumberOfSamples;
    finalColor = sqrt(finalColor);
    
    AccumulationImage[launchID] = float4(accumulatedColor, 0);
    OutputImage[launchID] = float4(finalColor, 0);
}