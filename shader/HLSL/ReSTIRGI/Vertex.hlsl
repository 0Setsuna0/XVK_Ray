
struct Vertex
{
    float3 Position; 
    float3 Normal; 
    float2 TexCoord; 
    int MaterialIndex; 
};

ByteAddressBuffer VertexArray : register(t4);

Vertex UnpackVertex(uint index)
{
    const uint vertexSize = 9;
    const uint offset = index * vertexSize;

    Vertex v;

    v.Position.x = asfloat(VertexArray.Load((offset + 0) * 4));
    v.Position.y = asfloat(VertexArray.Load((offset + 1) * 4));
    v.Position.z = asfloat(VertexArray.Load((offset + 2) * 4));

    v.Normal.x = asfloat(VertexArray.Load((offset + 3) * 4));
    v.Normal.y = asfloat(VertexArray.Load((offset + 4) * 4));
    v.Normal.z = asfloat(VertexArray.Load((offset + 5) * 4));

    v.TexCoord.x = asfloat(VertexArray.Load((offset + 6) * 4));
    v.TexCoord.y = asfloat(VertexArray.Load((offset + 7) * 4));

    v.MaterialIndex = asint(VertexArray.Load((offset + 8) * 4));

    return v;
}