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

    //float4 viewPos = mul(uniformBufferObject.viewPrev, float4(s_new.x_view, 1.0));
    //float4 clipPos = mul(uniformBufferObject.projectionPrev, viewPos);
    //float3 ndc = clipPos.xyz / clipPos.w;
    //prevUV = ndc.xy * 0.5 + 0.5;
    //prevCoord = int2(prevUV * float2(launchSize));
    
    if (prevCoord.x >= 0 && prevCoord.x < launchSize.x &&
        prevCoord.y >= 0 && prevCoord.y < launchSize.y)
    {
        float4 prevPosData = GPosPrev[prevCoord];
        float3 prevNorm = GNrmPrev[prevCoord].xyz;
        uint prevPixelIndex = prevCoord.y * launchSize.x + prevCoord.x;
        ReSTIRGISample s_old = oldsamples[prevPixelIndex];
            
        if (prevPosData.w != 0 /*&& IsSimilar(s_new.x_view, s_new.n_view, prevPosData.xyz, prevNorm)*/)
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
    
    InitReservoir(r_new);
    float phat_new = p_hat(s_new.L_out);
    float w_new = phat_new / s_new.p_q;
    UpdateReservoir(r_new, s_new, w_new, rngState);
    r_new.W = (phat_new > 0) ? r_new.w_sum / (r_new.m * phat_new) : 0;

    ReSTIRGIReservoir r_final;
    r_final.w_sum = 0;
    r_final.W = 0;
    r_final.m = 0;
    // Combine new
    UpdateReservoir(r_final, r_new.s, r_new.m * r_new.W * phat_new, rngState);
    // Combine previous
    UpdateReservoir(r_final, r_prev.s, r_prev.m * r_prev.W * p_hat(r_prev.s.L_out), rngState);
    uint mval = r_prev.m;
    float new_phat = p_hat(r_final.s.L_out);
    if (new_phat > 0)
    {
        mval++;
    }
    r_final.m = min(r_prev.m + 1, M_MAX);
    r_final.W = new_phat * mval == 0 ? 0 : r_final.w_sum / (mval * new_phat);
    //r_final = r_new;
    //phat_new = p_hat(s_new.L_out);
    //w_new = phat_new / s_new.p_q;
    //UpdateReservoir(r_prev, s_new, w_new, rngState);
    //if (r_prev.m > M_MAX)
    //{
    //    r_prev.w_sum *= (float) M_MAX / (float) r_prev.m;
    //    r_prev.m = M_MAX;
    //}
    //r_prev.W = (p_hat(r_prev.s.L_out) * r_prev.m == 0) ? 0 : r_prev.w_sum / (r_prev.m * p_hat(r_prev.s.L_out));
    //r_final = r_prev;
    
    temporal_reservoirs[currentOffset + pixelIndex] = r_final;
    
    float3 finalColor = float3(0, 0, 0);
    if (r_final.W > 0)
    {
        float3 x_view_ = r_final.s.x_view;
        float3 n_view_ = r_final.s.n_view;
        float3 x_sample_ = r_final.s.x_sample;
        float3 wi = normalize(x_sample_ - x_view_);

        float pdf_val = 0.0;
        float cos_theta = 0.0;
       
        cos_theta = max(0.0, dot(n_view_, wi));
        finalColor = r_final.s.L_out * r_final.W * r_final.s.f * cos_theta;
    }
    finalColor = ACESFilm(finalColor);
    finalColor = pow(finalColor, 1.0 / 2.2);
    OutputImage[launchID] = float4(finalColor, 1.0);
}
