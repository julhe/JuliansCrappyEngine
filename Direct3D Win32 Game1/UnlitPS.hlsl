#include "Common.hlsl"

SamplerState diffuseTextureSampler : register(s0);

Texture2D diffuseTexture : register(t0);
Texture2D roughnessTexture : register(t1);
Texture2D metalnessTexture : register(t2);
Texture2D normalTexture : register(t3);
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ColorPixelShader(PixelInputType i) : SV_TARGET
{
    float3 viewDir = normalize(i.posWS - camera_positionWS);

    PBRMaterial mat;
    float4 albedoAlpha = diffuseTexture.Sample(diffuseTextureSampler, i.uv);
    clip(albedoAlpha.a - 0.5);
    mat.albedo = albedoAlpha.rgb;
    mat.metallness = metalnessTexture.Sample(diffuseTextureSampler, i.uv);

    float3x3 tangentToWorld = cotangent_frame(normalize(i.normalWS), -viewDir, i.uv);
    float3 normalMap = UnpackNormalMap(normalTexture.Sample(diffuseTextureSampler, i.uv).xy);
    mat.normalWS =  normalize(mul(transpose(tangentToWorld), normalMap));

    mat.roughness = roughnessTexture.Sample(diffuseTextureSampler, i.uv);

    float4 outColor = float4(0, 0, 0, 1);

    {
        float3 light = { -500, 50, -0 };
        float3 light_color = { 0.792156899, 0.984313750, 0.929411829 };
        light_color *= 225000;
        outColor.rgb += ShadePBR(i.posWS, -viewDir, light, light_color, mat);
    }    
    {
        float3 light = { 500, 50, 0 };
        float3 light_color = { 0.992156899, 0.984313750, 0.729411829 };
        light_color *= 225000;
        outColor.rgb += ShadePBR(i.posWS, -viewDir, light, light_color, mat);
    }

    outColor.rgb += float3(0.4, 0.4, 0.42) * mat.albedo; // ambient light
    outColor.rgb = Tonemap(outColor.rgb);
   // outColor.rgb = normalMap * 0.5 + 0.5;
  

    return outColor;
}