Texture2D diffuseTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer Tutoiral7ResizeCB : register(b1)
{
	matrix projection;
}
cbuffer Tutoiral7OnFrameCB : register(b2)
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
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

PS_INPUT vertex(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.pos = mul(transform, float4(input.pos, 1));
	output.pos = output.pos / output.pos.w;
	output.normal = mul(world, float4(input.normal, 0));
	output.uv = input.uv;

	return output;
}

float4 pixel(PS_INPUT input) : SV_Target
{
	float3 diffuse = diffuseTexture.Sample(linearSampler, input.uv).xyz;
	float clampCos = saturate(dot(input.normal, lightDir.xyz));

	return float4(diffuse * tint.xyz * (clampCos + (float)base.x), 1);
}
