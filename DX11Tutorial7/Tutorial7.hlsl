Texture2D diffuseTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer Tutoiral7ImmutableCB : register(b0)
{
	matrix view;
}
cbuffer Tutoiral7ResizeCB : register(b1)
{
	matrix projection;
}
cbuffer Tutoiral7OnFrameCB : register(b2)
{
	matrix world;
	float4 meshColor;
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
};

PS_INPUT vertex(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.pos = mul(world, float4(input.pos, 1));
	output.pos = mul(view, output.pos);
	output.pos = mul(projection, output.pos);
	output.uv = input.uv;

	return output;
}

float4 pixel(PS_INPUT input) : SV_Target
{ 
	return diffuseTexture.Sample(linearSampler, input.uv) * meshColor;
}
