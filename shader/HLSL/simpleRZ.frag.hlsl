#include "material.hlsl"

struct PS_INPUT
{
    [[vk::location(0)]] float3 FragColor : COLOR0;
    [[vk::location(1)]] float3 FragNormal : NORMAL;
    [[vk::location(2)]] float2 FragTexCoord : TEXCOORD0;
    [[vk::location(3)]] int FragMaterialIndex : MATERIALINDEX;
};

struct PS_OUTPUT
{
    float4 OutColor : SV_Target0;
};

StructuredBuffer<Material> MaterialArray : register(t1);
Texture2D TextureSamplers[] : register(t2);
SamplerState TextureSamplersSampler : register(s2);

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;

    Material m = MaterialArray[input.FragMaterialIndex];

    const int textureId = m.baseColorTextureIndex;

    const float3 lightVector = normalize(float3(3, 0, 0));
    const float d = max(dot(lightVector, normalize(input.FragNormal)), 0.2f);

    float3 c = input.FragColor * d;

    if (textureId >= 0)
    {
        c *= TextureSamplers[textureId].Sample(TextureSamplersSampler, input.FragTexCoord).rgb;
    }

    output.OutColor = float4(c, 1.0f);
    return output;
}
