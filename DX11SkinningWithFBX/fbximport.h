#include <functional>
#include "datatypes.h"

#pragma once

struct FBXLoadOptionChunk
{
	union
	{
		struct
		{
			uint flipface : 1;
			uint flipU : 1;
			uint flipV : 1;
			uint fillVertexNomral : 1;
			uint fillVertexTangent : 1;
			uint fillVertexBinormal : 1;
			uint padding : 2;
		};
		byte flags;
	};
	uint sampleFPS : 8;
	uint fbxSkinIndex : 8;

	FBXLoadOptionChunk() : flags(0), sampleFPS(60), fbxSkinIndex(0) { }
};

struct FBXPivotChunk
{
	double prevRotation[3];
	double postRotation[3];
	double rotationPivot[3];
	double rotationOffset[3];
	double scalingPivot[3];
	double scalingOffset[3];
};

struct FBXMeshChunk
{
	char* name;
	struct FBXGeometryChunk
	{
		Bounds bound;
		uint vertexCount;
		Vector3f* vertices;
		Vector3f* normals;
		Vector3f* tangents;
		Vector3f* binormals;
		uint uvSlotCount;
		Vector2f** uvSlots;
		Vector4i* boneIndices;
		Vector4f* boneWeights;
		uint indexCount;
		uint* indices;
	} geometry;
	struct FBXSubmesh
	{
		uint submeshCount;
		struct Submesh
		{
			uint indexStart;
			uint indexCount;
			uint materialRef;
		}*submeshs;
	} submesh;
};

struct FBXHierarchyNode
{
	char* name;
	int parentIndex;
	int childIndexStart;
	int childCount;
	Matrix4x4 inverseGlobalTransformMatrix;
	TRS localTransform;
};

struct FBXChunk
{
	// mesh
	uint meshCount;
	FBXMeshChunk* meshs;

	// material
	uint materialPropertyCount;
	uint* textureRefs;

	// hierarchy
	uint hierarchyCount;
	FBXHierarchyNode* hierarchyNodes;

	// animation
	uint animationCount;
	struct FBXAnimation
	{
		char* animationName;
		uint frameKeyCount;
		uint fpsCount;
		Matrix4x4* globalAffineTransforms;
	}* animations;

	// allocater
	const Allocaters* allocs;

	FBXChunk() :
		meshCount(0), meshs(nullptr), materialPropertyCount(0), textureRefs(0),
		hierarchyCount(0), hierarchyNodes(nullptr), animationCount(0), animations(nullptr), 
		allocs(nullptr)
	{}
	~FBXChunk()
	{
	}
};

void InitializeFBX();
void DestroyFBX();

bool ImportFBX(
	const wchar_t* fileDirectory, FBXChunk& chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
bool ImportFBX(
	const char* fileDirectory, FBXChunk& chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
uint ImportFBX(
	uint chunkCount, const char* const * fileDirectories, FBXChunk* chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
uint ImportFBX(
	uint chunkCount, const wchar_t* const * fileDirectories, FBXChunk* chunk, 
	const FBXLoadOptionChunk* opt = nullptr, const Allocaters* allocs = nullptr
);
void ReleaseFBX(FBXChunk* c, const Allocaters* allocs);
