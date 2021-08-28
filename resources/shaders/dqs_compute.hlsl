#include "dq.hlsl"

struct VertexData
{
	float4 position;
	float3 normal;
	float2 uv;
	int4 boneIndices;
	float4 boneWeights;
};

struct VertexStream
{
	float4 position;
	float3 normal;
	float2 uv;
};

StructuredBuffer<VertexData> vertexData : register(t0);
StructuredBuffer<DQ> bonePoses : register(t1);
StructuredBuffer<DQ> boneBindPoses : register(t1);
RWStructuredBuffer<VertexStream> vertexStream : register(u0);

[numthreads(1024, 1, 1)]
void dqs( uint3 threadID : SV_DispatchThreadID )
{
	for (uint vrtID = threadID[0]; vrtID < vertexCount; vrtID += 1024)
	{
		int offset = frameIndex * boneCount;
		VertexData data = vertexData[vrtID];
		matrix t = mul(bonePoses[offset + data.index[0]], boneBindPoses[data.index[0]]);
		matrix wt = t * data.weight[0];
		float3 p0 = mul(t, float4(data.position, 1)).xyz;

		t = mul(bonePoses[offset + data.index[1]], boneBindPoses[data.index[1]]);
		wt += t * data.weight[1];
		float3 p1 = mul(t, float4(data.position, 1)).xyz - p0;

		t = mul(bonePoses[offset + data.index[2]], boneBindPoses[data.index[2]]);
		wt += t * data.weight[2];
		float3 p2 = mul(t, float4(data.position, 1)).xyz - p0;

		t = mul(bonePoses[offset + data.index[3]], boneBindPoses[data.index[3]]);
		wt += t * data.weight[3];
		float3 p3 = mul(t, float4(data.position, 1)).xyz - p0;

		vertexStream[tid.x].position = p0 + p1 * data.weight[1] + p2 * data.weight[2] + p3 * data.weight[3];
		vertexStream[tid.x].normal = normalize(mul(wt, vertexData[vrtID].normal).xyz);
		vertexStream[tid.x].uv = vertexData[vrtID].uv;
	}
}
