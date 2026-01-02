uint InitRandomSeed(uint2 pixelCoords, uint2 resolution, uint frameNumber)
{
    uint seed = dot(pixelCoords, uint2(1, resolution.x)); // 线性化坐标
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed * (frameNumber + 1);
}
uint NextRandom(inout uint state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}
float RandomFloat(inout uint state)
{
    return float(NextRandom(state) & 0x00FFFFFF) / 16777216.0f;
}
float RandomFloatSNorm(inout uint state)
{
    return RandomFloat(state) * 2.0f - 1.0f;
}
float2 RandomInUnitDisk(inout uint state)
{
    float a = RandomFloat(state) * 6.28318530718; // 角度
    float r = sqrt(RandomFloat(state)); // 半径 (开根号为了均匀分布)
    
    float s, c;
    sincos(a, s, c);
    return float2(c * r, s * r);
}