#include "random.hlsl"
#include "raypayload.hlsl"
#include "uniformBufferObject.hlsl"

//resource bindings

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> AccumulationImage : register(u0);
RWTexture2D<float4> OutputImage : register(u1);
cbuffer UniformBuffer : register(b0)
{
    UniformBufferObject uniformBufferObject;
};

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
    rayPayload.RandomSeed = InitRandomSeed(
        InitRandomSeed(launchID.x, launchID.y),
        uniformBufferObject.totalNumberOfSamples
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
        float2 uv = (pixel / launchSize) * 2.0 - 1.0;//texture uv range[-1, 1]

        float2 offset = RandomInUnitDisk(rayPayload.RandomSeed);
        float4 origin = mul(uniformBufferObject.modelViewInverse, float4(offset, 0, 1)); 
        float4 target = mul(uniformBufferObject.projectionInverse, float4(uv.x, uv.y, 1, 1));
    }

}