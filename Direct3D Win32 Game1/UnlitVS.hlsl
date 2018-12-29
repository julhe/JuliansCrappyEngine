
#include "Common.hlsl"

PixelInputType ColorVertexShader(VertexInputType v)
{
    PixelInputType o;
    

    // Change the Position vector to be 4 units for proper matrix calculations.
    //input.Position.w = 1.0f;

    // Calculate the Position of the vertex against the world, view, and projection matrices.
    //output.Position = mul(input.Position, worldMatrix);
    //output.Position = mul(output.Position, viewMatrix);
    //output.Position = mul(output.Position, projectionMatrix);
    
    o.Position = mul(objectToClip, float4(v.Position, 1.0));
    // Store the input color for the pixel shader to use.
    o.color = v.color;
    o.color *= dot(float3(0.5, 0.5, 0), v.normalOS) * 0.5 + 0.5;
    o.posWS = v.Position;
    o.normalWS = v.normalOS;
    o.uv = v.uv;
    return o;
}
