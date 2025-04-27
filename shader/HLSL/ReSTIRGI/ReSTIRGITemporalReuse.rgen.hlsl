#include "BSDF.hlsl"
#include "UBO.hlsl"
RaytracingAccelerationStructure scene : register(t0);
RWStructuredBuffer<ReSTIRGISample> samples : register(u9);
RWStructuredBuffer<ReSTIRGISample> oldsamples : register(u10);
RWStructuredBuffer<ReSTIRGIReservoir> temporal_reservoirs : register(u10);
RWStructuredBuffer<ReSTIRGIReservoir> spatial_reservoirs : register(u11);

StructuredBuffer<BSDFMaterial> Materials : register(t6);

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};

RWTexture2D<float4> AccumulationImage : register(u1);
RWTexture2D<float4> OutputImage : register(u2);

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
    uint seed = InitRandomSeed(
        InitRandomSeed(launchID.x, launchID.y),
        uniformBufferObject.currentFrame
    );
    uint pixelIndex = launchID.y * launchSize.x + launchID.x; //row major
    ReSTIRGISample sample = samples[pixelIndex];
    
    if (sample.materialID == -1 || sample.materialID == BSDFDiffuseLight)
    {
        OutputImage[launchID] = AccumulationImage[launchID];
        return;
    }
    
    if(uniformBufferObject.currentFrame == 0)
    {
        InitReservoir(temporal_reservoirs[pixelIndex]);
        InitReservoir(spatial_reservoirs[pixelIndex]);
        oldsamples[pixelIndex] = samples[pixelIndex];
    }
    
    ReSTIRGIReservoir r = temporal_reservoirs[pixelIndex];
    float w_new = length(sample.L_out) / sample.p_q;
    if (!Similar(samples[pixelIndex], oldsamples[pixelIndex]))
    {
        oldsamples[pixelIndex] = samples[pixelIndex];
        return;
    }
    oldsamples[pixelIndex] = samples[pixelIndex];
    UpdateReservoir(r, sample, w_new, seed);
    r.W = r.w_sum / (r.m * p_hat(r.s.L_out));
    
    temporal_reservoirs[pixelIndex] = r;
    //float3 L_out = r.s.L_out;
    //float3 f = r.s.f;
    //float3 n_view = normalize(sample.n_view);
    //float3 wi = normalize(r.s.x_sample - sample.x_view);
    //float3 col = AccumulationImage[launchID].rgb;
    //if (uniformBufferObject.reuse)
    //{
    //    col += r.s.f * abs(dot(wi, n_view)) * r.s.L_out * r.W;
    //}
    //OutputImage[launchID] = float4(col, 1.0);
}
