#include "BSDF.hlsl"
#include "UBO.hlsl"
#include "RayPayload.hlsl"
#define NUM_SPATIAL_SAMPLES 9
#define SPATIAL_RADIUS 10.0
#define M_MAX 300
RaytracingAccelerationStructure scene : register(t0);
RWStructuredBuffer<ReSTIRGISample> samples : register(u9);
RWStructuredBuffer<ReSTIRGIReservoir> temporal_reservoirs : register(u10);
RWStructuredBuffer<ReSTIRGIReservoir> spatial_reservoirs : register(u11);

StructuredBuffer<BSDFMaterial> Materials : register(t6);

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};

RWTexture2D<float4> AccumulationImage : register(u1);
RWTexture2D<float4> OutputImage : register(u2);
RWTexture2D<float4> GMatUV : register(u14);
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

uint GetNeighborIndex(uint2 launchID, uint2 launchSize, float max_r, inout uint seed, uint stride)
{
    float randa = RandomFloat(seed) * 2.0 * PI;
    float randr = sqrt(RandomFloat(seed)) * max_r;
    int2 rand_offset = int2(floor(cos(randa) * randr), floor(sin(randa) * randr));
    int2 coords = clamp(int2(launchID) + rand_offset, int2(0, 0), int2(launchSize) - 1);
    return coords.y * stride + coords.x;
}

bool CheckVisibility(float3 from, float3 to, float3 normal)
{
    RayDesc ray;
    float3 dir = to - from;
    float dist = length(dir);
    
    ray.Origin = from + normal * 0.01;
    ray.Direction = normalize(dir);
    ray.TMin = 0.0;
    ray.TMax = dist - 0.01;

    RayPayload shadowPayload;
    shadowPayload.HitT = -1.0;
    
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
             0xFF, 0, 0, 0, ray, shadowPayload);
             
    return shadowPayload.HitT < 0;
}

[shader("raygeneration")]
void main()
{
    uint2 launchID = GetLaunchID();
    uint2 launchSize = GetLaunchSize();
    uint pixelIndex = launchID.y * launchSize.x + launchID.x;
    
    uint rngState = InitRandomSeed(launchID, launchSize, uniformBufferObject.currentFrame);
    
    uint totalPixels = launchSize.x * launchSize.y;
    uint pingpong = uniformBufferObject.currentFrame % 2;
    uint readOffset = pingpong * totalPixels; 
    uint writeOffset = (1 - pingpong) * totalPixels; 

    ReSTIRGISample s_new = samples[pixelIndex];
    
    if (length(s_new.n_view) == 0)
    {
        OutputImage[launchID] = float4(0, 0, 0, 1);
        return;
    }
    if (length(s_new.n_sample) == 0)
    {
        OutputImage[launchID] = float4(1, 1, 1, 1);
        return;
    }

    ReSTIRGIReservoir centerRes = temporal_reservoirs[readOffset + pixelIndex];
    
    ReSTIRGIReservoir r_s;
    InitReservoir(r_s);

    float p_hat_center = p_hat(centerRes.s.L_out);
    MergeReservoir(r_s, centerRes, p_hat_center, rngState);

    for (int i = 0; i < NUM_SPATIAL_SAMPLES; i++)
    {
        uint neighborIdx = GetNeighborIndex(launchID, launchSize, SPATIAL_RADIUS, rngState, launchSize.x);
        ReSTIRGISample s_neighbor = samples[neighborIdx];
        
        if (length(s_neighbor.n_view) == 0 || !IsSimilar(s_new, s_neighbor))
        {
            continue;
        }
        
        ReSTIRGIReservoir r_neighbor = temporal_reservoirs[readOffset + neighborIdx];
        float3 x_sample = r_neighbor.s.x_sample;
        float3 n_sample = r_neighbor.s.n_sample;
        
        float3 v_orig = s_neighbor.x_view - x_sample;
        float dist2_orig = dot(v_orig, v_orig);
        
        float3 v_curr = s_new.x_view - x_sample;
        float dist2_curr = dot(v_curr, v_curr);
        
        float jacobian = 1.0;
        if (dist2_orig > 1e-6 && dist2_curr > 1e-6)
        {
            float3 dir_orig = v_orig * rsqrt(dist2_orig);
            float3 dir_curr = v_curr * rsqrt(dist2_curr);

            float cos_phi_orig = max(0.0, dot(n_sample, dir_orig));
            float cos_phi_curr = max(0.0, dot(n_sample, dir_curr));

            if (cos_phi_orig > 1e-6)
            {
                jacobian = (cos_phi_curr * dist2_orig) / (cos_phi_orig * dist2_curr);
            }
        }
        jacobian = clamp(jacobian, 0.1, 10.0);
        float p_hat_n = p_hat(r_neighbor.s.L_out) * jacobian;
        if (!CheckVisibility(r_s.s.x_view, x_sample, r_s.s.n_view))
        {
            p_hat_n = 0;
        }
        MergeReservoir(r_s, r_neighbor, p_hat_n, rngState);
        if(p_hat_n == 0)
        {
            r_s.m -= r_neighbor.m;
        }
    }

    r_s.m = min(r_s.m, M_MAX);
    float p_hat_final = p_hat(r_s.s.L_out);
    
    if (p_hat_final > 1e-6 && r_s.m > 0)
    {
        r_s.W = r_s.w_sum / (float(r_s.m) * p_hat_final);
    }
    else
    {
        r_s.W = 0.0;
    }

    float3 finalColor = float3(0, 0, 0);
    if (r_s.W > 0)
    {
        float3 x_curr = r_s.s.x_view;
        float3 n_curr = normalize(r_s.s.n_view);
        float3 x_sample = r_s.s.x_sample;
           
        float3 wi = normalize(x_sample - x_curr);
        float3 camPos = mul(uniformBufferObject.modelViewInverse, float4(0, 0, 0, 1)).xyz;
        float3 wo = normalize(camPos - x_curr);

        float cos_theta = max(0.0, dot(n_curr, wi));
                
        finalColor = r_s.s.L_out * r_s.W * r_s.s.f * cos_theta;
    }
    
    finalColor = ACESFilm(finalColor);
    finalColor = pow(finalColor, 1.0 / 2.2);
    
    spatial_reservoirs[writeOffset + pixelIndex] = r_s;
    OutputImage[launchID] = float4(finalColor, 1.0);
}
