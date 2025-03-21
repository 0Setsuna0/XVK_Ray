struct [raypayload] RayPayload
{
    [[vk::location(0)]] float4 ColorAndDistance; // 允许TraceRay返回该值
    [[vk::location(1)]] float4 ScatterDirection; // 允许TraceRay返回该值
    [[vk::location(2)]] uint RandomSeed; // 允许TraceRay返回该值
};