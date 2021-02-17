#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <functional>

#include "datatypes.h"
#include "allocators.h"

#pragma once

enum class PipelineKind : uint
{
	Draw,
	Compute,
	Copy,
};

enum class CopyKind : uint
{
	CopyResource,
	UpdateSubResource,
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
	union CopyArgs
	{
		struct CopyResourceArgs
		{
			uint srcBufferIndex;
			uint dstBufferIndex;
		} copyRes;
		struct UpdateSubResourceArgs
		{
			ResourceKind resKind;
			uint resIndex;
			uint dstSubres;
			std::function<const D3D11_BOX*(D3D11_BOX*)> getBoxFunc;
			uint srcRowPitch;
			uint srcDepthPitch;

			uint dataBufferSize;
			void* param;
			std::function<void(void*, void*)> copyToBufferFunc;

			UpdateSubResourceArgs() : getBoxFunc(), copyToBufferFunc() {}
		} updateSubRes;
		CopyArgs() {}
		~CopyArgs() {}
	} args;

	DX11CopyDependancy() 
	{
		new (&args.updateSubRes) DX11CopyDependancy::CopyArgs::UpdateSubResourceArgs();
	}
	DX11CopyDependancy(CopyKind kind) : kind(kind)
	{
		switch (kind)
		{
		case CopyKind::CopyResource:
			new (&args.copyRes) DX11CopyDependancy::CopyArgs::CopyResourceArgs();
			break;
		case CopyKind::UpdateSubResource:
			new (&args.updateSubRes) DX11CopyDependancy::CopyArgs::UpdateSubResourceArgs();
			break;
		}
	}
	DX11CopyDependancy(const DX11CopyDependancy& d) : kind(d.kind) 
	{
		switch (kind)
		{
		case CopyKind::CopyResource:
			new (&args.copyRes) DX11CopyDependancy::CopyArgs::CopyResourceArgs(d.args.copyRes);
			break;
		case CopyKind::UpdateSubResource:
			new (&args.updateSubRes) DX11CopyDependancy::CopyArgs::UpdateSubResourceArgs(d.args.updateSubRes);
			break;
		}
	}
	DX11CopyDependancy& operator=(const DX11CopyDependancy& d)
	{
		kind = d.kind;
		switch (kind)
		{
		case CopyKind::CopyResource:
			new (&args.copyRes) DX11CopyDependancy::CopyArgs::CopyResourceArgs(d.args.copyRes);
			break;
		case CopyKind::UpdateSubResource:
			new (&args.updateSubRes) DX11CopyDependancy::CopyArgs::UpdateSubResourceArgs(d.args.updateSubRes);
			break;
		}
		return *this;
	}
	~DX11CopyDependancy() { }
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
		None,
		Dispatch,
		DispatchIndirect,
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
	DX11ComputePipelineDependancy() : dispatchType(DispatchType::None), resources() { }
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
	DX11DrawPipelineDependancy() : drawType(DrawType::None)
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
	PipelineKind pipelineKind;
	union
	{
		DX11DrawPipelineDependancy draw;
		DX11ComputePipelineDependancy compute;
		DX11CopyDependancy copy;
	};

	DX11PipelineDependancy() 
	{
		new (&copy) DX11CopyDependancy();
	}
	DX11PipelineDependancy(CopyKind ck) : pipelineKind(PipelineKind::Copy)
	{
		new (&copy) DX11CopyDependancy(ck);
	}
	DX11PipelineDependancy(const DX11PipelineDependancy& d)
		: pipelineKind(d.pipelineKind)
	{
		switch (pipelineKind)
		{
		case PipelineKind::Draw:
			draw = d.draw;
			break;
		case PipelineKind::Compute:
			compute = d.compute;
			break;
		case PipelineKind::Copy:
			copy = d.copy;
			break;
		}
	}
	DX11PipelineDependancy(DX11PipelineDependancy&& d)
		: pipelineKind(d.pipelineKind)
	{
		switch (pipelineKind)
		{
		case PipelineKind::Draw:
			draw = eastl::move(d.draw);
			break;
		case PipelineKind::Compute:
			compute = eastl::move(d.compute);
			break;
		case PipelineKind::Copy:
			copy = eastl::move(d.copy);
			break;
		}
	}
	~DX11PipelineDependancy() {}
};
struct DX11PipelineDependancySet
{
	uint initDependancyCount;
	DX11PipelineDependancy* initDependancy;
	uint resizeDependancyCount;
	DX11PipelineDependancy* resizeDependancy;
	uint frameDependancyCount;
	DX11PipelineDependancy* frameDependancy;

	DX11PipelineDependancySet() :
		initDependancyCount(0), initDependancy(nullptr), resizeDependancyCount(0), resizeDependancy(nullptr),
		frameDependancyCount(0), frameDependancy(nullptr)
	{
	}
};

void PrintPipelineDependancy(const char* prefix, const DX11PipelineDependancy& d);

struct RenderContextState;
struct DX11Resources;

HRESULT ExecuteExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT ExecuteImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT CopyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11CopyDependancy* depends);
HRESULT ComputeExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends);
HRESULT ComputeImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends);
HRESULT DrawExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends);
HRESULT DrawImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends);
HRESULT ReleaseDX11Dependancy(uint dependCount, DX11PipelineDependancy* dependancy);
HRESULT ReleaseDX11Dependancy(DX11PipelineDependancySet* set);

struct RenderContextState
{
	uint bufferCount;
	void** bufferPtrBuffer;
	uint* numberBuffer;
};

HRESULT DependancyContextStatePrepare(RenderContextState* state, const DX11PipelineDependancySet* set);
HRESULT DependancyContextStatePrepare(RenderContextState* state, uint dependCount, const DX11PipelineDependancy* depends);
HRESULT ReleaseContext(RenderContextState* context);
