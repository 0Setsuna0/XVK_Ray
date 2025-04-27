#include "BSDF.hlsl"
#include "UBO.hlsl"
#include "RayPayload.hlsl"
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

uint2 GetLaunchID()
{
    return DispatchRaysIndex().xy;
}
uint2 GetLaunchSize()
{
    return DispatchRaysDimensions().xy;
}


// 输入：当前线程坐标、屏幕尺寸、最大采样半径、种子
uint GetNeighborIndex(uint2 launchID, uint2 launchSize, float max_r, inout uint seed, uint size_y)
{
    // 随机极坐标角度
    float randa = RandomFloat(seed) * 2.0 * PI;
    float randr = sqrt(RandomFloat(seed)) * max_r;

    // 转换为笛卡尔偏移并转为整型
    int2 rand_offset = int2(floor(cos(randa) * randr), floor(sin(randa) * randr));

    // 当前像素加偏移，clamp 到合法范围
    int2 coords = clamp(int2(launchID) + rand_offset, int2(0, 0), int2(launchSize) - 1);

    // 将 2D 坐标转为 1D 索引（注意 size_y 是宽度）
    return coords.x * size_y + coords.y;
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
    if (length(sample.f)==0)
        return;
    ReSTIRGIReservoir r_spatial = spatial_reservoirs[pixelIndex];
    ReSTIRGIReservoir r_temporal = temporal_reservoirs[pixelIndex];
    float3 Q[9] =
    {
        float3(0, 0, 0), float3(0, 0, 0), float3(0, 0, 0),
        float3(0, 0, 0), float3(0, 0, 0), float3(0, 0, 0),
        float3(0, 0, 0), float3(0, 0, 0), float3(0, 0, 0)
    };
    uint Q_h[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint q_cnt = 0;
    
    float max_radius = 2.0f; // 可调参数
    const int neighbor_count = 8;

    for (int i = 0; i < neighbor_count; i++)
    {
        // Choose a neighbor pixel
        uint neighborIndex = GetNeighborIndex(launchID.xy, launchSize.xy, max_radius, seed, launchSize.x);
        ReSTIRGISample s_neighbor = samples[neighborIndex];
        if (!Similar(sample, s_neighbor))
        {
            continue;
        }

        ReSTIRGIReservoir rt_neighbor = temporal_reservoirs[pixelIndex];
        
        float3 x_r1 = sample.x_view;
        float3 x_q1 = s_neighbor.x_view;
        float3 x_q2 = s_neighbor.x_sample;
        float3 normal = s_neighbor.n_sample;
        float3 e_rq = x_r1 - x_q2;
        float3 e_qq = x_q1 - x_q2;
        float cos_phir = abs(dot(normalize(normal), normalize(e_rq)));
        float cos_phiq = abs(dot(normalize(normal), normalize(e_qq)));
        float div_inv = cos_phir * length(e_qq) * length(e_qq);
        float jacobian_inv = div_inv == 0 ? 0 : (cos_phiq * length(e_rq) * length(e_rq)) / div_inv;
        
        float p_qreal = p_hat(rt_neighbor.s.L_out) * jacobian_inv;

        // check visibility
        bool visibale;
        AnyHitPayload ahitPayload;
        ahitPayload.hit = 1;
        float3 wi = rt_neighbor.s.x_sample - sample.x_view;
        RayDesc ray;
        ray.Origin = sample.x_view;
        ray.Direction = wi;
        ray.TMin = 0.001f; // avoid self‑intersection
        ray.TMax = length(wi) - 0.001f; // stop just before the sample point
        TraceRay(
            scene, // acceleration structure
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // terminate on any hit
            ~0, // instance & geometry masks
            1, // ray contribution to hit group index
            0, // miss shader index
            1, // ray payload location
            ray, // the ray we built
            ahitPayload // in/out payload
        );
        visibale = ahitPayload.hit == 0;
        if(!visibale)
        {
            p_qreal = 0;
        }
        
        MergeReservoir(r_spatial, rt_neighbor, p_qreal, seed);
        Q_h[q_cnt++] = rt_neighbor.m;
    }

    //for (int j = 0; j < q_cnt; j++)
    //{
    //    if (p_hat())

    //}
    r_temporal.W = r_temporal.w_sum / (r_temporal.m * p_hat(r_temporal.s.L_out));
    float3 n_view = normalize(sample.n_view);
    float3 wi = normalize(r_temporal.s.x_sample - sample.x_view);
    float3 col = AccumulationImage[launchID].rgb;
    if (uniformBufferObject.reuse)
    {
        col += r_temporal.s.f * abs(dot(wi, n_view)) * r_temporal.s.L_out * r_temporal.W;
    }
    OutputImage[launchID] = float4(col, 1.0);
}