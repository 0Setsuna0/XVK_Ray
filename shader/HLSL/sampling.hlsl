#include "material.hlsl"
#include "random.hlsl"
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define INV_PI (1. / PI)
#define sqrt2 1.41421356237309504880

Texture2D TextureSamplers[] : register(t8);
SamplerState LinearSampler : register(s8);

void BranchlessONB(float3 n, out float3 b1, out float3 b2)
{
    float sign = n.z >= 0.0 ? 1.0 : -1.0;
    float a = -1.0 / (sign + n.z);
    float b = n.x * n.y * a;

    b1 = float3(1.0 + sign * n.x * n.x * a, sign * b, -sign * n.x);
    b2 = float3(b, sign + n.y * n.y * a, -n.y);
}

float3 to_world(float3 v, float3 T, float3 B, float3 N)
{
    return v.x * T + v.y * B + v.z * N;
}

float3 to_local(float3 v, float3 T, float3 B, float3 N)
{
    return float3(dot(v, T), dot(v, B), dot(v, N));
}

//cos weighted hemisphere sampling
float3 SampleHemisphere(float2 xi)
{
    float phi = TWO_PI * xi.x; // Azimuth angle [0, 2¦Ð)
    float cos_theta = sqrt(1.0 - xi.y); // sqrt for cosine-weighted sampling
    float sin_theta = sqrt(xi.y); // sin(theta) = sqrt(1 - cos^2(theta))
    
    // Convert spherical coordinates to Cartesian (local space, z-up)
    return float3(
        sin_theta * cos(phi),
        sin_theta * sin(phi),
        cos_theta
    );
}

float3 SampleLambertian(Material mat, float3 wo, out float3 wi, inout float pdf, out float cos_theta,
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

float3 SampleDiffuseLight(Material mat, float2 uv)
{
    float4 texColor = (mat.baseColorTextureIndex >= 0) ? TextureSamplers[mat.baseColorTextureIndex].SampleLevel(LinearSampler, uv, 0) : float4(1, 1, 1, 1);
    float3 color = (mat.baseColorTextureIndex >= 0) ? texColor.rgb : mat.baseColor.rgb;
    return color * 4;
}

float3 SampleBSDF(float3 n, float2 uv, const Material mat, float3 wo_world, const bool forward_facing,
    out float3 wi_world, inout float pdf_w, inout float cos_theta, inout uint seed)
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
        case MaterialLambertian:
            f = SampleLambertian(mat, wo_local, wi_local, pdf_w, cos_theta, n, uv, seed);
            break;
        default:
            break;
    }
    wi_world = to_world(wi_local, T, B, n);
    
    return f;
}