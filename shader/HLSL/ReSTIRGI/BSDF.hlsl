#include "Sampling.hlsl"

//todo: add more kinds of material models
Texture2D TextureSamplers[] : register(t8);
SamplerState LinearSampler : register(s8);

float3 SampleLambertian(BSDFMaterial mat, float3 wo, out float3 wi, out float pdf, out float cos_theta,
    float3 n, float2 uv, uint2 seed)
{
    float2 xi = float2(RandomFloat(seed.x), RandomFloat(seed.x));
    wi = SampleHemisphere(xi);
    cos_theta = wi.z;
    pdf = cos_theta * INV_PI;
    if (min(wi.z, wo.z) <= 0.0)
    {
        return float3(0, 0, 0);
    }
    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 color = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return color * INV_PI; // Lambertian reflectance is 1/PI
}

float3 SampleDiffuseLight(BSDFMaterial mat, float2 uv)
{
    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 color = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return color;
}

float3 SampleBSDF(float3 n, float2 uv, const BSDFMaterial mat, float3 wo, const bool forward_facing,
    out float3 wi, out float pdf_w, out float cos_theta, inout uint2 seed)
{
    float3 f = 0;
    pdf_w = 0;
    cos_theta = 0;
    wi = 0;
    
    float3 T, B;
    BranchlessONB(n, T, B);//world to local
    wo = to_local(wo, T, B, n);
    switch (mat.materialModel)
    {
        case BSDFLambertian:
            f = SampleLambertian(mat, wo, wi, pdf_w, cos_theta, n, uv , seed);
            break;
        default:
            break;
    }
    wi = to_world(wi, T, B, n);
    
    return f;
}