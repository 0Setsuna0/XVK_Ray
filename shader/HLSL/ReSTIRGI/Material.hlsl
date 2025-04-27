#define BSDFLambertian 0
#define BSDFMetallic 1
#define BSDFDielectric 2
#define BSDFIsotropic 3
#define BSDFDiffuseLight 4

struct BSDFMaterial
{
    float4 baseColor;
    int baseColorTextureIndex;
    float fuzziness;
    float refractionIndex;
    uint materialModel;
};
