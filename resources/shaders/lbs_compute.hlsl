struct VertexData
{
	float3 position;
	float3 normal;
	float2 uv;
	int4 index;
	float4 weight;
};

struct VertexStream
{
	float3 position;
	float3 normal;
	float2 uv;
};

cbuffer SkinningConfigCB : register(b0)
{
	uint vertexCount;
	uint poseOffset;
}

StructuredBuffer<VertexData> vertexData : register(t0);
StructuredBuffer<matrix> boneBindPoses : register(t1);
StructuredBuffer<matrix> bonePoses : register(t2);
RWStructuredBuffer<VertexStream> vertexStream : register(u0);

[numthreads(1024, 1, 1)]
void lbs(uint3 threadID : SV_DispatchThreadID)
{
	for (uint vrtID = threadID[0]; vrtID < vertexCount; vrtID += 1024)
	{
		VertexData data = vertexData[vrtID];
		matrix t = mul(bonePoses[poseOffset + data.index[0]], boneBindPoses[data.index[0]]);
		matrix wt = t * data.weight[0];
		float3 p0 = mul(t, float4(data.position, 1)).xyz;

		t = mul(bonePoses[poseOffset + data.index[1]], boneBindPoses[data.index[1]]);
		wt += t * data.weight[1];
		float3 p1 = mul(t, float4(data.position, 1)).xyz - p0;

		t = mul(bonePoses[poseOffset + data.index[2]], boneBindPoses[data.index[2]]);
		wt += t * data.weight[2];
		float3 p2 = mul(t, float4(data.position, 1)).xyz - p0;

		t = mul(bonePoses[poseOffset + data.index[3]], boneBindPoses[data.index[3]]);
		wt += t * data.weight[3];
		float3 p3 = mul(t, float4(data.position, 1)).xyz - p0;

		vertexStream[vrtID].position = p0 + p1 * data.weight[1] + p2 * data.weight[2] + p3 * data.weight[3];
		vertexStream[vrtID].normal = normalize(mul(wt, float4(vertexData[vrtID].normal, 1)).xyz);
		vertexStream[vrtID].uv = vertexData[vrtID].uv;
	}
}
