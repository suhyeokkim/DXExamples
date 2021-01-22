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
};
struct DX11UAVDesc
{
	uint bufferIndex;
	D3D11_UNORDERED_ACCESS_VIEW_DESC view;
};
struct DX11ILDesc
{
	uint vertexLayoutChunkIndex;
	uint vertexShaderIndex;
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
			std::vector<ShaderCompileDesc> vertexShaderCompileDescs;
			std::vector<ShaderCompileDesc> pixelShaderCompileDescs;
			std::vector<ShaderCompileDesc> computeShaderCompileDescs;
			std::vector<ShaderCompileDesc> geometryShaderCompileDescs;
			std::vector<ShaderCompileDesc> hullShaderCompileDescs;
			std::vector<ShaderCompileDesc> domainShaderCompileDescs;
		};
		struct
		{
			std::vector<ShaderCompileDesc> shaderCompileDesces[6];
		};
	};
	std::vector<DX11ILDesc> inputLayoutDescs;
	std::vector<D3D11_SAMPLER_DESC> samplerDescs;
	std::vector<DX11BufferDesc> bufferDescs;
	std::vector<DX11UAVDesc> uavDescs;
	std::vector<DX11SRVDesc> srvDescs;
	std::vector<DX11Texture2DDesc> tex2DDescs;

	DX11InternalResourceDescBuffer() :
		samplerDescs(), bufferDescs(), uavDescs(), srvDescs(), inputLayoutDescs(), /*shaderTex2DDescs(),*/
		tex2DDescs(), vertexShaderCompileDescs(), pixelShaderCompileDescs(), computeShaderCompileDescs(),
		geometryShaderCompileDescs(), hullShaderCompileDescs(), domainShaderCompileDescs()
	{
	}
	~DX11InternalResourceDescBuffer() { }
};

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