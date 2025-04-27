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
    float phi = TWO_PI * xi.x; // Azimuth angle [0, 2¦Ð)
    float cos_theta = 2.0 * xi.y - 1.0; // sqrt for cosine-weighted sampling
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta); // sin(theta) = sqrt(1 - cos^2(theta))
    
    // Convert spherical coordinates to Cartesian (local space, z-up)
    return float3(
        sin_theta * cos(phi),
        sin_theta * sin(phi),
        1 - cos_theta
    );
}

