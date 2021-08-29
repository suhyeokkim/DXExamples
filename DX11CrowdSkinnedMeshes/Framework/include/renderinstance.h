#include "dx11resdesc.h"

#pragma once

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
	DeformedVertexBufferForSkin,
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

struct DX11Resources;
struct DX11InternalResourceDescBuffer;
struct DX11PipelineDependancySet;

HRESULT LoadResourceAndDependancyFromInstance(
	IN ID3D11Device* device, IN uint instanceCount, IN RenderInstance* instances,
	OUT DX11Resources* res, OUT DX11InternalResourceDescBuffer* rawBuffer, OUT DX11PipelineDependancySet* set
);