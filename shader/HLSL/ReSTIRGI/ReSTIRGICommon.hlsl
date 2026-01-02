#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define INV_PI (1. / PI)
#define sqrt2 1.41421356237309504880
#include "../random.hlsl"
struct ReSTIRGISample
{
    float3 x_view;   
    uint materialID;
    float3 n_view;   
    uint randSeed;
    float3 x_sample;
    float p_q;
    float3 n_sample; 
    uint pad1;
    float3 L_out;
    uint pad2;
    float3 f;
    uint pad3;
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
    s.materialID = -1;
    s.randSeed = 0;
}

void InitReservoir(out ReSTIRGIReservoir r)
{
    r.w_sum = 0;
    r.W = 0;
    r.m = 0;
    r.pad = 0;
    InitSample(r.s);
}

void UpdateReservoir(inout ReSTIRGIReservoir r, const ReSTIRGISample s, float w_new, inout uint seed)
{
    r.w_sum += w_new;

    r.m++;
    if (RandomFloat(seed) < w_new / max(1e-10, r.w_sum))
    {
        r.s = s;
    }
}

void MergeReservoir(inout ReSTIRGIReservoir target, const ReSTIRGIReservoir source, float p_hat, inout uint seed)
{
    uint M0 = target.m;

    //p_hat * source.w_sum * source.m
    float w_new = p_hat * source.W * float(source.m);

    UpdateReservoir(target, source.s, w_new, seed);

    target.m = M0 + source.m;
}

float p_hat(const float3 f)
{
    return length(f);
}

float GetTargetPDF(float3 L, float3 x_curr, float3 n_curr, float3 x_sample)
{
    float3 wi = normalize(x_sample - x_curr);
    float cos_theta = max(0.0, dot(n_curr, wi));
    return length(L) * cos_theta;
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

bool IsSimilar(ReSTIRGISample current, ReSTIRGISample history)
{
    float dist2 = dot(current.x_view - history.x_view, current.x_view - history.x_view);
    if (dist2 > 0.1)
        return false;
    if (dot(current.n_view, history.n_view) < 0.8)
        return false;
    if(current.materialID != history.materialID)
        return false;
    return true;
}

bool IsSimilar(float3 currPos, float3 currNorm, float3 prevPos, float3 prevNorm)
{
    if (dot(currNorm, prevNorm) < 0.8) 
        return false;

    float dist2 = dot(currPos - prevPos, currPos - prevPos);
    if (dist2 > 0.1)
        return false;
    
    return true;
}

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}