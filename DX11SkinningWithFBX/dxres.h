#pragma once

#include <vector>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include "fbximport.h"

// TODO:: 
// TODO:: 리소스 로드 후 인덱스 반환

struct DX11ShaderCompileDesc
{
	const wchar_t* fileName;
	const char* entrypoint;
	const char* target;
};
struct DX11Texture2DDesc
{
	wchar_t* fileName;
	void* bufferPtr;
	D3D11_TEXTURE2D_DESC desc;
};
struct DX11BufferDesc
{
	wchar_t* fileName;
	void* bufferPtr;
	std::function<void(void*)> copyToPtr;
	D3D11_BUFFER_DESC buffer;
	D3D11_SUBRESOURCE_DATA subres;
};
struct DX11SRVDesc
{
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

struct DX11RawResourceBuffer
{
	union
	{
		struct
		{
			std::vector<DX11ShaderCompileDesc> vertexShaderCompileDescs;
			std::vector<DX11ShaderCompileDesc> pixelShaderCompileDescs;
			std::vector<DX11ShaderCompileDesc> computeShaderCompileDescs;
			std::vector<DX11ShaderCompileDesc> geometryShaderCompileDescs;
			std::vector<DX11ShaderCompileDesc> hullShaderCompileDescs;
			std::vector<DX11ShaderCompileDesc> domainShaderCompileDescs;
		};
		struct
		{
			std::vector<DX11ShaderCompileDesc> shaderCompileDesces[6];
		};
	};
	std::vector<DX11ILDesc> inputLayoutDescs;
	std::vector<D3D11_SAMPLER_DESC> samplerDescs;
	std::vector<DX11BufferDesc> bufferDescs;
	std::vector<DX11UAVDesc> uavDescs;
	std::vector<DX11SRVDesc> srvDescs;
	std::vector<DX11Texture2DDesc> tex2DDescs;

	DX11RawResourceBuffer() : 
		samplerDescs(), bufferDescs(), uavDescs(), srvDescs(), inputLayoutDescs(), tex2DDescs(),
		vertexShaderCompileDescs(), pixelShaderCompileDescs(), computeShaderCompileDescs(),
		geometryShaderCompileDescs(), hullShaderCompileDescs(), domainShaderCompileDescs()
	{
	}
	~DX11RawResourceBuffer() { }
};

uint AppendInputLayouts(DX11RawResourceBuffer* rawResBuffer, uint additionalCount, const DX11ILDesc* ilDescs);
uint AppendConstantBuffers(DX11RawResourceBuffer* rawResBuffer, uint constantBufferCount, const uint* bufferSizes);
void AppendShaders(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11ShaderCompileDesc* descs, OUT ShaderKind* kinds, OUT int* indices);
uint AppendSamplerStates(DX11RawResourceBuffer* rawResBuffer, uint samplerCount, const D3D11_SAMPLER_DESC* descs);
uint AppendShaderResourceViews(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11SRVDesc* descs);
uint AppendUnorderedAccessViews(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const  DX11UAVDesc* descs);
uint AppendBuffers(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11BufferDesc* descs);
uint AppendTex2Ds(DX11RawResourceBuffer* rawResBuffer, uint additioanlCount, const DX11Texture2DDesc* descs);

uint AppendInputLayout(DX11RawResourceBuffer* rawResBuffer, const DX11ILDesc& desc);
uint AppendConstantBuffer(DX11RawResourceBuffer* rawResBuffer, const uint bufferSize);
uint AppendShader(DX11RawResourceBuffer* rawResBuffer, const DX11ShaderCompileDesc* desc, OUT ShaderKind* s);
uint AppendSamplerState(DX11RawResourceBuffer* rawResBuffer, const D3D11_SAMPLER_DESC* desc);
uint AppendShaderResourceView(DX11RawResourceBuffer* rawResBuffer, const DX11SRVDesc* desc);
uint AppendUnorderedAccessViews(DX11RawResourceBuffer* rawResBuffer, const DX11UAVDesc* desc);
uint AppendBuffer(DX11RawResourceBuffer* rawResBuffer, const DX11BufferDesc* desc);
uint AppendTex2D(DX11RawResourceBuffer* rawResBuffer, const DX11Texture2DDesc* desc);

struct DX11CompileDescToShader
{
	uint shaderKindIndex;
	uint shaderIndex;
};
struct FBXChunkConfig
{
	struct FBXMeshConfig
	{
		bool isSkinned;
	}* meshConfigs;
};
struct DX11InputLayoutDesc
{
	uint layoutChunkIndex;
	uint shaderCompileDescIndex;
};
struct DX11ResourceDesc
{
	uint fbxChunkCount;
	const FBXChunk* fbxChunks;
	const FBXChunkConfig* fbxMeshConfigs;
	uint shaderCompileCount;
	const DX11ShaderCompileDesc* shaderCompileDescs;
	uint inputLayoutCount;
	const DX11InputLayoutDesc* inputLayoutDescs;
	uint textureDirCount;
	const wchar_t** texturedirs;
	uint textureBufferSize = 0;
	void* allocatedtextureBuffer = nullptr;

	uint constantBufferCount;
	const uint* constantBufferSizes;
	uint samplerCount;
	const D3D11_SAMPLER_DESC* samplerDescs;
};
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
	uint vertexLayoutCount;
	struct DX11LayoutChunk
	{
		uint vertexSize;
		uint descCount;
		D3D11_INPUT_ELEMENT_DESC* descs;
	}*vertexLayouts;

	// TODO:: 묶어야함
	uint geometryCount;
	struct DX11GeometryChunk
	{
		bool isSkinned;
		uint vertexCount;
		uint indexCount;
		uint indexBufferIndex;
		uint vertexBufferIndex;
		uint vertexLayoutIndex;
		// skinning
		uint vertexDataBufferIndex;
		uint vertexDataSRVIndex;
		uint vertexStreamBufferIndex;
		uint vertexStreamUAVIndex;
	}*geometryChunks;

	uint boneCount;
	uint bindPoseTransformBufferIndex;
	uint binePoseTransformSRVIndex;

	uint animCount;
	struct DX11Animation
	{
		char* animName;
		uint frameKeyCount;
		uint fpsCount;
		uint animPoseTransformBufferIndex;
		uint animPoseTransformSRVIndex;
	}* anims;

	uint texture2DCount;
	struct DX11Texture2D
	{
		ID3D11Texture2D* texture;
		uint srvIndex;
	}* texture2Ds;

	uint shaderFileCount;
	struct DX11ShaderFile
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

	}* shaderFiles;

	uint inputLayoutCount;
	struct DX11InputLayout
	{
		uint layoutChunkIndex;
		uint vertexShaderIndex;
		uint inputLayoutIndex;
	}* inputLayouts;

	uint constantBufferCount;
	uint* constantBufferIndices;

	uint inputLayoutItemCount;
	ID3D11InputLayout** inputLayoutItems;

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

	uint bufferCount;
	ID3D11Buffer** buffers;
	uint samplerCount;
	ID3D11SamplerState** samplerStates;
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
		constantBufferCount(0), constantBufferIndices(nullptr),
		bufferCount(0), buffers(nullptr),
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

HRESULT CreateDX11RawResourcesByDesc(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device, bool isDebug
);
HRESULT LoadDX11Resoureces(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, DX11ResourceDesc* desc, 
	const Allocaters* allocs, ID3D11Device* device
);
HRESULT LoadGeometryAndAnimationFromFBXChunk(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device,
	uint chunkCount, const FBXChunk* chunks, const FBXChunkConfig* configs
);
HRESULT LoadTexture2DAndSRVFromDirectories(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device,
	uint dirCount, const wchar_t** dirs, uint textureBufferSize = 0, void* allocatedtextureBuffer = nullptr
);
HRESULT LoadShaderFromDirectories(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs, ID3D11Device* device,
	uint compileCount, const DX11ShaderCompileDesc* descs, DX11CompileDescToShader* descToFileShader
);
HRESULT LinkInputLayout(
	DX11Resources* res, DX11RawResourceBuffer* rawBuffer, const Allocaters* allocs,
	uint descCount, const DX11CompileDescToShader* dtoss,
	uint inputLayoutCount, const DX11InputLayoutDesc* inputLayoutDesc
);

HRESULT UploadConstantBuffer(
	DX11Resources* res, ID3D11DeviceContext* deviceContext, uint constantBufferIndex, void* uploadData
);
HRESULT ReleaseResources(DX11Resources* res, const Allocaters* allocs);

enum class PIPELINE_KIND : uint
{
	DRAW,
	COMPUTE,
	COPY,
};

struct DX11DrawPipelineDependancyDesc
{

};
struct DX11ComputePipelineDependancyDesc
{

};
struct DX11CopyDependancyDesc
{

};
struct DX11PipelineDependancyDesc
{
	PIPELINE_KIND pipelineKind;
	union
	{
		DX11DrawPipelineDependancyDesc draw;
		DX11ComputePipelineDependancyDesc compute;
		DX11CopyDependancyDesc copy;
	};
};

enum class CopyKind : uint
{
	COPY_RESOURCE
};
// dependant on DX11Resource
struct DX11CopyDependancy
{
	CopyKind kind;
	union
	{
		struct
		{
			uint srcBufferIndex;
			uint dstBufferIndex;
		};
	};
};
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
	}*constantBuffers;

	DX11ShaderResourceDependancy() :
		shaderFileIndex(-1), shaderIndex(0), srvCount(0), srvs(nullptr), uavCount(0), uavs(nullptr),
		samplerCount(0), samplers(nullptr), constantBufferCount(0), constantBuffers(nullptr)
	{ }
};
struct DX11ComputePipelineDependancy
{
	DX11ShaderResourceDependancy resources;
	enum class DispatchType : uint
	{
		NONE,
		DISPATCH,
		DISPATCH_INDIRECT,
	} dispatchType;
	union ArgsAsDispatchType
	{
		struct DispatchArgs
		{
			uint threadGroupCountX;
			uint threadGroupCountY;
			uint threadGroupCountZ;
		} dispatch;
		struct DispatchIndirectArgs
		{
			uint bufferIndex;
			uint alignedByteOffset;
		} dispatchIndirect;
	} argsAsDispatch;
	DX11ComputePipelineDependancy() : dispatchType(DispatchType::NONE), resources() { }
};
struct DX11DrawPipelineDependancy
{
	DX11InputLayoutDependancy input;
	union
	{
		struct
		{
			DX11ShaderResourceDependancy vs;
			DX11ShaderResourceDependancy ps;
			DX11ShaderResourceDependancy gs;
			DX11ShaderResourceDependancy hs;
			DX11ShaderResourceDependancy ds;
		};
		DX11ShaderResourceDependancy dependants[5];
	};
	enum class DrawType : uint
	{
		NONE,
		DRAW,
		DRAW_INDEXED
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
	DX11DrawPipelineDependancy() : drawType(DrawType::NONE)
	{
		new (dependants + 0) DX11ShaderResourceDependancy();
		new (dependants + 1) DX11ShaderResourceDependancy();
		new (dependants + 2) DX11ShaderResourceDependancy();
		new (dependants + 3) DX11ShaderResourceDependancy();
		new (dependants + 4) DX11ShaderResourceDependancy();
	}
};
struct DX11PipelineDependancy
{
	PIPELINE_KIND pipelineKind;
	union
	{
		DX11DrawPipelineDependancy draw;
		DX11ComputePipelineDependancy compute;
		DX11CopyDependancy copy;
	};
};
struct DX11ContextState
{
	uint bufferCount;
	void** bufferPtrBuffer;
	uint* numberBuffer;
};

HRESULT DependancyContextStatePrepare(DX11ContextState* state, const Allocaters* allocs, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT ExecuteExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT ExecuteImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT Copy(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11CopyDependancy* depends);
HRESULT ComputeExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends);
HRESULT ComputeImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends);
HRESULT DrawExplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends);
HRESULT DrawImplicitly(ID3D11DeviceContext* deviceContext, DX11ContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends);
HRESULT ReleaseDependancy(DX11PipelineDependancy* dependancy, const Allocaters* allocs);
HRESULT ReleaseContext(DX11ContextState* context, const Allocaters* allocs);
