#include "Material.hlsl"
#include "ReSTIRGICommon.hlsl"

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
    float phi = TWO_PI * xi.x; // Azimuth angle [0, 2π)
    float cos_theta = sqrt(1.0 - xi.y); // sqrt for cosine-weighted sampling
    float sin_theta = sqrt(xi.y); // sin(theta) = sqrt(1 - cos^2(theta))
    
    // Convert spherical coordinates to Cartesian (local space, z-up)
    return float3(
        sin_theta * cos(phi),
        sin_theta * sin(phi),
        cos_theta
    );
}

float2 WorldToScreenUV(float3 worldPos, float4x4 view, float4x4 projection)
{
    // 1. View Space
    float4 viewPos = mul(view, float4(worldPos, 1.0));
    
    // 2. Clip Space (Homogeneous)
    float4 clipPos = mul(projection, viewPos);
    
    // 3. NDC Space [-1, 1]
    // 必须除以 w 分量，这是透视除法的核心
    float3 ndc = clipPos.xyz / clipPos.w;

    // 4. Screen UV [0, 1]
    // 将 [-1, 1] 映射到 [0, 1]
    float2 uv = ndc.xy * 0.5 + 0.5;
    
    return uv;
}
