#include "ACES.hlsl"
cbuffer PerDrawcall : register(b0)
{
    float4x4 objectToClip;
    float3 camera_positionWS;
};
struct VertexInputType
{
    float3 Position : POSITION;
    float3 normalOS : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PixelInputType
{
    float4 Position : SV_POSITION;
    float3 normalWS : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 posWS : TEXCOORD1;
};

//========================================

struct PBRMaterial
{
    float3 albedo;
    float3 normalWS;
    float roughness;
    float metallness;
};

inline half4 Pow5(half4 x)
{
    return x * x * x * x * x;
}

inline half3 FresnelTerm(half3 F0, half cosA)
{
    half t = Pow5(1 - cosA); // ala Schlick interpoliation
    return F0 + (1 - F0) * t;
}

inline half SmithJointGGXVisibilityTerm(half NdotL, half NdotV, half roughness)
{
    // Approximation of the above formulation (simplify the sqrt, not mathematically correct but close enough)
    half a = roughness;
    half lambdaV = NdotL * (NdotV * (1 - a) + a);
    half lambdaL = NdotV * (NdotL * (1 - a) + a);

    return 0.5f / (lambdaV + lambdaL + 1e-5f);
}

#define dot2(x) (dot(x,x))

#define PI (3.14159)
#define PI_inv (1.0 / PI)
#define DiElectricSpecular (0.4)

inline float GGXTerm(float NdotH, float roughness)
{
    float a2 = roughness * roughness;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0f; // 2 mad
    return PI_inv * a2 / (d * d + 1e-7f); // This function is not intended to be running on Mobile,
                                            // therefore epsilon is smaller than what can be represented by half
}

inline half OneMinusReflectivityFromMetallic(half metallic)
{
    // We'll need oneMinusReflectivity, so
    //   1-reflectivity = 1-lerp(dielectricSpec, 1, metallic) = lerp(1-dielectricSpec, 0, metallic)
    // store (1-dielectricSpec) in unity_ColorSpaceDielectricSpec.a, then
    //   1-reflectivity = lerp(alpha, 0, metallic) = alpha + metallic*(0 - alpha) =
    //                  = alpha - metallic * alpha
    half oneMinusDielectricSpec = DiElectricSpecular;
    return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
}

inline half3 DiffuseAndSpecularFromMetallic(half3 albedo, half metallic, out half3 specColor, out half oneMinusReflectivity)
{
    specColor = lerp(DiElectricSpecular, albedo, metallic);
    oneMinusReflectivity = OneMinusReflectivityFromMetallic(metallic);
    return albedo * oneMinusReflectivity;
}

half DisneyDiffuse(half NdotV, half NdotL, half LdotH, half perceptualRoughness)
{
    half fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    // Two schlick fresnel term
    half lightScatter = (1 + (fd90 - 1) * Pow5(1 - NdotL));
    half viewScatter = (1 + (fd90 - 1) * Pow5(1 - NdotV));

    return lightScatter * viewScatter;
}

inline half3 GammaToLinearSpace(half3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011h + 0.682171111h) + 0.012522878h);

    // Precise version, useful for debugging.
    //return half3(GammaToLinearSpaceExact(sRGB.r), GammaToLinearSpaceExact(sRGB.g), GammaToLinearSpaceExact(sRGB.b));
}
inline half3 LinearToGammaSpace(half3 linRGB)
{
    linRGB = max(linRGB, half3(0.h, 0.h, 0.h));
    // An almost-perfect approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return max(1.055h * pow(linRGB, 0.416666667h) - 0.055h, 0.h);

    // Exact version, useful for debugging.
    //return half3(LinearToGammaSpaceExact(linRGB.r), LinearToGammaSpaceExact(linRGB.g), LinearToGammaSpaceExact(linRGB.b));
}

float3 ShadePBR(const float3 positionWS, const float3 viewDir, float3 lightPos, float3 lightColor, PBRMaterial material) {

    float3 specularColor;
    float oneMinusReflectivity;
    material.albedo = DiffuseAndSpecularFromMetallic(material.albedo, material.metallness, specularColor, oneMinusReflectivity);

    //===================================
    float atten = rcp(dot2(positionWS - lightPos) + 1.0);
    lightColor *= atten;

    //===================================
    const float3 normal = material.normalWS;
    const float3 lightDir = normalize(lightPos - positionWS);
    const float3 halfDir = normalize(lightDir + viewDir);

    float roughness = (material.roughness * material.roughness); //perceptual roughness to roughness
    const half nl = saturate(dot(normal, lightDir));
    const float nh = saturate(dot(normal, halfDir));
    const half lv = saturate(dot(lightDir, viewDir));
    const half lh = saturate(dot(lightDir, halfDir));
    const half nv = abs(dot(normal, viewDir)); // This abs allow to limit artifact

    half diffuseTerm = DisneyDiffuse(nv, nl, lh, roughness) * nl;
    
    half V = SmithJointGGXVisibilityTerm(nl, nv, roughness);
    half D = GGXTerm(nh, roughness);

    half specularTerm = max(0, V * D * PI) * nl;

    //return specularTerm * FresnelTerm(specularColor, lh) * lightColor;
    return 
        material.albedo * diffuseTerm * lightColor +
        specularTerm * FresnelTerm(specularColor, lh) * lightColor;

}

//src: http://www.thetenthplanet.de/archives/1180
float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
    // get edge vectors of the pixel triangle
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);
 
    // solve the linear system
    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    return float3x3(T * invmax, B * invmax, N);
}


// src: https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ToneMapping.hlsl
float3 ToneMap_Hejl2015(in float3 hdr)
{
    const float WhitePoint_Hejl = 100;
    float4 vh = float4(hdr, WhitePoint_Hejl);
    float4 va = (1.435f * vh) + 0.05;
    float4 vf = ((vh * va + 0.004f) / ((vh * (va + 0.55f) + 0.0491f))) - 0.0821f;
    return vf.xyz / vf.www;
}

float3 UnpackNormalMap(float2 normalMap)
{
    normalMap = normalMap * 2.0 - 1.0;
    return float3(
        normalMap,
        sqrt(1.0 - saturate(dot(normalMap, normalMap))) );
}

#define REINHARDT 1
float3 Tonemap(float3 color)
{
    float3 tonemapped;
#ifdef REINHARDT
    float maxVal = max(color.r, max(color.g, color.b));
    tonemapped = color * rcp(0.8 * maxVal + 1.0);
    
#elif ACES
    tonemapped = ACESFitted(color);
#elif HEJI
    tonemapped = ToneMap_Hejl2015(color);
#endif


    return GammaToLinearSpace(tonemapped);
}