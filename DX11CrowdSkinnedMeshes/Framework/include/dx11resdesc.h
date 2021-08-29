#pragma once

#include <vector>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include "datatypes.h"
#include "fbximport.h"

struct ShaderCompileDesc
{
	const wchar_t* fileName;
	const char* entrypoint;
	const char* target;
};

inline bool operator==(const ShaderCompileDesc& d0, const ShaderCompileDesc& d1)
{
	return
		wcscmp(d0.fileName, d1.fileName) == 0 &&
		strcmp(d0.entrypoint, d1.entrypoint) == 0 &&
		strcmp(d0.target, d1.target) == 0;
}

struct DX11ShaderTexture2DDesc
{
	uint srvIndex;
	const wchar_t* fileName;
	D3D11_USAGE usage;
	uint bindFlags;
	uint miscFlags;
	uint cpuAccessFlags;
	bool forceSRGB;
};
struct DX11Texture2DDesc
{
	bool loadFromFile;
	union
	{
		struct
		{
			const wchar_t* fileName;
			D3D11_USAGE usage;
			uint bindFlags;
			uint miscFlags;
			uint cpuAccessFlags;
			bool forceSRGB;
		};
		struct
		{
			D3D11_TEXTURE2D_DESC tex2DDesc;
			D3D11_SUBRESOURCE_DATA subres;
		};
	};
};
struct DX11BufferDesc
{
	wchar_t* fileName;
	std::function<void(void*)> copyToPtr;
	D3D11_BUFFER_DESC buffer;
	D3D11_SUBRESOURCE_DATA subres;

	DX11BufferDesc() : fileName(nullptr), copyToPtr() 
	{
		memset(&buffer, 0, sizeof(buffer));
		memset(&subres, 0, sizeof(subres));
	}
};
struct DX11SRVDesc
{
	std::function<bool(D3D11_SHADER_RESOURCE_VIEW_DESC*)> setSRVDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC view;
	union
	{
		uint bufferIndex;
		uint texture2DIndex;
	};

	DX11SRVDesc() : setSRVDesc(), bufferIndex(0)
	{
		memset(&view, 0, sizeof(view));
	}
};
struct DX11UAVDesc
{
	uint bufferIndex;
	D3D11_UNORDERED_ACCESS_VIEW_DESC view;

	DX11UAVDesc() : bufferIndex(0)
	{
		memset(&view, 0, sizeof(view));
	}
};
struct DX11ILDesc
{
	uint vertexLayoutChunkIndex;
	uint vertexShaderIndex;

	DX11ILDesc() : vertexLayoutChunkIndex(0), vertexShaderIndex(0) {}
};
enum class ShaderKind : uint
{
	Vertex = 0,
	Pixel,
	Compute,
	Geometry,
	Hull,
	Doamin
};

inline int ShaderTargetToIndex(char c)
{
	switch (c)
	{
	case 'v':
		return 0;
	case 'p':
		return 1;
	case 'c':
		return 2;
	case 'g':
		return 3;
	case 'h':
		return 4;
	case 'd':
		return 5;
	default:
		return -1;
	}
}

struct DX11InternalResourceDescBuffer
{
	union
	{
		struct
		{
			eastl::vector<ShaderCompileDesc, EASTLAllocator> vertexShaderCompileDescs;
			eastl::vector<ShaderCompileDesc, EASTLAllocator> pixelShaderCompileDescs;
			eastl::vector<ShaderCompileDesc, EASTLAllocator> computeShaderCompileDescs;
			eastl::vector<ShaderCompileDesc, EASTLAllocator> geometryShaderCompileDescs;
			eastl::vector<ShaderCompileDesc, EASTLAllocator> hullShaderCompileDescs;
			eastl::vector<ShaderCompileDesc, EASTLAllocator> domainShaderCompileDescs;
		};
		struct
		{
			eastl::vector<ShaderCompileDesc, EASTLAllocator> shaderCompileDesces[6];
		};
	};
	eastl::vector<DX11ILDesc, EASTLAllocator> inputLayoutDescs;
	eastl::vector<D3D11_SAMPLER_DESC, EASTLAllocator> samplerDescs;
	eastl::vector<DX11BufferDesc, EASTLAllocator> bufferDescs;
	eastl::vector<DX11UAVDesc, EASTLAllocator> uavDescs;
	eastl::vector<DX11SRVDesc, EASTLAllocator> srvDescs;
	eastl::vector<DX11Texture2DDesc, EASTLAllocator> tex2DDescs;

	DX11InternalResourceDescBuffer() :
		samplerDescs(EASTL_TEMPARARY_NAME), bufferDescs(EASTL_TEMPARARY_NAME), uavDescs(EASTL_TEMPARARY_NAME), 
		srvDescs(EASTL_TEMPARARY_NAME), inputLayoutDescs(EASTL_TEMPARARY_NAME), tex2DDescs(EASTL_TEMPARARY_NAME), 
		vertexShaderCompileDescs(EASTL_TEMPARARY_NAME), pixelShaderCompileDescs(EASTL_TEMPARARY_NAME), 
		computeShaderCompileDescs(EASTL_TEMPARARY_NAME), geometryShaderCompileDescs(EASTL_TEMPARARY_NAME), 
		hullShaderCompileDescs(EASTL_TEMPARARY_NAME), domainShaderCompileDescs(EASTL_TEMPARARY_NAME)
	{
	}
	~DX11InternalResourceDescBuffer() { }
};

size_t GetMaximumBufferSize(DX11InternalResourceDescBuffer* rawResBuffer);

uint ReserveLoadInputLayouts(DX11InternalResourceDescBuffer* rawResBuffer, uint additionalCount, const DX11ILDesc* ilDescs);
uint ReserveLoadConstantBuffers(DX11InternalResourceDescBuffer* rawResBuffer, uint constantBufferCount, const uint* bufferSizes);
void ReserveLoadShaders(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const ShaderCompileDesc* descs, OUT ShaderKind* kinds, OUT int* indices);
uint ReserveLoadSamplerStates(DX11InternalResourceDescBuffer* rawResBuffer, uint samplerCount, const D3D11_SAMPLER_DESC* descs);
uint ReserveLoadShaderResourceViews(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11SRVDesc* descs);
uint ReserveLoadUnorderedAccessViews(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const  DX11UAVDesc* descs);
uint ReserveLoadBuffers(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11BufferDesc* descs);
uint ReserveLoadTexture2Ds(DX11InternalResourceDescBuffer* rawResBuffer, uint additioanlCount, const DX11Texture2DDesc* descs);

uint ReserveLoadInputLayout(DX11InternalResourceDescBuffer* rawResBuffer, const DX11ILDesc& desc);
uint ReserveLoadConstantBuffer(DX11InternalResourceDescBuffer* rawResBuffer, const uint bufferSize);
uint ReserveLoadShader(DX11InternalResourceDescBuffer* rawResBuffer, const ShaderCompileDesc* desc, OUT ShaderKind* s);
uint ReserveLoadSamplerState(DX11InternalResourceDescBuffer* rawResBuffer, const D3D11_SAMPLER_DESC* desc);
uint ReserveLoadShaderResourceView(DX11InternalResourceDescBuffer* rawResBuffer, const DX11SRVDesc* desc);
uint ReserveLoadUnorderedAccessView(DX11InternalResourceDescBuffer* rawResBuffer, const DX11UAVDesc* desc);
uint ReserveLoadBuffer(DX11InternalResourceDescBuffer* rawResBuffer, const DX11BufferDesc* desc);
uint ReserveLoadTexture2D(DX11InternalResourceDescBuffer* rawResBuffer, const DX11Texture2DDesc* desc);

struct DX11CompileDescToShader
{
	// file to shader
	uint shaderFileIndex;
	uint shaderIndexInFile;
	// resource
	uint shaderKindIndex;
	uint shaderIndex;
};
struct FBXChunkConfig
{
	struct FBXMeshConfig
	{
		bool isSkinned;
		FBXMeshConfig() : isSkinned(false) {}
	}*meshConfigs;
};
struct DX11InputLayoutDesc
{
	uint layoutChunkIndex;
	uint shaderCompileDescIndex;
};
struct SkinningInstanceDesc
{
	uint geometryIndex;
	uint animationIndex;
};
struct DX11ResourceDesc
{
	uint fbxChunkCount;
	const FBXChunk* fbxChunks;
	const FBXChunkConfig* fbxMeshConfigs;
	uint shaderCompileCount;
	const ShaderCompileDesc* shaderCompileDescs;
	uint inputLayoutCount;
	const DX11InputLayoutDesc* inputLayoutDescs;
	uint textureDirCount;
	const wchar_t** texturedirs;
	uint constantBufferCount;
	const uint* constantBufferSizes;
	uint samplerCount;
	const D3D11_SAMPLER_DESC* samplerDescs;
	uint skinningInstanceCount;
	const SkinningInstanceDesc* skinningInstances;
};