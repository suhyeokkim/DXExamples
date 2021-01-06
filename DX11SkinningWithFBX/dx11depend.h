#pragma once

#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <functional>
#include <DirectXMath.h>
#include "renderres.h"

enum class PIPELINE_KIND : uint
{
	DRAW,
	COMPUTE,
	COPY,
};

enum class CopyKind : uint
{
	COPY_RESOURCE,
	UPDATE_SUBRESOURCE,
};
enum class ResourceKind : uint
{
	Buffer,
	Textur2D,
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
		struct
		{
			ResourceKind resKind;
			uint resIndex;
			uint dstSubres;
			std::function<const D3D11_BOX*(D3D11_BOX*)> getBoxFunc;
			uint dataBufferSize;
			std::function<void(void*)> copyToBufferFunc;
			uint srcRowPitch;
			uint srcDepthPitch;
		};
	};

	~DX11CopyDependancy() {}
};
struct DX11InputLayoutDependancy
{
	uint inputLayoutIndex;
	uint vertexBufferIndex;
	uint geometryIndex;
	uint vertexSize;
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

	~DX11PipelineDependancy() {}
};

struct RenderContextState;

HRESULT ExecuteExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT ExecuteImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT CopyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11CopyDependancy* depends);
HRESULT ComputeExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11ComputePipelineDependancy* depends);
HRESULT ComputeImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11ComputePipelineDependancy* depends);
HRESULT DrawExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11DrawPipelineDependancy* depends);
HRESULT DrawImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11DrawPipelineDependancy* depends);
HRESULT ReleaseDX11Dependancy(uint dependCount, DX11PipelineDependancy* dependancy, const Allocaters* allocs);

struct RenderContextState
{
	uint bufferCount;
	void** bufferPtrBuffer;
	uint* numberBuffer;
};

HRESULT DependancyContextStatePrepare(RenderContextState* state, const Allocaters* allocs, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT ReleaseContext(RenderContextState* context, const Allocaters* allocs);
