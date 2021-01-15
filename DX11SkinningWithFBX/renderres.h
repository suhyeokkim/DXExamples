#pragma once

#include <DirectXMath.h>
#include "dx11res.h"

enum class ShaderParamKind : uint
{
	None,
	ConstantBuffer,
	Texture2DSRV,
	SamplerState,
	ExistBufferSRV,
	ExistBufferUAV,
};

enum class UpdateFrequency : byte
{
	PerFrame,
	OnResize,
	OnlyOnce
};

struct FileGeometryParams
{
	const wchar_t* filePath;
	const char* meshName;
};

struct FileAnimationParams
{
	const wchar_t* filePath;
	const char* animationName;
};

struct ShaderParamCB
{
	const wchar_t* name;
	uint size;
	UpdateFrequency freq;
	enum class ExistParam : uint
	{
		None,
		SkinningInstanceIndex
	} existParam;
	void* param;
	std::function<void(void*, void*)> setFunc;
	bool unique;

	ShaderParamCB() : setFunc(), unique(false) {}
	ShaderParamCB(
		const wchar_t* name, uint size, UpdateFrequency freq, std::function<void(void*, void*)> setFunc, bool unique
	) : name(name), size(size), freq(freq), existParam(ExistParam::None), 
		param(nullptr), setFunc(setFunc), unique(unique)
	{ }
	ShaderParamCB(
		const wchar_t* name, uint size, UpdateFrequency freq,
		ShaderParamCB::ExistParam existParam, std::function<void(void*, void*)> setFunc, bool unique
	) : name(name), size(size), freq(freq), existParam(existParam), param(nullptr), setFunc(setFunc), unique(unique)
	{ }
	ShaderParamCB(
		const wchar_t* name, uint size, UpdateFrequency freq,
		void* param, std::function<void(void*, void*)> setFunc, bool unique
	) : name(name), size(size), freq(freq), existParam(ExistParam::None), param(param), setFunc(setFunc), unique(unique)
	{ }
};

template<typename T, typename... U>
size_t getAddress(std::function<T(U...)> f) {
	typedef T(fnType)(U...);
	fnType ** fnPointer = f.template target<fnType*>();
	return (size_t)*fnPointer;
}
inline bool operator==(const ShaderParamCB& s0, const ShaderParamCB& s1)
{
	return !(s0.unique || s1.unique) && wcscmp(s0.name, s1.name) == 0;
}

struct ShaderParamTex2DSRV
{
	const wchar_t* filePath;
};
inline bool operator==(const ShaderParamTex2DSRV& s0, const ShaderParamTex2DSRV& s1)
{
	return wcscmp(s0.filePath, s1.filePath) == 0;
}

struct ShaderParamSampler
{
	const wchar_t* name;
	bool isLinear;
};
inline bool operator==(const ShaderParamSampler& s0, const ShaderParamSampler& s1)
{
	return s0.isLinear == s1.isLinear;
}

enum class ExistSRVKind : uint
{
	GeometryVertexBufferForSkin,
	BindPoseBufferForSkin,
	AnimationBufferForSkin,
};
struct ShaderParamExistSRV
{
	ExistSRVKind kind;

	ShaderParamExistSRV(ExistSRVKind kind) : kind(kind) {}
};

enum class ExistUAVKind : uint
{
	DeformedVertexBufferForSkin,
};
struct ShaderParamExistUAV
{
	ExistUAVKind kind;
	ShaderParamExistUAV(ExistUAVKind kind) : kind(kind) {}
};

struct ShaderParams
{
	ShaderParamKind kind;
	uint slotIndex;
	union
	{
		ShaderParamCB cb;
		ShaderParamTex2DSRV tex2DSRV;
		ShaderParamSampler sampler;
		ShaderParamExistUAV existUAV;
		ShaderParamExistSRV existSRV;
	};

	ShaderParams() {}
	ShaderParams(
		uint slotIndex, const wchar_t* name, uint size, UpdateFrequency freq,
		std::function<void(void*, void*)> setFunc, bool unique) :
		kind(ShaderParamKind::ConstantBuffer), slotIndex(slotIndex), cb(name, size, freq, setFunc, unique)
	{}
	ShaderParams(
		uint slotIndex, const wchar_t* name, uint size, UpdateFrequency freq,
		ShaderParamCB::ExistParam existParam, std::function<void(void*, void*)> setFunc, bool unique) :
		kind(ShaderParamKind::ConstantBuffer), slotIndex(slotIndex),
		cb(name, size, freq, existParam, setFunc, unique)
	{}
	ShaderParams(
		uint slotIndex, const wchar_t* name, uint size, UpdateFrequency freq,
		void* param, std::function<void(void*, void*)> setFunc, bool unique) :
		kind(ShaderParamKind::ConstantBuffer), slotIndex(slotIndex),
		cb(name, size, freq, param, setFunc, unique)
	{}
	ShaderParams(uint slotIndex, ExistSRVKind srvKind) :
		kind(ShaderParamKind::ExistBufferSRV), slotIndex(slotIndex), existSRV(srvKind)
	{}
	ShaderParams(uint slotIndex, ExistUAVKind uavKind) :
		kind(ShaderParamKind::ExistBufferUAV), slotIndex(slotIndex), existUAV(uavKind)
	{}
	~ShaderParams() {}
};

struct ShaderInstance
{
	uint paramCount;
	ShaderParams* params;
	ShaderCompileDesc sd;
};

enum ShaderFlags
{
	EnableVertex = 0x01,
	EnablePixel = 0x02,
	EnableGeometry = 0x04,
	EnableHull = 0x08,
	EnableDomain = 0x10,
	ForceInt = 0xffffffff
};

struct RenderInstance
{
	bool isSkinDeform;
	ShaderFlags shaderFlag;
	FileGeometryParams geometry;
	union
	{
		struct
		{
			ShaderInstance vs;
			ShaderInstance ps;
			ShaderInstance gs;
			ShaderInstance hs;
			ShaderInstance ds;
		};
		ShaderInstance si[5];
	};
	ShaderInstance skinCSParam;
	FileAnimationParams anim;
};

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
		uint animationIndex;
		uint vertexBufferIndex;
		uint vertexStreamBufferIndex;
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
	uint compileCount, const ShaderCompileDesc* descs, DX11CompileDescToShader* descToFileShader
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

struct DX11PipelineDependancy;
HRESULT LoadResourceAndDependancyFromInstance(
	IN ID3D11Device* device, IN const Allocaters* allocs, IN uint instanceCount, IN RenderInstance* instances,
	OUT RenderResources* res, OUT DX11InternalResourceDescBuffer* rawBuffer, OUT uint* dependancyCount, OUT DX11PipelineDependancy** dependancies
);