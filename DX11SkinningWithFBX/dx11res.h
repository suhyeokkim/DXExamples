#include <dxgi1_6.h>
#include <d3d11_4.h>
#include "dx11resdesc.h"
#include "fbximport.h"

#pragma once

struct DX11CompiledShader
{
	char* entrypoint;
	char* target;
	ID3DBlob* shaderBlob;
	union
	{
		void* shaderPtr;
		ID3D11VertexShader* vs;
		ID3D11PixelShader* ps;
		ID3D11ComputeShader* cs;
		ID3D11GeometryShader* gs;
		ID3D11HullShader* hs;
		ID3D11DomainShader* ds;
	};

	operator ID3D11VertexShader*() { return vs; }
	operator ID3D11PixelShader*() { return ps; }
	operator ID3D11ComputeShader*() { return cs; }
	operator ID3D11GeometryShader*() { return gs; }
	operator ID3D11HullShader*() { return hs; }
	operator ID3D11DomainShader*() { return ds; }
};

struct DX11Resources
{
	uint skinningCount;
	struct SkinningInstance
	{
		uint geometryIndex;
		uint animationIndex;
		//uint vertexBufferIndex;
		uint vertexStreamBufferIndex;
		uint vertexStreamSRVIndex;
		uint vertexStreamUAVIndex;
	}*skinningInstances;

	uint geometryCount;
	struct GeometryChunk
	{
		bool isSkinned;
		Bounds bound;
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

	uint vertexLayoutCount;
	struct DX11LayoutChunk
	{
		uint vertexSize;
		uint descCount;
		D3D11_INPUT_ELEMENT_DESC* descs;
	}*vertexLayouts;

	uint inputLayoutCount;
	struct DX11InputLayout
	{
		uint layoutChunkIndex;
		uint vertexShaderIndex;
		uint inputLayoutIndex;
	}* inputLayouts;

	uint constantBufferCount;
	uint* constantBufferIndices;

	union
	{
		struct
		{
			uint vsCount;
			DX11CompiledShader* vss;
			uint psCount;
			DX11CompiledShader* pss;
			uint csCount;
			DX11CompiledShader* css;
			uint gsCount;
			DX11CompiledShader* gss;
			uint hsCount;
			DX11CompiledShader* hss;
			uint dsCount;
			DX11CompiledShader* dss;
		} shaders;
		struct 
		{
			uint shaderCount;
			DX11CompiledShader* shaders;
		} shadersByKind[6];
	};

	uint texture2DCount;
	ID3D11Texture2D** texture2Ds;
	uint inputLayoutItemCount;
	ID3D11InputLayout** inputLayoutItems;
	uint bufferCount;
	ID3D11Buffer** buffers;
	uint samplerCount;
	ID3D11SamplerState** samplerStates;
	uint srvCount;
	ID3D11ShaderResourceView** srvs;
	uint uavCount;
	ID3D11UnorderedAccessView** uavs;

	DX11Resources() : 
		geometryCount(0), geometryChunks(nullptr),
		shaderTex2DCount(0), shaderTex2Ds(nullptr),
		shaderFileCount(0), shaderFiles(nullptr),
		animCount(0), anims(nullptr),
		boneSetCount(0), boneSets(nullptr),
		vertexLayoutCount(0), vertexLayouts(nullptr), 
		inputLayoutCount(0), inputLayouts(nullptr),
		samplerCount(0), samplerStates(nullptr),
		constantBufferCount(0), constantBufferIndices(nullptr),
		bufferCount(0), buffers(nullptr),
		uavCount(0), uavs(nullptr),
		srvCount(0), srvs(nullptr),
		texture2DCount(0), texture2Ds(nullptr)
	{ }
};

inline bool ShaderTargetToIndex(wchar_t wc, uint* i)
{
	switch (wc)
	{
	case 'v':
		*i = 0;
		break;
	case 'p':
		*i = 1;
		break;
	case 'c':
		*i = 2;
		break;
	case 'g':
		*i = 3;
		break;
	case 'h':
		*i = 4;
		break;
	case 'd':
		*i = 5;
		break;
	default:
		return false;
	}

	return true;
}

struct DX11ResourceDesc;
struct DX11InternalResourceDescBuffer;
struct DX11CompileDescToShader;
struct DX11InputLayoutDesc;
struct ShaderCompileDesc;
struct FBXChunkConfig;
struct SkinningInstanceDesc;

HRESULT LoadDX11Resoureces(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, DX11ResourceDesc* desc,
	const Allocaters* allocs, ID3D11Device* device
);
HRESULT CreateDX11ResourcesByDesc(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device, bool isDebug
);
HRESULT UploadDX11ConstantBuffer(
	DX11Resources* res, ID3D11DeviceContext* deviceContext, uint constantBufferIndex, void* uploadData
);
HRESULT ReleaseResources(DX11Resources* res, const Allocaters* allocs);

HRESULT ReserveLoadInputLayoutRefIndex(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint descCount, const DX11CompileDescToShader* dtoss,
	uint inputLayoutCount, const DX11InputLayoutDesc* inputLayoutDesc
);
HRESULT LoadMeshAndAnimsFromFBXByDX11(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint chunkCount, const FBXChunk* chunks, const FBXChunkConfig* configs
);
HRESULT ReserveTex2DAndSRVFromFileByDX11(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint dirCount, const wchar_t** dirs, uint textureBufferSize = 0, void* allocatedtextureBuffer = nullptr
);
HRESULT ReserveShaderFromFileByDX11(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint compileCount, const ShaderCompileDesc* descs, DX11CompileDescToShader* descToFileShader
);
HRESULT ReserveSkinningInstances(
	DX11Resources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint skinningInstanceCount, const SkinningInstanceDesc* skinningInstances
);

