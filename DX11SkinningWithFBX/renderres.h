#pragma once

#include <DirectXMath.h>
#include "dx11res.h"

enum class GraphicsAPIKind : uint
{
	DIRECTX_11,
};
struct RenderResources
{
	uint skinningCount;
	struct SkinningInstance
	{
		uint geometryIndex;
		uint vertexBufferIndex;
		uint vertexStreamBufferIndex;
		uint vertexStreamUAVIndex;
	}*skinningInstances;

	uint geometryCount;
	struct GeometryChunk
	{
		bool isSkinned;
		uint vertexCount;
		uint indexBufferIndex;
		uint indexCount;
		uint vertexLayoutIndex;
		struct
		{
			uint vertexBufferIndex;
		};
		// skinning
		struct
		{
			uint streamedVertexSize;
			uint vertexDataBufferIndex;
			uint vertexDataSRVIndex;
		};
	}*geometryChunks;

	uint boneSetCapacity;
	uint boneSetCount;
	struct BoneSet
	{
		uint boneCount;
		struct Bone
		{
			char* name;
			int parentIndex;
			int childIndexStart;
			int childCount;
			Matrix4x4 inverseGlobalTransformMatrix;
		}*bones;

		uint bindPoseTransformBufferIndex;
		uint binePoseTransformSRVIndex;
	}*boneSets;

	uint animCount;
	struct Animation
	{
		char* animName;
		uint frameKeyCount;
		uint fpsCount;
		uint animPoseTransformBufferIndex;
		uint animPoseTransformSRVIndex;
		uint boneSetIndex;
	}*anims;

	uint shaderTex2DCount;
	struct ShaderTexture2D
	{
		uint tex2DIndex;
		uint srvIndex;
	}*shaderTex2Ds;

	uint shaderFileCount;
	struct ShaderFile
	{
		wchar_t* fileName;
		union
		{
			struct
			{
				uint vsCount;
				uint* vsIndices;
				uint psCount;
				uint* psIndices;
				uint csCount;
				uint* csIndices;
				uint gsCount;
				uint* gsIndices;
				uint hsCount;
				uint* hsIndices;
				uint dsCount;
				uint* dsIndices;
			};
			struct
			{
				uint count;
				uint* indices;
			} shaderIndices[6];
		};
	}*shaderFiles;

	GraphicsAPIKind kind;
	union
	{
		DX11Resources dx11;
	};

	RenderResources() :
		geometryCount(0), geometryChunks(nullptr),
		shaderTex2DCount(0), shaderTex2Ds(nullptr),
		shaderFileCount(0), shaderFiles(nullptr),
		animCount(0), anims(nullptr),
		boneSetCount(0), boneSets(nullptr)
	{}
};

HRESULT LoadMeshAndAnimsFromFBXByDX11(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint chunkCount, const FBXChunk* chunks, const FBXChunkConfig* configs
);
HRESULT ReserveTex2DAndSRVFromFileByDX11(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint dirCount, const wchar_t** dirs, uint textureBufferSize = 0, void* allocatedtextureBuffer = nullptr
);
HRESULT ReserveShaderFromFileByDX11(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint compileCount, const DX11ShaderCompileDesc* descs, DX11CompileDescToShader* descToFileShader
);
HRESULT ReserveSkinningInstances(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint skinningInstanceCount, const SkinningInstanceDesc* skinningInstances
);

HRESULT LoadDX11Resoureces(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, DX11ResourceDesc* desc,
	const Allocaters* allocs, ID3D11Device* device
);
HRESULT ReleaseResources(RenderResources* res, const Allocaters* allocs);
