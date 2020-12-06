cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

struct VS_INPUT
{
	float4 Pos : POSITION;
	float4 Color : COLOR;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

PS_INPUT vertex(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(World, input.Pos);
	output.Pos = mul(View, output.Pos);
	output.Pos = mul(Projection, output.Pos);
	output.Color = input.Color;

	return output;
}

float4 pixel(PS_INPUT input) : SV_Target
{
	return input.Color;
}
