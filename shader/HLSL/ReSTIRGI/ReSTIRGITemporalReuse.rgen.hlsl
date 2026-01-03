#include "BSDF.hlsl"
#include "UBO.hlsl"
#define M_MAX 30
RaytracingAccelerationStructure scene : register(t0);
RWStructuredBuffer<ReSTIRGISample> samples : register(u9);
RWStructuredBuffer<ReSTIRGIReservoir> temporal_reservoirs : register(u10);
RWStructuredBuffer<ReSTIRGIReservoir> spatial_reservoirs : register(u11);
RWStructuredBuffer<ReSTIRGISample> oldsamples : register(u18);
StructuredBuffer<BSDFMaterial> Materials : register(t6);

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};

RWTexture2D<float4> AccumulationImage : register(u1);
RWTexture2D<float4> OutputImage : register(u2);

RWTexture2D<float2> GMotionVector : register(u15);

RWTexture2D<float4> GPosPrev : register(u16);
RWTexture2D<float4> GNrmPrev : register(u17);

uint2 GetLaunchID()
{
    return DispatchRaysIndex().xy;
    
}
uint2 GetLaunchSize()
{
    return DispatchRaysDimensions().xy;
}
float EvaluateTargetFunction(ReSTIRGISample s, float3 x_curr, float3 n_curr)
{
    float3 wi = normalize(s.x_sample - x_curr);
    float cos_theta = dot(n_curr, wi);
    if (cos_theta <= 0.0)
        return 0.0;
    return length(s.L_out);
}
[shader("raygeneration")]
void main()
{
    const uint2 launchID = GetLaunchID();
    const uint2 launchSize = GetLaunchSize();
    const uint pixelIndex = launchID.y * launchSize.x + launchID.x;
    
    uint rngState = InitRandomSeed(launchID, launchSize, uniformBufferObject.currentFrame);
    uint totalPixels = launchSize.x * launchSize.y;
    uint pingpong = uniformBufferObject.currentFrame % 2;
    uint currentOffset = pingpong * totalPixels; //current frame
    uint prevOffset = (1 - pingpong) * totalPixels; //prev frame
    
    ReSTIRGISample s_new = samples[pixelIndex];
    ReSTIRGIReservoir r_new;
    ReSTIRGIReservoir r_prev;
    
    if (length(s_new.n_sample) == 0)
    {
        InitReservoir(r_new);
        temporal_reservoirs[currentOffset + pixelIndex] = r_new;
        spatial_reservoirs[currentOffset + pixelIndex] = r_new;
        return;
    }
    bool foundTemporal = false;
    
    
    float2 motion = GMotionVector[launchID].xy;
        
    float2 uv = (float2(launchID) + 0.5) / float2(launchSize);
    float2 prevUV = uv - motion;
    int2 prevCoord = int2(prevUV * float2(launchSize));
    
    if (prevCoord.x >= 0 && prevCoord.x < launchSize.x &&
        prevCoord.y >= 0 && prevCoord.y < launchSize.y)
    {
        float4 prevPosData = GPosPrev[prevCoord];
        float3 prevNorm = GNrmPrev[prevCoord].xyz;
        uint prevPixelIndex = prevCoord.y * launchSize.x + prevCoord.x;
        ReSTIRGISample s_old = oldsamples[prevPixelIndex];
            
        if (prevPosData.w != 0)
        {
            r_prev = temporal_reservoirs[prevOffset + prevPixelIndex];
            foundTemporal = true;
        }
        else
        {
            InitReservoir(r_prev);
            spatial_reservoirs[currentOffset + pixelIndex] = r_prev;
        }
    }
    
    float3 x_curr = s_new.x_view;
    float3 n_curr = normalize(s_new.n_view);

    ReSTIRGIReservoir r_final;
    InitReservoir(r_final);
    // add new sample
    float target_p_new = EvaluateTargetFunction(s_new, x_curr, n_curr);
    float w_new = (s_new.p_q > 1e-6) ? (target_p_new / s_new.p_q) : 0.0;
    UpdateReservoir(r_final, s_new, w_new, rngState);
    // add temporial sample
    if (foundTemporal)
    {
        float target_p_prev = EvaluateTargetFunction(r_prev.s, x_curr, n_curr);
        MergeReservoir(r_final, r_prev, target_p_prev, rngState);
    }
    else
    {
        r_final.m = 1;
    }
    
    if (r_final.m > M_MAX)
    {
        r_final.w_sum *= (float) M_MAX / (float) r_final.m;
        r_final.m = M_MAX;
    }
        
    float target_p_final = EvaluateTargetFunction(r_final.s, x_curr, n_curr);
    
    if (target_p_final > 1e-6 && r_final.m > 0)
    {
        r_final.W = r_final.w_sum / (float(r_final.m) * target_p_final);
    }
    else
    {
        r_final.W = 0.0;
    }

    temporal_reservoirs[currentOffset + pixelIndex] = r_final;
    
    float3 finalColor = float3(0, 0, 0);
    if (r_final.W > 0)
    {
        float3 x_curr = s_new.x_view;
        float3 n_curr = s_new.n_view;
        float3 x_sample_ = r_final.s.x_sample;

        float3 wi = normalize(x_sample_ - x_curr);

        float pdf_val = 0.0;
        float cos_theta = 0.0;
        cos_theta = max(0.0, dot(n_curr, wi));
    
        finalColor = r_final.s.L_out * r_final.W * r_final.s.f * cos_theta;
    }
    finalColor = ACESFilm(finalColor);
    finalColor = pow(finalColor, 1.0 / 2.2);
    OutputImage[launchID] = float4(finalColor, 1.0);
}
