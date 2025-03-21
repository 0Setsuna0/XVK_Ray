
struct Vertex
{
    float3 Position; // 顶点位置
    float3 Normal; // 法线
    float2 TexCoord; // 纹理坐标
    int MaterialIndex; // 材质索引
};

[[vk::binding(4, 0)]] 
ByteAddressBuffer VertexArray;

// 从字节地址缓冲区解包顶点数据
Vertex UnpackVertex(uint index)
{
    // 每个顶点的大小（9个浮点数）
    const uint vertexSize = 9;
    const uint offset = index * vertexSize;

    // 从字节地址缓冲区加载数据
    // 假设 VertexBufferIndex 是顶点缓冲区的描述符索引
    Vertex v;

    // 读取顶点位置
    v.Position.x = asfloat(VertexArray.Load((offset + 0) * 4));
    v.Position.y = asfloat(VertexArray.Load((offset + 1) * 4));
    v.Position.z = asfloat(VertexArray.Load((offset + 2) * 4));

    // 读取法线
    v.Normal.x = asfloat(VertexArray.Load((offset + 3) * 4));
    v.Normal.y = asfloat(VertexArray.Load((offset + 4) * 4));
    v.Normal.z = asfloat(VertexArray.Load((offset + 5) * 4));

    // 读取纹理坐标
    v.TexCoord.x = asfloat(VertexArray.Load((offset + 6) * 4));
    v.TexCoord.y = asfloat(VertexArray.Load((offset + 7) * 4));

    // 读取材质索引
    v.MaterialIndex = asint(VertexArray.Load((offset + 8) * 4));

    return v;
}