#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define INV_PI (1. / PI)
#define sqrt2 1.41421356237309504880
#include "../random.hlsl"
struct ReSTIRGISample
{
    float3 x_view;   float _pad0;
    float3 n_view;   float _pad1;
    float3 x_sample; float _pad2;
    float3 n_sample; float _pad3;
    float3 L_out;    float _pad4;
    float3 f;        float _pad5;
    uint materialID;
    float p_q;       float2 _pad6;
};

struct ReSTIRGIReservoir
{
    float w_sum;
    float W;
    uint m;
    uint pad;

    ReSTIRGISample s;
};

void InitSample(out ReSTIRGISample s)
{
    s.x_view = float3(0, 0, 0);
    s.n_view = float3(0, 0, 0);
    s.x_sample = float3(0, 0, 0);
    s.n_sample = float3(0, 0, 0);
    s.L_out = float3(0, 0, 0);
    s.f = float3(0, 0, 0);
    s.materialID = 0;
    s.p_q = 0;
}

void InitReservoir(out ReSTIRGIReservoir r)
{
    r.w_sum = 0;
    r.W = 0;
    r.m = 0;
    r.pad = 0;
    InitSample(r.s);
}

void UpdateReservoir(inout ReSTIRGIReservoir r, const ReSTIRGISample s, float w_new, uint seed)
{
    r.w_sum += w_new;
    r.m++;
    if (RandomFloat(seed) < w_new / r.w_sum)
    {
        r.s = s;
    }
}

void MergeReservoir(inout ReSTIRGIReservoir target, const ReSTIRGIReservoir source, float p_hat, uint seed)
{
    uint M0 = target.m;

    //p_hat * source.w_sum * source.m
    float w_new = p_hat * source.w_sum * float(source.m);

    UpdateReservoir(target, source.s, w_new, seed);

    target.m = M0 + source.m;
}

float p_hat(const float3 f)
{
    return length(f);
}

//used for geometric similarity test in spatial reuse pass
bool Similar(ReSTIRGISample q, ReSTIRGISample q_n)
{
    const float depth_threshold = 0.5;
    const float angle_threshold = 25 * PI / 180;
    if (dot(q_n.n_view, q.n_view) < cos(angle_threshold))
    {
        return false;
    }
    return true;
}