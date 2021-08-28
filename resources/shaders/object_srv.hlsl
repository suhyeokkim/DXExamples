cbuffer ResizeCB : register(b1)
{
	matrix projection;
}
cbuffer OnFrameCB : register(b2)
{
	matrix transform;
	matrix view;
	matrix world;
	float4 viewDir;
	float4 base;
	float4 tint;
	float4 lightDir;
}

struct VS_INPUT
{
	float3 pos;
	float3 normal;
	float2 uv;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

StructuredBuffer<VS_INPUT> vertexBuffer : register(t0);

PS_INPUT vertex(uint vi : SV_VertexID)
{
	VS_INPUT input = vertexBuffer[vi];
	PS_INPUT output = (PS_INPUT)0;
	output.pos = mul(transform, float4(input.pos, 1));
	output.pos = output.pos / output.pos.w;
	output.normal = mul(world, float4(input.normal, 0));
	output.uv = input.uv;

	return output;
}

Texture2D diffuseTexture : register(t1);
SamplerState linearSampler : register(s0);

float4 pixel(PS_INPUT input) : SV_Target
{ 
	float3 diffuse = diffuseTexture.Sample(linearSampler, input.uv).xyz;
	float clampCos = saturate(dot(input.normal, lightDir.xyz));
	return float4(diffuse * tint.xyz * (clampCos + (float)base.x), 1);
}
