#include "RayPayload.hlsl"
#include "Vertex.hlsl"
#include "UBO.hlsl"
#include "BSDF.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWStructuredBuffer<ReSTIRGISample> samples : register(u9);
//test
RWTexture2D<float4> AccumulationImage : register(u1);
RWTexture2D<float4> OutputImage : register(u2);
//GBuffer
RWTexture2D<float4> GBufferPos : register(u12);
RWTexture2D<float4> GNormal : register(u13);
RWTexture2D<float4> GMatUV : register(u14);
RWTexture2D<float2> GMotionVector : register(u15);
RWTexture2D<float4> GBufferPosPrev : register(u16);
RWTexture2D<float4> GNormalPrev : register(u17);

StructuredBuffer<BSDFMaterial> Materials : register(t6);

cbuffer UniformBuffer : register(b3)
{
    UniformBufferObject uniformBufferObject;
};

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
    uint pixelIndex = launchID.y * launchSize.x + launchID.x;
    uint rngState = InitRandomSeed(launchID, launchSize, uniformBufferObject.currentFrame);
    float3 pixelColor = 0;
    float2 pixelOffset = float2(
            RandomFloat(rngState),
            RandomFloat(rngState));
    float2 pixel = (float2) launchID + pixelOffset;
    float2 uv = (pixel / (float2) launchSize) * 2.0 - 1.0;

    float4 origin = mul(uniformBufferObject.modelViewInverse, float4(0, 0, 0, 1));
    float4 target = mul(uniformBufferObject.projectionInverse, float4(uv.x, uv.y, 1, 1));
        
    float3 direction = normalize(mul(uniformBufferObject.modelViewInverse, float4(normalize(target.xyz), 0)).xyz);

    float3 rayColor = 0;
    float3 throughput = 1.0;
    float3 throughput0 = 1.0;
    float testPDF = 0.0;
    float2 testUV = 0.0;
    
    //sample
    float3 x_view = 0;
    float3 n_view = 0;
    float3 x_sample = 0;
    float3 n_sample = 0;
    float3 L_out = 0;
    float3 f_ = 0;
    float p_q = 0;
    uint mat_idx = -1;
    uint rand_seed = 0;
    float3 t0 = float3(1, 1, 1); //t0 is the bsdf factor of first hitting
    bool primitiveHitValid = false;
    for (uint bounce = 0;; bounce++)
    {   
        if(bounce == uniformBufferObject.numberOfBounces)
            break;
        RayDesc ray;
        ray.Origin = origin.xyz;
        ray.Direction = direction.xyz;
        ray.TMin = 0.0001f;
        ray.TMax = 10000.0f;
            
        RayPayload rayPayload;
        rayPayload.MaterialIndex = -1;
            
        rayPayload.Position = float3(0, 0, 0);
        rayPayload.Normal = float3(0, 0, 0);
        rayPayload.UV = float2(0, 0);
        rayPayload.HitT = 0.0f;
        
        TraceRay(
                scene, 
                RAY_FLAG_NONE, 
                0xFF, 
                0, 
                0, 
                0, 
                ray, 
                rayPayload); 
            
        //defined in ray miss shader, material index = -1

        bool foundHit = rayPayload.MaterialIndex != -1;
        if (rayPayload.MaterialIndex == -1)
        {
            float3 skyColor = float3(1, 1, 1);  
            rayColor += uniformBufferObject.hasSkyBox ? throughput0 * throughput * skyColor : 0;
            L_out += uniformBufferObject.hasSkyBox ? throughput * skyColor : 0;
            if(bounce == 1)
            {
                x_sample = origin.xyz + direction * 1000.0f;
                n_sample = -direction;
            }
            break;
        }
        BSDFMaterial mat = Materials[rayPayload.MaterialIndex];

        if (mat.materialModel == BSDFDiffuseLight)
        {
            rayColor += throughput0 * throughput * mat.baseColor.rgb * 4.0;
            L_out += throughput * mat.baseColor.rgb * 4.0;
            if (bounce == 1)
            {
                x_sample = rayPayload.Position;
                n_sample = rayPayload.Normal;
            }
            if (bounce == 0)
            {
                x_view = rayPayload.Position;
                n_view = rayPayload.Normal;
            }
            break;
        }

        float pdf = 0;
        float cos_theta = 0;
        float3 wi = 0;
        float3 wo = -direction;
        
        if (bounce == 1)
        {
            x_sample = rayPayload.Position;
            n_sample = rayPayload.Normal;
        }
        
        const BSDFMaterial material = Materials[rayPayload.MaterialIndex];
        uint seedForThisBounce = rngState;
        float3 f = SampleBSDF(rayPayload.Normal, rayPayload.UV, mat, wo,
                true, wi, pdf, cos_theta, rngState);
             
        if (bounce == 0)
        {
            primitiveHitValid = true;
            x_view = rayPayload.Position;
            n_view = rayPayload.Normal;
            rand_seed = seedForThisBounce;
            mat_idx = rayPayload.MaterialIndex;
            throughput0 *= f * cos_theta / pdf;
            testUV = rayPayload.UV;
            p_q = pdf;
            f_ = f;
        }
        else if (bounce > 0)
        {
            throughput *= f * cos_theta / pdf;
        }

        if (bounce > 3)
        {
            float p = max(throughput.r, max(throughput.g, throughput.b));
            if (RandomFloat(rngState) > p)
                break;
            throughput *= 1.0 / p;
        }

        float3 hitPosition = ray.Origin + ray.Direction * rayPayload.HitT;
        float3 geoNormal = rayPayload.GeometryNormal;
        float3 offsetDir = dot(wi, geoNormal) > 0 ? geoNormal : -geoNormal;
        origin.xyz = hitPosition + offsetDir * 0.01f;

        direction = wi;
    }
    pixelColor += rayColor;
    samples[pixelIndex].x_view = x_view;
    samples[pixelIndex].n_view = n_view;
    samples[pixelIndex].x_sample = x_sample;
    samples[pixelIndex].n_sample = n_sample;
    samples[pixelIndex].L_out = L_out;
    samples[pixelIndex].materialID = mat_idx;
    samples[pixelIndex].randSeed = rand_seed;
    samples[pixelIndex].p_q = p_q;
    samples[pixelIndex].f = f_;
    
    GBufferPosPrev[launchID] = GBufferPos[launchID];
    GNormalPrev[launchID] = GNormal[launchID];

    if(primitiveHitValid)
    {
        GBufferPos[launchID] = float4(x_view, 1.0); // w=1 means valid geometry
        GNormal[launchID] = float4(n_view, 0.0);
        GMatUV[launchID] = float4(float(mat_idx), testUV.x, testUV.y, 0.0);
        float2 currentUV = (float2(launchID) + 0.5) / float2(launchSize);
        float2 prevUV = WorldToScreenUV(x_view, uniformBufferObject.viewPrev, uniformBufferObject.projectionPrev);
        GMotionVector[launchID] = currentUV - prevUV;
    }
    else
    {
        GBufferPos[launchID] = float4(0, 0, 0, 0.0);
        GNormal[launchID] = 0;
        GMatUV[launchID] = float4(-1, 0, 0, 0);
        GMotionVector[launchID] = 0;
        samples[pixelIndex].L_out = float3(0, 0, 0);
        samples[pixelIndex].n_sample = float3(0, 0, 0);
    }
}