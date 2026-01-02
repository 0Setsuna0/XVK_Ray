#include "Sampling.hlsl"

//todo: add more kinds of material models
Texture2D TextureSamplers[] : register(t8);
SamplerState LinearSampler : register(s8);

float3 SampleLambertian(BSDFMaterial mat, float3 wo, inout float3 wi, inout float pdf, inout float cos_theta,
    float3 n, float2 uv, inout uint seed)
{
    float2 xi = float2(RandomFloat(seed), RandomFloat(seed));
    wi = SampleHemisphere(xi);
    
    cos_theta = wi.z;
    pdf = cos_theta * INV_PI;
    
    if (cos_theta <= 0)
    {
        pdf = 0;
        return float3(0, 0, 0);
    }

    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 albedo = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return albedo * INV_PI; // Lambertian reflectance is 1/PI
}

float3 SampleDiffuseLight(BSDFMaterial mat, float2 uv)
{
    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 color = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return color * 4;
}

float3 SampleBSDF(float3 n, float2 uv, const BSDFMaterial mat, float3 wo_world, const bool forward_facing,
    inout float3 wi_world, inout float pdf_w, inout float cos_theta, inout uint seed)
{
    float3 f = 0;
    pdf_w = 0;
    cos_theta = 0;
    wi_world = 0;
    
    float3 T, B;
    BranchlessONB(n, T, B); //world to local
    float3 wo_local = to_local(wo_world, T, B, n);
    float3 wi_local = float3(0, 0, 0);
    switch (mat.materialModel)
    {
        case BSDFLambertian:
            f = SampleLambertian(mat, wo_local, wi_local, pdf_w, cos_theta, n, uv, seed);
            break;
        default:
            break;
    }
    wi_world = to_world(wi_local, T, B, n);
    
    return f;
}

float3 EvaluateLambertian(BSDFMaterial mat, float3 wo_local, float3 wi_local, inout float cos_theta, inout float pdf, float3 n, float2 uv)
{
    cos_theta = wi_local.z;
    pdf = cos_theta * INV_PI;
    
    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 albedo = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return albedo * INV_PI; // Lambertian reflectance is 1/PI
}
float3 EvaluateDiffuseLight(BSDFMaterial mat, float2 uv)
{
    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 color = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return color * 4;
}
//wi and wo is known, get bsdf factor and pdf
float3 EvaluateBSDF(float3 n, float2 uv, BSDFMaterial mat, inout float cos_theta, float3 wo, float3 wi, inout float pdf)
{
    float3 f = float3(0, 0, 0);
    pdf = 0;
    
    float3 T, B;
    BranchlessONB(n, T, B);
    
    //world to local
    float3 wo_local = to_local(wo, T, B, n);
    float3 wi_local = to_local(wi, T, B, n);
    
    switch (mat.materialModel)
    {
        case BSDFLambertian:
            f = EvaluateLambertian(mat, wo_local, wi_local, cos_theta, pdf, n, uv);
            break;
        case BSDFDiffuseLight:
            f = EvaluateDiffuseLight(mat, uv);
            break;
        default:
            break;
    }
    
    return f;
}