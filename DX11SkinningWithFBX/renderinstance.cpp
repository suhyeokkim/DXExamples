#include <array>
#include <map>

#include "fbximport.h"
#include "dx11res.h"
#include "dx11resdesc.h"
#include "dx11depend.h"
#include "renderinstance.h"

struct FBXAdjChunk
{
	FBXChunk chunk;
	FBXChunkConfig config;
	uint startGeometryCount;
	uint startAnimationCount;

	FBXAdjChunk(FBXChunk chunk, FBXChunkConfig config, uint startGeometryCount, uint startAnimationCount) :
		chunk(chunk), config(config), 
		startGeometryCount(startGeometryCount), startAnimationCount(startAnimationCount)
	{}
};
using PathFBXAdjChunkPair = std::pair<const wchar_t*, FBXAdjChunk>;

inline void GetDX11Sampler(ShaderParamSampler s, D3D11_SAMPLER_DESC& sd)
{
	sd = {
			s.isLinear? D3D11_FILTER_MIN_MAG_MIP_LINEAR: D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			0,
			0,
			D3D11_COMPARISON_NEVER,
			{ 0, 0, 0, 0 },
			0,
			0
	};
}

struct InstanceToDependancy
{
	int fbxGeometryChunkIndex;
	int meshIndex;
	int resGeometryIndex;
	int fbxAnimChunkIndex;
	int animIndex;
	int resAnimationIndex;
	int resInputLayoutIndex;
	int skinInstanceIndex;

	// shader iteration
	int shaderCompileIndices[6];
	// link between dependancy and resource
	struct SRVParamIndices
	{
		enum class SRVArrayKind : uint
		{
			Texture2D,
			ExistSRV,
		} kind;
		uint instanceParamIndex;
		uint refArrayIndex;

		SRVParamIndices(SRVArrayKind kind, uint instanceParamIndex, uint refArrayIndex) :
			kind(kind), instanceParamIndex(instanceParamIndex), refArrayIndex(refArrayIndex)
		{}
	};
	std::vector<SRVParamIndices> srvCurrentIndexVector[6];
	std::vector<std::pair<uint, uint>> samplerCurrentIndexVector[6];
	struct CBParamIndices
	{
		uint instanceIndex;
		uint instanceParamIndex;
		uint cbIndex;

		CBParamIndices(uint instanceIndex, uint instanceParamIndex, uint cbIndex)
			: instanceIndex(instanceIndex), instanceParamIndex(instanceParamIndex), cbIndex(cbIndex)
		{}
	};
	std::vector<CBParamIndices> cbCurrentIndexVector[6];
	std::vector<std::pair<uint, uint>> uavCurrentIndexVector[6];

	InstanceToDependancy()
	{
		fbxGeometryChunkIndex = -1;
		meshIndex = -1;
		resGeometryIndex = -1;
		fbxAnimChunkIndex = -1;
		animIndex = -1;
		resAnimationIndex = -1;
		resInputLayoutIndex = -1;
		skinInstanceIndex = -1;

		for (int i = 0; i < 6; i++)
		{
			shaderCompileIndices[i] = -1;
			new (srvCurrentIndexVector + i) std::vector<SRVParamIndices>();
			new (samplerCurrentIndexVector + i) std::vector<std::pair<uint, uint>>();
			new (cbCurrentIndexVector + i) std::vector<CBParamIndices>();
			new (uavCurrentIndexVector + i) std::vector<std::pair<uint, uint>>();
		}
	}
};

void SetShaderResourceDependancy(
	const DX11Resources* res, const Allocaters* allocs, 
	int shaderDescIndex, const DX11CompileDescToShader* cdToShader, const InstanceToDependancy& itod,
	int shaderIndex, ShaderInstance& shaderInstance, DX11ShaderResourceDependancy& srd
)
{
	srd.shaderFileIndex = cdToShader[shaderDescIndex].shaderFileIndex;
	srd.shaderIndex = cdToShader[shaderDescIndex].shaderIndexInFile;

	// constant buffer
	{
		const std::vector<InstanceToDependancy::CBParamIndices>& cbVector =
			itod.cbCurrentIndexVector[shaderIndex];
		srd.constantBufferCount = static_cast<uint>(cbVector.size());
		srd.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * srd.constantBufferCount
			);
		for (size_t i = 0; i < cbVector.size(); i++)
		{
			ShaderParams& p = shaderInstance.params[cbVector[i].instanceParamIndex];
			srd.constantBuffers[i].indexCount = 1;
			srd.constantBuffers[i].slotOrRegister = p.slotIndex;
			srd.constantBuffers[i].indices =
				(uint*)allocs->alloc(sizeof(uint) * srd.constantBuffers[i].indexCount);
			srd.constantBuffers[i].indices[0] = cbVector[i].cbIndex;
		}
	}

	// sampelr state
	{
		const std::vector<std::pair<uint, uint>>& samplerVector = itod.samplerCurrentIndexVector[shaderIndex];
		srd.samplerCount = static_cast<uint>(samplerVector.size());
		srd.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * srd.samplerCount
			);
		for (size_t i = 0; i < samplerVector.size(); i++)
		{
			ShaderParams& p = shaderInstance.params[samplerVector[i].first];
			srd.samplers[i].indexCount = 1;
			srd.samplers[i].slotOrRegister = p.slotIndex;
			srd.samplers[i].indices =
				(uint*)allocs->alloc(sizeof(uint) * srd.samplers[i].indexCount);
			srd.samplers[i].indices[0] = samplerVector[i].second;
		}
	}

	// uav
	{
		const std::vector<std::pair<uint, uint>>& uavVector = itod.uavCurrentIndexVector[shaderIndex];
		srd.uavCount = static_cast<uint>(uavVector.size());
		srd.uavs =
			(DX11ShaderResourceDependancy::DX11UAVRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11UAVRef) * srd.uavCount
			);
		for (size_t i = 0; i < uavVector.size(); i++)
		{
			ShaderParams& p = shaderInstance.params[uavVector[i].first];
			srd.uavs[i].indexCount = 1;
			srd.uavs[i].slotOrRegister = p.slotIndex;
			srd.uavs[i].indices =
				(uint*)allocs->alloc(sizeof(uint) * srd.uavs[i].indexCount);
			srd.uavs[i].indices[0] = uavVector[i].second;
		}
	}

	// SRV : various types
	{
		const std::vector<InstanceToDependancy::SRVParamIndices>& srvVector =
			itod.srvCurrentIndexVector[shaderIndex];
		srd.srvCount = static_cast<uint>(srvVector.size());
		srd.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * srd.srvCount
			);
		for (size_t i = 0; i < srvVector.size(); i++)
		{
			const InstanceToDependancy::SRVParamIndices& srvIndex = srvVector[i];
			ShaderParams& p = shaderInstance.params[srvIndex.instanceParamIndex];
			srd.srvs[i].indexCount = 1;
			srd.srvs[i].slotOrRegister = p.slotIndex;
			srd.srvs[i].indices =
				(uint*)allocs->alloc(sizeof(uint) * srd.srvs[i].indexCount);

			switch (srvIndex.kind)
			{
			case InstanceToDependancy::SRVParamIndices::SRVArrayKind::Texture2D:
				srd.srvs[i].indices[0] = res->shaderTex2Ds[srvIndex.refArrayIndex].srvIndex;
				break;
			case InstanceToDependancy::SRVParamIndices::SRVArrayKind::ExistSRV:
				srd.srvs[i].indices[0] = srvIndex.refArrayIndex;
				break;
			}
		}
	}
}

HRESULT LoadResourceAndDependancyFromInstance(
	IN ID3D11Device* device, IN const Allocaters* allocs, IN uint instanceCount, IN RenderInstance* instances,
	OUT DX11Resources* res, OUT DX11InternalResourceDescBuffer* rawBuffer, OUT DX11PipelineDependancySet* set
)
{
	std::vector<std::pair<const wchar_t*, FBXAdjChunk>> fbxPathChunkVector;
	std::vector<const wchar_t*> texturePathVector;
	std::vector<ShaderParamSampler> samplerStateVector;
	std::vector<std::pair<uint, ShaderParamCB>> constantBufferVector;
	std::vector<ShaderCompileDesc> shaderCompileVector;
	std::vector<SkinningInstanceDesc> skinningInstanceDescVector;
	std::vector<std::pair<uint, uint>> vertexShaderAndGeometryVector;

	std::vector<DX11PipelineDependancy> dx11FrameDependVector;
	std::vector<DX11PipelineDependancy> dx11ResizeDependVector;
	std::vector<DX11PipelineDependancy> dx11InitDependVector;

	std::vector<InstanceToDependancy> itodVector;

	std::vector<FBXChunkConfig::FBXMeshConfig> fbxMeshConfigVector;

	for (uint instanceIndex = 0; instanceIndex < instanceCount; instanceIndex++)
	{
		InstanceToDependancy itod = InstanceToDependancy();
		RenderInstance& instance = instances[instanceIndex];
		uint accumGeometryCount = 0, accumAnimationCount = 0, resInputLayoutIndex = 0;
		FBXMeshChunk* m = nullptr;
		FBXChunk::FBXAnimation* animPtr = nullptr;

		// instance :: geometry
		{
			auto it = std::find_if(
				fbxPathChunkVector.begin(), 
				fbxPathChunkVector.end(), 
				[=](std::pair<const wchar_t*, FBXAdjChunk> val) -> bool
				{
					return wcscmp(val.first, instance.geometry.filePath) == 0;
				}
			);

			if (it == fbxPathChunkVector.end())
			{
				FBXChunk c;
				memset(&c, 0, sizeof(c));
				FBXLoadOptionChunk opt;
				memset(&opt, 0, sizeof(opt));
				FBXChunkConfig config;
				memset(&config, 0, sizeof(config));

				FALSE_ERROR_MESSAGE_CONTINUE_ARGS(
					ImportFBX(instance.geometry.filePath, c, &opt, allocs),
					L"fail to import FBX(%s) for geometry..",
					instance.geometry.filePath
				);

				size_t prevIndex = fbxMeshConfigVector.size();
				for (uint i = 0; i < c.meshCount; i++)
					fbxMeshConfigVector.push_back(FBXChunkConfig::FBXMeshConfig());
				config.meshConfigs = fbxMeshConfigVector.data() + prevIndex;

				fbxPathChunkVector.push_back(
					PathFBXAdjChunkPair(
						instance.geometry.filePath,
						FBXAdjChunk(std::move(c), config, accumGeometryCount, accumAnimationCount)
					)
				);

				it = std::prev(fbxPathChunkVector.end());

				accumGeometryCount += c.meshCount;
				accumAnimationCount += c.animationCount;
			}

			itod.fbxGeometryChunkIndex = static_cast<int>(std::distance(fbxPathChunkVector.begin(), it));

			for (uint j = 0; j < it->second.chunk.meshCount; j++)
			{
				FBXMeshChunk& mesh = it->second.chunk.meshs[j];
				if (strcmp(mesh.name, instance.geometry.meshName) == 0)
				{
					m = &mesh;
					itod.meshIndex = j;
					break;
				}
			}

			FALSE_ERROR_MESSAGE_CONTINUE_ARGS(
				itod.meshIndex >= 0,
				L"cannot find mesh(%s) from FBX(%s)..", instance.geometry.meshName, instance.geometry.filePath
			);
			it->second.config.meshConfigs[itod.meshIndex].isSkinned |= instance.isSkinDeform;
			itod.resGeometryIndex = it->second.startGeometryCount + itod.meshIndex;

			if (instance.isSkinDeform)
			{
				it = std::find_if(
					fbxPathChunkVector.begin(),
					fbxPathChunkVector.end(),
					[=](std::pair<const wchar_t*, FBXAdjChunk> val) -> bool
					{
						return wcscmp(val.first, instance.anim.filePath) == 0;
					}
				);

				if (it == fbxPathChunkVector.end())
				{
					FBXChunk c;
					memset(&c, 0, sizeof(c));
					FBXLoadOptionChunk opt;
					memset(&opt, 0, sizeof(opt));
					FBXChunkConfig config;
					memset(&config, 0, sizeof(config));

					FALSE_ERROR_MESSAGE_CONTINUE_ARGS(
						ImportFBX(instance.anim.filePath, c, &opt, allocs),
						L"fail to import FBX(%s) for animation..",
						instance.anim.filePath
					);

					fbxPathChunkVector.push_back(
						PathFBXAdjChunkPair(
							instance.geometry.filePath,
							FBXAdjChunk(std::move(c), config, accumGeometryCount, accumAnimationCount)
						)
					);

					it = std::prev(fbxPathChunkVector.end());

					accumGeometryCount += c.meshCount;
					accumAnimationCount += c.animationCount;
				}

				itod.fbxAnimChunkIndex = static_cast<int>(std::distance(fbxPathChunkVector.begin(), it));

				// animation find in FBXChunk
				for (uint j = 0; j < it->second.chunk.animationCount; j++)
				{
					FBXChunk::FBXAnimation& anim = it->second.chunk.animations[j];
					if (strcmp(anim.animationName, instance.anim.animationName) == 0)
					{
						animPtr = &anim;
						itod.animIndex = j;
						break;
					}
				}

				FALSE_ERROR_MESSAGE_CONTINUE_ARGS(
					itod.animIndex >= 0,
					L"cannot find aniamtion(%s) from FBX(%s)..", 
					instance.anim.animationName, instance.anim.filePath
				);
				itod.resAnimationIndex = it->second.startAnimationCount + itod.animIndex;
				itod.skinInstanceIndex = static_cast<int>(skinningInstanceDescVector.size());

				SkinningInstanceDesc sid;
				sid.animationIndex = itod.resAnimationIndex;
				sid.geometryIndex = itod.resGeometryIndex;
				skinningInstanceDescVector.push_back(sid);
			}
		}

		int shaderCount = instance.isSkinDeform ? 6 : 5;
		for (int shaderIndex = 0; shaderIndex < shaderCount; shaderIndex++)
		{
			if (shaderIndex < 5 && !(instance.shaderFlag & (0x01 << shaderIndex))) continue;

			ShaderInstance& shaderInstance = shaderIndex < 5? instance.si[shaderIndex]: instance.skinCSParam;

			// shader
			{
				auto it = std::find_if(
					shaderCompileVector.begin(), shaderCompileVector.end(),
					[=](const ShaderCompileDesc& d) -> bool 
					{
						return shaderInstance.sd == d;
					}
				);

				if (it == shaderCompileVector.end())
				{
					shaderCompileVector.push_back(shaderInstance.sd);
					it = std::prev(shaderCompileVector.end());
				}

				itod.shaderCompileIndices[shaderIndex] = static_cast<int>(std::distance(shaderCompileVector.begin(), it));
			}

			// param 
			for (uint paramIndex = 0; paramIndex < shaderInstance.paramCount; paramIndex++)
			{
				switch (shaderInstance.params[paramIndex].kind)
				{
				case ShaderParamKind::Texture2DSRV:
				{
					auto it = std::find_if(
						texturePathVector.begin(),
						texturePathVector.end(),
						[=](const std::wstring& ws) -> bool {
							return ws.compare(shaderInstance.params[paramIndex].tex2DSRV.filePath) == 0;
						});

					if (it == texturePathVector.end())
					{
						texturePathVector.push_back(
							shaderInstance.params[paramIndex].tex2DSRV.filePath
						);
						it = std::prev(texturePathVector.end());
					}

					itod.srvCurrentIndexVector[shaderIndex].push_back(
						InstanceToDependancy::SRVParamIndices(
							InstanceToDependancy::SRVParamIndices::SRVArrayKind::Texture2D,
							paramIndex, static_cast<uint>(std::distance(texturePathVector.begin(), it))
						)
					);
				}
				break;
				case ShaderParamKind::SamplerState:
				{
					auto it = std::find_if(
						samplerStateVector.begin(),
						samplerStateVector.end(),
						[=](const ShaderParamSampler& sampler) -> bool {
							return sampler == shaderInstance.params[paramIndex].sampler;
						});

					if (it == samplerStateVector.end())
					{
						samplerStateVector.push_back(shaderInstance.params[paramIndex].sampler);
						it = std::prev(samplerStateVector.end());
					}

					itod.samplerCurrentIndexVector[shaderIndex].push_back(
						std::pair<uint, uint>(
							paramIndex, std::distance(samplerStateVector.begin(), it)
						)
					);
				}
				break;
				case ShaderParamKind::ConstantBuffer:
				{
					auto it = std::find_if(
						constantBufferVector.begin(),
						constantBufferVector.end(),
						[=](const std::pair<uint, ShaderParamCB>& cb) -> bool {
							return cb.second == shaderInstance.params[paramIndex].cb;
						});

					if (it == constantBufferVector.end())
					{
						constantBufferVector.push_back(
							std::pair<uint, ShaderParamCB>(
								instanceIndex,
								shaderInstance.params[paramIndex].cb
								)
						);
						it = std::prev(constantBufferVector.end());
					}

					itod.cbCurrentIndexVector[shaderIndex].push_back(
						InstanceToDependancy::CBParamIndices(
							instanceIndex, paramIndex, static_cast<uint>(std::distance(constantBufferVector.begin(), it))
						)
					);
				}
				break;
				}
			}
		}

		FALSE_ERROR_MESSAGE_CONTINUE_ARGS(
			itod.shaderCompileIndices[0] >= 0,
			L"fail to find vertex shader desc.."
		);
		auto it = 
			std::find_if(
				vertexShaderAndGeometryVector.begin(),
				vertexShaderAndGeometryVector.end(),
				[=](std::pair<uint, uint> p) -> bool {
					return p.first == itod.shaderCompileIndices[0] && p.second == itod.resGeometryIndex; 
				}
			);
		if (it == vertexShaderAndGeometryVector.end())
		{
			vertexShaderAndGeometryVector.push_back(
				std::pair<uint, uint>(itod.shaderCompileIndices[0], itod.resGeometryIndex)
			);
			it = std::prev(vertexShaderAndGeometryVector.end());
		}
		itod.resInputLayoutIndex = static_cast<int>(std::distance(vertexShaderAndGeometryVector.begin(), it));

		itodVector.push_back(itod);
	}

#pragma region load resource from instance

	// fbx to geometry
	{
		FBXChunk* chunks = (FBXChunk*)alloca(sizeof(FBXChunk) * fbxPathChunkVector.size());
		FBXChunkConfig* chunkConfigs = (FBXChunkConfig*)alloca(sizeof(FBXChunkConfig) * fbxPathChunkVector.size());
		for (auto ci = fbxPathChunkVector.begin(); ci != fbxPathChunkVector.end(); ci++)
		{
			uint i = static_cast<uint>(std::distance(ci, fbxPathChunkVector.begin()));

			chunks[i] = ci->second.chunk;
			chunkConfigs[i] = ci->second.config;
		}
		FAILED_ERROR_MESSAGE_RETURN_ARGS(
			LoadMeshAndAnimsFromFBXByDX11(res, rawBuffer, allocs, static_cast<uint>(fbxPathChunkVector.size()), chunks, chunkConfigs),
			L"fail to LoadMeshAndAnimsFromFBXByDX11.."
		);
	}

	// texture load
	FAILED_ERROR_MESSAGE_RETURN_ARGS(
		ReserveTex2DAndSRVFromFileByDX11(
			res, rawBuffer, allocs,
			static_cast<uint>(texturePathVector.size()), texturePathVector.data()
		),
		L"fail to LoadMeshAndAnimsFromFBXByDX11.."
	);

	// shader compile
	DX11CompileDescToShader* cdToShader =
		(DX11CompileDescToShader*)alloca(sizeof(DX11CompileDescToShader) * shaderCompileVector.size());
	FAILED_ERROR_MESSAGE_RETURN(
		ReserveShaderFromFileByDX11(
			res, rawBuffer, allocs,
			static_cast<uint>(shaderCompileVector.size()), shaderCompileVector.data(), cdToShader
		),
		L"fail to compile shaders.."
	);

	// input layouts
	{
		DX11InputLayoutDesc* inputLayoutDescs = (DX11InputLayoutDesc*)alloca(
			sizeof(DX11InputLayoutDesc) * vertexShaderAndGeometryVector.size()
		);
		for (uint i = 0; i < vertexShaderAndGeometryVector.size(); i++)
		{
			inputLayoutDescs[i].shaderCompileDescIndex = vertexShaderAndGeometryVector[i].first;
			inputLayoutDescs[i].layoutChunkIndex = res->geometryChunks[vertexShaderAndGeometryVector[i].second].vertexLayoutIndex;
		}
		FAILED_ERROR_MESSAGE_RETURN(
			ReserveLoadInputLayoutRefIndex(
				res, rawBuffer, allocs,
				static_cast<uint>(shaderCompileVector.size()), cdToShader,
				static_cast<uint>(vertexShaderAndGeometryVector.size()), inputLayoutDescs
			),
			L"fail to create dx11 input layout.."
		);
	}

	// skinning instance load
	FAILED_ERROR_MESSAGE_RETURN(
		ReserveSkinningInstances(
			res, rawBuffer, allocs, 
			static_cast<uint>(skinningInstanceDescVector.size()), skinningInstanceDescVector.data()
		),
		L"fail to create dx11 input layout.."
	);

	// constant buffer
	{
		ALLOC_RANGE_ZEROMEM(
			res->constantBufferCount, constantBufferVector.size(),
			uint, res->constantBufferIndices, allocs->alloc
		);
		uint* cbSizes = (uint*)alloca(sizeof(uint) * constantBufferVector.size());
		for (uint i = 0; i < constantBufferVector.size(); i++)
			cbSizes[i] = constantBufferVector[i].second.size;
		uint count = ReserveLoadConstantBuffers(rawBuffer, static_cast<uint>(constantBufferVector.size()), cbSizes);
		for (uint i = 0; i < res->constantBufferCount; i++)
			res->constantBufferIndices[i] = count + i;
	}

	// sampler states
	{
		D3D11_SAMPLER_DESC* sds = (D3D11_SAMPLER_DESC*)alloca(
			sizeof(D3D11_SAMPLER_DESC) * samplerStateVector.size()
		);
		for (uint i = 0; i < samplerStateVector.size(); i++)
		{
			D3D11_SAMPLER_DESC d;
			GetDX11Sampler(samplerStateVector[i], d);
			sds[i] = d;
		}
		ReserveLoadSamplerStates(rawBuffer, static_cast<uint>(samplerStateVector.size()), sds);
	}

	FAILED_ERROR_MESSAGE_RETURN(
		CreateDX11ResourcesByDesc(res, rawBuffer, allocs, device, true),
		L"fail to create dx11 samplers.."
	);
#pragma endregion

#pragma region link instance to dependancy

	for (uint instanceIndex = 0; instanceIndex < instanceCount; instanceIndex++)
	{
		InstanceToDependancy& itod = itodVector[instanceIndex];
		RenderInstance& instance = instances[instanceIndex];

		DX11Resources::SkinningInstance* skin = 
			instance.isSkinDeform ? &res->skinningInstances[itod.skinInstanceIndex] : nullptr;

		for (uint sidx = 0; sidx < 6; sidx++)
		{
			if (sidx < 5 && !(instance.shaderFlag & (0x01 << sidx))) continue;
			const ShaderInstance& si = sidx < 5 ? instance.si[sidx] : instance.skinCSParam;

			for (uint paramIndex = 0; paramIndex < si.paramCount; paramIndex++)
			{
				const ShaderParams& p = si.params[paramIndex];

				switch (p.kind)
				{
				case ShaderParamKind::ExistBufferSRV:
				{
					int srvIndex = -1;
					switch (p.existSRV.kind)
					{
					case ExistSRVKind::GeometryVertexBufferForSkin:
						srvIndex = res->geometryChunks[skin->geometryIndex].vertexDataSRVIndex;
						break;
					case ExistSRVKind::BindPoseBufferForSkin:
						srvIndex = 
							res->boneSets[res->anims[skin->animationIndex].boneSetIndex].binePoseTransformSRVIndex;
						break;
					case ExistSRVKind::AnimationBufferForSkin:
						srvIndex = res->anims[skin->animationIndex].animPoseTransformSRVIndex;
						break;
					case ExistSRVKind::DeformedVertexBufferForSkin:
						srvIndex = skin->vertexStreamSRVIndex;
						break;
					}

					itod.srvCurrentIndexVector[sidx].push_back(
						InstanceToDependancy::SRVParamIndices(
							InstanceToDependancy::SRVParamIndices::SRVArrayKind::ExistSRV,
							paramIndex, srvIndex
						)
					);
				}
					break;
				case ShaderParamKind::ExistBufferUAV:
					switch (p.existUAV.kind)
					{
					case ExistUAVKind::DeformedVertexBufferForSkin:
						itod.uavCurrentIndexVector[sidx].push_back(
							std::pair<uint, uint>(paramIndex, skin->vertexStreamUAVIndex)
						);
						break;
					}
					break;
				}
			}
		}
	}

	std::vector<ShaderParamCB> cbVector;
	for (uint instanceIndex = 0; instanceIndex < instanceCount; instanceIndex++)
	{
		// variables
		InstanceToDependancy& itod = itodVector[instanceIndex];
		RenderInstance& instance = instances[instanceIndex];
		DX11Resources::GeometryChunk& geometryChunk = res->geometryChunks[itod.resGeometryIndex];
		DX11Resources::DX11LayoutChunk& layoutChunk = res->vertexLayouts[geometryChunk.vertexLayoutIndex];

		for (uint shaderIndex = 0; shaderIndex < 6; shaderIndex++)
		for (uint cbIndex = 0; cbIndex < itod.cbCurrentIndexVector[shaderIndex].size(); cbIndex++)
		{
			// TODO:: prune duplicate update
			const uint dstSubres = 0, srcRowPitch = 0, srcDepthPitch = 0;
			const std::vector<InstanceToDependancy::CBParamIndices>& cbCurrentIndexVector =
				itod.cbCurrentIndexVector[shaderIndex];
			const InstanceToDependancy::CBParamIndices& indices = cbCurrentIndexVector[cbIndex];
			ShaderParamCB& cb = constantBufferVector[indices.cbIndex].second;

			DX11PipelineDependancy cbDepend = DX11PipelineDependancy(CopyKind::UpdateSubResource);

			cbDepend.copy.args.updateSubRes.resKind = ResourceKind::Buffer;
			cbDepend.copy.args.updateSubRes.resIndex = res->constantBufferIndices[indices.cbIndex];
			cbDepend.copy.args.updateSubRes.dstSubres = dstSubres;
			cbDepend.copy.args.updateSubRes.getBoxFunc;
			cbDepend.copy.args.updateSubRes.srcRowPitch = srcRowPitch;
			cbDepend.copy.args.updateSubRes.srcDepthPitch = srcDepthPitch;

			cbDepend.copy.args.updateSubRes.dataBufferSize = cb.size;
			switch (cb.existParam)
			{
			case ShaderParamCB::ExistParam::None:
				cbDepend.copy.args.updateSubRes.param = cb.param;
				break;
			case ShaderParamCB::ExistParam::SkinningInstanceIndex:
				cbDepend.copy.args.updateSubRes.param =
					IntToPtr(itodVector[instanceIndex].skinInstanceIndex);
				break;
			}
			cbDepend.copy.args.updateSubRes.copyToBufferFunc = cb.setFunc;
			std::vector<DX11PipelineDependancy>* dep = nullptr;
			switch (cb.freq)
			{
			case UpdateFrequency::PerFrame:
				dep = &dx11FrameDependVector;
				break;
			case UpdateFrequency::OnlyOnce:
				dep = &dx11InitDependVector;
				break;
			case UpdateFrequency::OnResize:
				dep = &dx11ResizeDependVector;
				break;
			}

			if (!cb.unique)
			{
				auto it = std::find_if(
					cbVector.begin(), cbVector.end(),
					[=](const ShaderParamCB& c) -> bool
					{ 
						return wcscmp(cb.name, c.name) == 0;
					}
				);
				if (it != cbVector.end())
					continue;
				cbVector.push_back(cb);
			}

			dep->push_back(cbDepend);
		}

		// only zero values
		const sint baseVertexLocation = 0;
		const uint vertexBufferOffset = 0, startIndexLocation = 0;

		if (instance.isSkinDeform)
		{
			DX11PipelineDependancy computeDepend;
			computeDepend.pipelineKind = PipelineKind::Compute;
			DX11ComputePipelineDependancy& compute = computeDepend.compute;
			compute.dispatchType = DX11ComputePipelineDependancy::DispatchType::Dispatch;
			compute.argsAsDispatch.dispatch.threadGroupCountX = geometryChunk.vertexCount;
			compute.argsAsDispatch.dispatch.threadGroupCountY = 1;
			compute.argsAsDispatch.dispatch.threadGroupCountZ = 1;

			SetShaderResourceDependancy(
				res, allocs, itod.shaderCompileIndices[5], cdToShader, itod, 5, 
				instance.skinCSParam, compute.resources
			);

			dx11FrameDependVector.push_back(computeDepend);
		}

		{
			DX11PipelineDependancy draw;
			draw.pipelineKind = PipelineKind::Draw;

			for (uint shaderIndex = 0; shaderIndex < 5; shaderIndex++)
			{
				new (draw.draw.dependants + shaderIndex) DX11ShaderResourceDependancy();

				if (instance.shaderFlag & (0x01 << shaderIndex))
				{
					SetShaderResourceDependancy(
						res, allocs,
						itod.shaderCompileIndices[shaderIndex], cdToShader, itod, shaderIndex,
						instance.si[shaderIndex], draw.draw.dependants[shaderIndex]
					);
				}
			}

			draw.draw.input.geometryIndex = itod.resGeometryIndex;
			draw.draw.input.inputLayoutIndex = itod.resInputLayoutIndex;
			draw.draw.input.topology = D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			draw.draw.input.vertexBufferOffset = vertexBufferOffset;
			draw.draw.input.vertexSize = layoutChunk.vertexSize;
			if (instance.isSkinDeform && itod.skinInstanceIndex >= 0)
				draw.draw.input.vertexBufferIndex = UINT_MAX;
			else
				draw.draw.input.vertexBufferIndex = geometryChunk.vertexBufferIndex;

			draw.draw.drawType = DX11DrawPipelineDependancy::DrawType::DrawIndexed;
			draw.draw.argsAsDraw.drawIndexedArgs.indexCount = geometryChunk.indexCount;
			draw.draw.argsAsDraw.drawIndexedArgs.baseVertexLocation = baseVertexLocation;
			draw.draw.argsAsDraw.drawIndexedArgs.startIndexLocation = startIndexLocation;

			dx11FrameDependVector.push_back(draw);
		}
	}


	set->frameDependancyCount = static_cast<uint>(dx11FrameDependVector.size());
	set->frameDependancy = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * dx11FrameDependVector.size());
	for (size_t i = 0; i < dx11FrameDependVector.size(); i++)
		new (set->frameDependancy + i) DX11PipelineDependancy(std::move(dx11FrameDependVector[i]));

	set->resizeDependancyCount = static_cast<uint>(dx11ResizeDependVector.size());
	set->resizeDependancy = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * dx11ResizeDependVector.size());
	for (size_t i = 0; i < dx11ResizeDependVector.size(); i++)
		new (set->resizeDependancy + i) DX11PipelineDependancy(std::move(dx11ResizeDependVector[i]));

	set->initDependancyCount = static_cast<uint>(dx11InitDependVector.size());
	set->initDependancy = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * dx11InitDependVector.size());
	for (size_t i = 0; i < dx11InitDependVector.size(); i++)
		new (set->initDependancy + i) DX11PipelineDependancy(std::move(dx11InitDependVector[i]));

#pragma endregion

	return S_OK;
}

