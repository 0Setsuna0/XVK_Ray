
#define MaterialLambertian 0
#define MaterialMetallic 1
#define MaterialDielectric 2
#define MaterialIsotropic 3
#define MaterialDiffuseLight 4

struct Material
{
    float4 baseColor;
    int baseColorTextureIndex;
    float fuzziness;
    float refractionIndex;
    uint materialModel;
};