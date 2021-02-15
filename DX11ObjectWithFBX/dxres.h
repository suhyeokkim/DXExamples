#pragma once

#include <dxgi1_6.h>
#include <d3d11_4.h>
#include "fbximport.h"

struct ShaderCompileDesc
{
	const wchar_t* fileName;
	const char* entrypoint;
	const char* target;
};
struct DX11InputLayoutDesc
{
	uint layoutChunkIndex;
	uint shaderCompileDescIndex;
};

struct DX11Resources
{
	uint vertexLayoutCount;
	struct DX11LayoutChunk
	{
		uint vertexSize;
		uint descCount;
		D3D11_INPUT_ELEMENT_DESC* descs;
	}*vertexLayouts;
	uint geometryCount;
	struct DX11GeometryChunk
	{
		uint vertexLayoutIndex;
		uint vertexCount;
		ID3D11Buffer* vertexBuffer;
		uint indexCount;
		ID3D11Buffer* indexBuffer;
	}*geometryChunks;
	
	uint texture2DCount;
	struct DX11Texture2D
	{
		ID3D11Texture2D* texture;
		uint srvIndex;
	}* texture2Ds;

	uint shaderFileCount;
	struct DX11ShaderFile
	{
		struct DX11CompiledShader
		{
			char* entrypoint;
			char* target;
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
		};

		const wchar_t* fileName;

		union
		{
			struct
			{
				uint vsBlobCount;
				DX11CompiledShader* vsBlobs;
				uint psBlobCount;
				DX11CompiledShader* psBlobs;
				uint csBlobCount;
				DX11CompiledShader* csBlobs;
				uint gsBlobCount;
				DX11CompiledShader* gsBlobs;
				uint hsBlobCount;
				DX11CompiledShader* hsBlobs;
				uint dsBlobCount;
				DX11CompiledShader* dsBlobs;
			};
			struct 
			{
				uint shaderCount;
				DX11CompiledShader* shaders;
			} shadersByKind[6];
		};

		DX11ShaderFile() : fileName(L"none"),
			vsBlobCount(0), vsBlobs(nullptr), psBlobCount(0), psBlobs(nullptr), csBlobCount(0), csBlobs(nullptr),
			gsBlobCount(0), gsBlobs(nullptr), hsBlobCount(0), hsBlobs(nullptr), dsBlobCount(0), dsBlobs(nullptr)
		{}
	}* shaderFiles;

	uint inputLayoutCount;
	struct DX11InputLayout
	{
		uint layoutChunkIndex;
		uint shaderFileIndex;
		uint vertexShaderIndex;
		ID3D11InputLayout* inputLayout;
	}* inputLayouts;

	uint samplerCount;
	ID3D11SamplerState** samplerStates;

	uint constantBufferCount;
	ID3D11Buffer** constantBuffers;

	uint srvCount;
	ID3D11ShaderResourceView** srvs;

	uint uavCount;
	ID3D11UnorderedAccessView** uavs;

	DX11Resources() : 
		vertexLayoutCount(0), vertexLayouts(nullptr), 
		geometryCount(0), geometryChunks(nullptr), 
		texture2DCount(0), texture2Ds(nullptr),
		shaderFileCount(0), shaderFiles(nullptr),
		inputLayoutCount(0), inputLayouts(nullptr),
		samplerCount(0), samplerStates(nullptr),
		uavCount(0), uavs(nullptr),
		srvCount(0), srvs(nullptr)
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

HRESULT LoadGeometryFromFBXChunk(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device, int chunkCount, FBXChunk* chunks
);
HRESULT LoadTexture2DAndSRVFromDirectories(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device, 
	int dirCount, const wchar_t** dirs, uint textureBufferSize = 0, void* allocatedtextureBuffer = nullptr
);
HRESULT LoadShaderFromDirectoriesAndInputLayout(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device, 
	int compileCount, const ShaderCompileDesc* descs, 
	int inputLayoutCount, const DX11InputLayoutDesc* inputLayoutDesc
);
HRESULT CreateSamplerStates(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device, 
	int samplerCount, D3D11_SAMPLER_DESC* descs
);
HRESULT CreateConstantBuffers(
	DX11Resources* res, const Allocaters* allocs, ID3D11Device* device,
	int contantBufferCount, const size_t* bufferSizes
);
HRESULT UploadConstantBuffer(
	DX11Resources* res, ID3D11DeviceContext* deviceContext, int constantBufferIndex, void* uploadData
);
HRESULT ReleaseResources(DX11Resources* res, const Allocaters* allocs);

// dependant on DX11Resource
struct DX11InputLayoutDependancy
{
	uint inputLayoutIndex;
	uint geometryIndex;
	uint vertexBufferOffset;
	D3D_PRIMITIVE_TOPOLOGY topology;
};
struct DX11ShaderResourceDependancy
{
	int shaderFileIndex : 16;
	uint shaderIndex : 16;
	uint srvCount;
	struct DX11SRVRef {
		uint slotOrRegister : 5;
		uint indexCount : 27;
		uint* indices;
	}*srvs;
	uint uavCount;
	struct DX11UAVRef {
		uint slotOrRegister : 5;
		uint indexCount : 27;
		uint* indices;
	}*uavs;
	uint samplerCount;
	struct DX11SamplerRef {
		uint slotOrRegister : 5;
		uint indexCount : 27;
		uint* indices;
	}*samplers;
	uint constantBufferCount;
	struct DX11ConstantBufferRef {
		uint slotOrRegister : 5;
		uint indexCount : 27;
		uint* indices;
	}* constantBuffers;

	DX11ShaderResourceDependancy() : 
		shaderFileIndex(-1), shaderIndex(0), srvCount(0), srvs(nullptr), uavCount(0), uavs(nullptr),
		samplerCount(0), samplers(nullptr), constantBufferCount(0), constantBuffers(nullptr)
	{ }
};
struct DX11PipelineDependancy
{
	DX11InputLayoutDependancy input;
	union
	{
		struct
		{
			DX11ShaderResourceDependancy vs;
			DX11ShaderResourceDependancy ps;
			DX11ShaderResourceDependancy cs;
			DX11ShaderResourceDependancy gs;
			DX11ShaderResourceDependancy hs;
			DX11ShaderResourceDependancy ds;
		};
		DX11ShaderResourceDependancy dependants[6];
	};
	enum class DrawType : uint 
	{
		None,
		Draw,
		DrawIndexed
	} drawType;
	union ArgsAsDrawType {
		struct DrawArgs
		{
			uint vertexCount;
			uint startVertexLocation;
		} drawArgs;
		struct DrawIndexedArgs
		{
			uint indexCount;
			uint startIndexLocation;
			sint baseVertexLocation;
		} drawIndexedArgs;
	} argsAsDraw;
	DX11PipelineDependancy() : drawType(DrawType::None)
	{
		new (dependants + 0) DX11ShaderResourceDependancy();
		new (dependants + 1) DX11ShaderResourceDependancy();
		new (dependants + 2) DX11ShaderResourceDependancy();
		new (dependants + 3) DX11ShaderResourceDependancy();
		new (dependants + 4) DX11ShaderResourceDependancy();
		new (dependants + 5) DX11ShaderResourceDependancy();
	}
};
struct DX11ContextState
{
	int bufferCount;
	void** bufferPtrBuffer;
	uint* numberBuffer;
};

HRESULT DrawingContextStatePrepare(DX11ContextState* state, const Allocaters* allocs, int dependCount, const DX11PipelineDependancy* depends);
HRESULT DrawExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, int dependCount, const DX11PipelineDependancy* depends);
HRESULT DrawImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, int dependCount, const DX11PipelineDependancy* depends);
HRESULT ReleaseDependancy(DX11PipelineDependancy* dependancy, const Allocaters* allocs);
HRESULT ReleaseContext(DX11ContextState* context, const Allocaters* allocs);
