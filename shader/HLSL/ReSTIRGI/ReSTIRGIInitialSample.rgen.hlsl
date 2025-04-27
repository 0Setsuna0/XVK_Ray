#include "RayPayload.hlsl"
#include "Vertex.hlsl"
#include "UBO.hlsl"
#include "BSDF.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWStructuredBuffer<ReSTIRGISample> samples : register(u9);
//test
RWTexture2D<float4> AccumulationImage : register(u1);
RWTexture2D<float4> OutputImage : register(u2);

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
   
    RayPayload rayPayload;
    rayPayload.RandomSeed = InitRandomSeed(
        InitRandomSeed(launchID.x, launchID.y),
        uniformBufferObject.currentFrame
    );

    uint pixelRandomSeed = uniformBufferObject.randomSeed;
    float2 pixelOffset = float2(
            RandomFloat(pixelRandomSeed),
            RandomFloat(pixelRandomSeed));
    float2 pixel = launchID + pixelOffset;
    uint pixelIndex = launchID.y * launchSize.x + launchID.x;//row major
    
    uint seed2 = InitRandomSeed(
        InitRandomSeed(uniformBufferObject.currentFrame, launchID.y + pixelIndex),
        pixelIndex
    );
    
    float2 uv = (pixel / (float2) launchSize) * 2.0 - 1.0; //texture uv range[-1, 1]
    
    //ray generation   
    float2 offset = RandomInUnitDisk(rayPayload.RandomSeed);
    float4 origin = mul(uniformBufferObject.modelViewInverse, float4(0, 0, 0, 1));
    float4 target = mul(uniformBufferObject.projectionInverse, float4(uv.x, uv.y, 1, 1));
    float4 direction = mul(uniformBufferObject.modelViewInverse, float4(normalize(target.xyz), 0));
    //sample
    float3 x_view = 0;
    float3 n_view = 0;
    float3 x_sample = 0;
    float3 n_sample = 0;
    float3 L_out = 0;
    float3 f_view = 0;
    uint mat_idx = -1;
    float p_q = 0;
    float3 pixelColor = 0;
    
    float3 throughput = float3(1, 1, 1); //used for accumulating bsdf impact
    float3 t0 = float3(1, 1, 1); //t0 is the bsdf factor of first hitting
    for (uint bounce = 0; bounce < uniformBufferObject.numberOfBounces; bounce++)
    {         
        RayDesc ray;
        ray.Origin = origin.xyz;
        ray.Direction = direction.xyz;
        ray.TMin = 0.01f;
        ray.TMax = 1000.0f;
            
        TraceRay(
                scene, 
                RAY_FLAG_NONE, 
                0xFF, 
                0, 
                0, 
                0, 
                ray, 
                rayPayload); 
            
        float t = rayPayload.Distance;
        //defined in ray miss shader, material index = -1

        bool foundHit = rayPayload.MaterialIndex != -1;
        if (!foundHit)
        {
            float3 val = throughput * rayPayload.SkyColor.rgb;
            if(bounce <= 1)
            {
                pixelColor += t0 * val;//第二次或者第一次就miss，则直接记录到临时color buffer
            }
            else
            {
                L_out += val;//否则记录到L_out
            }
            break;
        }
        const float3 wo = -direction;
        float3 pos = rayPayload.Position;
        float3 normal = rayPayload.Normal;
        if (bounce == 1)
        {
            x_sample = pos;
            n_sample = normal;
        }
        
        const BSDFMaterial material = Materials[rayPayload.MaterialIndex];
                
        //sample direction and update throughput
        uint2 seed = uint2(rayPayload.RandomSeed, seed2);
        float pdf, cos_theta;
        float3 dir = 0;
        
        const float3 f = SampleBSDF(
            normal,
            rayPayload.UV,
            material,
            wo,
            true,
            dir,
            pdf,
            cos_theta,
            seed
        );
             
        if (bounce == 0)
        {
            x_view = pos;
            n_view = normal;
            p_q = pdf;
            f_view = f;
            mat_idx = material.materialModel;
        }
        
        if (material.materialModel == BSDFDiffuseLight)
        {
            //f here simply means the light color, just end the loop
            float3 mat_emissive = SampleDiffuseLight(material, rayPayload.UV);
            if (bounce != 2 && bounce != 0)
            {
                L_out += throughput * mat_emissive * 4;
            }
            else if (bounce == 0)
            {
                pixelColor += mat_emissive * 4;
            }
            break;
        }
        
        //update throughput
        if (material.materialModel != BSDFDiffuseLight)
        {
            if (bounce > 0)
            {
                throughput *= f * abs(cos_theta) / pdf;
            }
            else
            {
                t0 = f * abs(cos_theta) / pdf;
            }
        }
        
        origin.xyz = pos + dir * 1e-3;
        direction.xyz = dir;
    }
    
    samples[pixelIndex].x_view = x_view;
    samples[pixelIndex].n_view = n_view;
    samples[pixelIndex].x_sample = x_sample;
    samples[pixelIndex].n_sample = n_sample;
    samples[pixelIndex].L_out = L_out;
    samples[pixelIndex].f = f_view;
    samples[pixelIndex].materialID = mat_idx;
    samples[pixelIndex].p_q = p_q;
    float3 tempcolor = pixelColor + t0 * L_out;

    //bool accumulate = uniformBufferObject.spp != uniformBufferObject.totalNumberOfSamples;
    //float3 accumulatedColor = accumulate ?
    //    AccumulationImage[launchID].rgb :
    //    float3(0, 0, 0);
    //accumulatedColor += tempcolor;

    //float3 finalColor = accumulatedColor / uniformBufferObject.totalNumberOfSamples;
    //finalColor = sqrt(finalColor);
    
    AccumulationImage[launchID] = float4(pixelColor, 0);
    //OutputImage[launchID] = float4(pixelColor, 0);
}