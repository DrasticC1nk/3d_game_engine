cbuffer ConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    
    float3 lightPos;
    float shininess;
    float3 viewPos;
    float4 lightColor;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 col : COLOR;
    float3 norm : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    
    float3 col : COLOR;
    float3 fragPos : TEXCOORD0;
    float3 worldNormal : NORMAL;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    float4 worldPos = mul(float4(input.pos, 1.0f), model);
    
    output.fragPos = worldPos.xyz;
    output.worldNormal = mul(input.norm, (float3x3) model);
    output.pos = mul(worldPos, view);
    output.pos = mul(output.pos, projection);
    output.col = input.col;
    
    return output;
}