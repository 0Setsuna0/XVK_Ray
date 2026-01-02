struct UniformBufferObject
{
    float4x4 modelView;
    float4x4 viewPrev;
	float4x4 projection;
    float4x4 projectionPrev;
	float4x4 modelViewInverse;
	float4x4 projectionInverse;
    uint spp;
	uint totalNumberOfSamples;
	uint numberOfBounces;
	uint randomSeed;
    bool hasSkyBox;
    bool reuse;
    uint currentFrame;
};