cbuffer ConstantBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float4 lightDir[2];
	float4 lightColor[2];
	float4 outputColor;
}

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 normal : TEXCOORD0;
};

PS_INPUT vertex(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.pos = mul(world, float4(input.pos, 1));
	output.pos = mul(view, output.pos);
	output.pos = mul(projection, output.pos);
	output.normal = mul(world, input.normal);

	return output;
}

float4 pixel(PS_INPUT input) : SV_Target
{ 
	float4 color = 0;

	for (int i = 0; i < 2; i++)
		color += saturate(dot((float3)lightDir[i], input.normal) * lightColor[i]);
	color.a = 1;

	return color;
}

float4 pixelSolid(PS_INPUT input) : SV_Target
{
	return outputColor;
}
