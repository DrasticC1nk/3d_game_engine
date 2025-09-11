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

struct PS_IN
{
    float4 pos : SV_POSITION;
    
    float3 col : COLOR;
    float3 fragPos : TEXCOORD0;
    float3 worldNormal : NORMAL;
};

float4 main(PS_IN input) : SV_TARGET
{
    float ambientStrength = 0.2f;
    float3 ambient = ambientStrength * lightColor.rgb * input.col;
    float3 norm = normalize(input.worldNormal);
    float3 lightDir = normalize(lightPos - input.fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = diff * lightColor.rgb * input.col;
    float3 viewDir = normalize(viewPos - input.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float3 specular = spec * lightColor.rgb;
    float3 result = ambient + diffuse + specular;
    
    return float4(result, 1.0f);
}