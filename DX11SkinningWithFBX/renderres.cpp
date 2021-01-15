#include <array>
#include <map>

#include "dx11depend.h"
#include "renderres.h"
#include "fbximport.h"

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
	const RenderResources* res, const Allocaters* allocs, 
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
		srd.constantBufferCount = cbVector.size();
		srd.constantBuffers =
			(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * srd.constantBufferCount
			);
		for (int i = 0; i < cbVector.size(); i++)
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
		srd.samplerCount = samplerVector.size();
		srd.samplers =
			(DX11ShaderResourceDependancy::DX11SamplerRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * srd.samplerCount
			);
		for (int i = 0; i < samplerVector.size(); i++)
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
		srd.uavCount = uavVector.size();
		srd.uavs =
			(DX11ShaderResourceDependancy::DX11UAVRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11UAVRef) * srd.uavCount
			);
		for (int i = 0; i < uavVector.size(); i++)
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
		srd.srvCount = srvVector.size();
		srd.srvs =
			(DX11ShaderResourceDependancy::DX11SRVRef*)
			allocs->alloc(
				sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * srd.srvCount
			);
		for (int i = 0; i < srvVector.size(); i++)
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
	OUT RenderResources* res, OUT DX11InternalResourceDescBuffer* rawBuffer, OUT DX11PipelineDependancySet* set
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

			itod.fbxGeometryChunkIndex = std::distance(fbxPathChunkVector.begin(), it);

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

				itod.fbxAnimChunkIndex = std::distance(fbxPathChunkVector.begin(), it);

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
				itod.skinInstanceIndex = skinningInstanceDescVector.size();

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

				itod.shaderCompileIndices[shaderIndex] = std::distance(shaderCompileVector.begin(), it);
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
							paramIndex, std::distance(texturePathVector.begin(), it)
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
							instanceIndex, paramIndex, std::distance(constantBufferVector.begin(), it)
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
		itod.resInputLayoutIndex = vertexShaderAndGeometryVector.size();
		vertexShaderAndGeometryVector.push_back(
			std::pair<uint, uint> (itod.shaderCompileIndices[0], itod.resGeometryIndex)
		);

		itodVector.push_back(itod);
	}

#pragma region load resource from instance

	// fbx to geometry
	{
		FBXChunk* chunks = (FBXChunk*)alloca(sizeof(FBXChunk) * fbxPathChunkVector.size());
		FBXChunkConfig* chunkConfigs = (FBXChunkConfig*)alloca(sizeof(FBXChunkConfig) * fbxPathChunkVector.size());
		for (auto ci = fbxPathChunkVector.begin(); ci != fbxPathChunkVector.end(); ci++)
		{
			uint i = std::distance(ci, fbxPathChunkVector.begin());

			chunks[i] = ci->second.chunk;
			chunkConfigs[i] = ci->second.config;
		}
		FAILED_ERROR_MESSAGE_RETURN_ARGS(
			LoadMeshAndAnimsFromFBXByDX11(res, rawBuffer, allocs, fbxPathChunkVector.size(), chunks, chunkConfigs),
			L"fail to LoadMeshAndAnimsFromFBXByDX11.."
		);
	}

	// texture load
	FAILED_ERROR_MESSAGE_RETURN_ARGS(
		ReserveTex2DAndSRVFromFileByDX11(
			res, rawBuffer, allocs,
			texturePathVector.size(), texturePathVector.data()
		),
		L"fail to LoadMeshAndAnimsFromFBXByDX11.."
	);

	// shader compile
	DX11CompileDescToShader* cdToShader =
		(DX11CompileDescToShader*)alloca(sizeof(DX11CompileDescToShader) * shaderCompileVector.size());
	FAILED_ERROR_MESSAGE_RETURN(
		ReserveShaderFromFileByDX11(
			res, rawBuffer, allocs,
			shaderCompileVector.size(), shaderCompileVector.data(), cdToShader
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
				&res->dx11, rawBuffer, allocs,
				shaderCompileVector.size(), cdToShader,
				vertexShaderAndGeometryVector.size(), inputLayoutDescs
			),
			L"fail to create dx11 input layout.."
		);
	}

	// skinning instance load
	FAILED_ERROR_MESSAGE_RETURN(
		ReserveSkinningInstances(
			res, rawBuffer, allocs, skinningInstanceDescVector.size(), skinningInstanceDescVector.data()
		),
		L"fail to create dx11 input layout.."
	);

	// constant buffer
	{
		ALLOC_RANGE_ZEROMEM(
			res->dx11.constantBufferCount, constantBufferVector.size(),
			uint, res->dx11.constantBufferIndices, allocs->alloc
		);
		uint* cbSizes = (uint*)alloca(sizeof(uint) * constantBufferVector.size());
		for (uint i = 0; i < constantBufferVector.size(); i++)
			cbSizes[i] = constantBufferVector[i].second.size;
		uint count = ReserveLoadConstantBuffers(rawBuffer, constantBufferVector.size(), cbSizes);
		for (uint i = 0; i < res->dx11.constantBufferCount; i++)
			res->dx11.constantBufferIndices[i] = count + i;
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
		ReserveLoadSamplerStates(rawBuffer, samplerStateVector.size(), sds);
	}

	FAILED_ERROR_MESSAGE_RETURN(
		CreateDX11ResourcesByDesc(&res->dx11, rawBuffer, allocs, device, true),
		L"fail to create dx11 samplers.."
	);
#pragma endregion

#pragma region link instance to dependancy

	for (uint instanceIndex = 0; instanceIndex < instanceCount; instanceIndex++)
	{
		// variables
		InstanceToDependancy& itod = itodVector[instanceIndex];
		RenderInstance& instance = instances[instanceIndex];
		RenderResources::GeometryChunk& geometryChunk = res->geometryChunks[itod.resGeometryIndex];
		DX11Resources::DX11LayoutChunk& layoutChunk = res->dx11.vertexLayouts[geometryChunk.vertexLayoutIndex];

		for (uint shaderIndex = 0; shaderIndex < 6; shaderIndex++)
		for (uint cbIndex = 0; cbIndex < itod.cbCurrentIndexVector[shaderIndex].size(); cbIndex++)
		{
			// TODO:: prune duplicate update
			const uint dstSubres = 0, srcRowPitch = 0, srcDepthPitch = 0;
			InstanceToDependancy::CBParamIndices& indices = itod.cbCurrentIndexVector[shaderIndex][cbIndex];
			ShaderParamCB& cb = constantBufferVector[indices.cbIndex].second;

			DX11PipelineDependancy cbDepend = DX11PipelineDependancy(CopyKind::UpdateSubResource);

			cbDepend.copy.args.updateSubRes.resKind = ResourceKind::Buffer;
			cbDepend.copy.args.updateSubRes.resIndex = res->dx11.constantBufferIndices[indices.cbIndex];
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

			switch (cb.freq)
			{
			case UpdateFrequency::PerFrame:
				dx11FrameDependVector.push_back(cbDepend);
				break;
			case UpdateFrequency::OnlyOnce:
				dx11InitDependVector.push_back(cbDepend);
				break;
			case UpdateFrequency::OnResize:
				dx11ResizeDependVector.push_back(cbDepend);
				break;
			}
		}

		// only zero values
		const sint baseVertexLocation = 0;
		const uint vertexBufferOffset = 0, startIndexLocation = 0;

		if (instance.isSkinDeform)
		{
			RenderResources::SkinningInstance& skin = res->skinningInstances[itod.skinInstanceIndex];

			DX11PipelineDependancy computeDepend;
			computeDepend.pipelineKind = PipelineKind::Compute;
			DX11ComputePipelineDependancy& compute = computeDepend.compute;
			compute.dispatchType = DX11ComputePipelineDependancy::DispatchType::Dispatch;
			compute.argsAsDispatch.dispatch.threadGroupCountX = geometryChunk.vertexCount;
			compute.argsAsDispatch.dispatch.threadGroupCountY = 1;
			compute.argsAsDispatch.dispatch.threadGroupCountZ = 1;

			for (uint i = 0; i < instance.skinCSParam.paramCount; i++)
			{
				ShaderParams& p = instance.skinCSParam.params[i];

				switch (p.kind)
				{
				case ShaderParamKind::ExistBufferSRV:
					switch (p.existSRV.kind)
					{
					case ExistSRVKind::GeometryVertexBufferForSkin:
						itod.srvCurrentIndexVector[5].push_back(
							InstanceToDependancy::SRVParamIndices(
								InstanceToDependancy::SRVParamIndices::SRVArrayKind::ExistSRV,
								i, res->geometryChunks[skin.geometryIndex].vertexDataSRVIndex
							)
						);
						break;
					case ExistSRVKind::BindPoseBufferForSkin:
						itod.srvCurrentIndexVector[5].push_back(
							InstanceToDependancy::SRVParamIndices(
								InstanceToDependancy::SRVParamIndices::SRVArrayKind::ExistSRV,
								i, res->boneSets[
									res->anims[skin.animationIndex].boneSetIndex
								].binePoseTransformSRVIndex
							)
						);
						break;
					case ExistSRVKind::AnimationBufferForSkin:
						itod.srvCurrentIndexVector[5].push_back(
							InstanceToDependancy::SRVParamIndices(
								InstanceToDependancy::SRVParamIndices::SRVArrayKind::ExistSRV,
								i, res->anims[skin.animationIndex].animPoseTransformSRVIndex
							)
						);
						break;
					}
					break;
				case ShaderParamKind::ExistBufferUAV:
					switch (p.existUAV.kind)
					{
					case ExistUAVKind::DeformedVertexBufferForSkin:
						itod.uavCurrentIndexVector[5].push_back(std::pair<uint, uint>(i, skin.vertexStreamUAVIndex));
						break;
					}
					break;
				}
			}

			SetShaderResourceDependancy(
				res, allocs, itod.shaderCompileIndices[5], cdToShader, itod, 5, 
				instance.skinCSParam, compute.resources
			);

			dx11FrameDependVector.push_back(computeDepend);

			DX11PipelineDependancy vtxCopy;
			vtxCopy.pipelineKind = PipelineKind::Copy;
			DX11CopyDependancy& copy = vtxCopy.copy;
			copy.kind = CopyKind::CopyResource;
			copy.args.copyRes.srcBufferIndex = skin.vertexStreamBufferIndex;
			copy.args.copyRes.dstBufferIndex = skin.vertexBufferIndex;

			dx11FrameDependVector.push_back(vtxCopy);
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
			{
				RenderResources::SkinningInstance& skinInstance =
					res->skinningInstances[itod.skinInstanceIndex];
				draw.draw.input.vertexBufferIndex = skinInstance.vertexBufferIndex;
			}
			else
				draw.draw.input.vertexBufferIndex = geometryChunk.vertexBufferIndex;

			draw.draw.drawType = DX11DrawPipelineDependancy::DrawType::DrawIndexed;
			draw.draw.argsAsDraw.drawIndexedArgs.indexCount = geometryChunk.indexCount;
			draw.draw.argsAsDraw.drawIndexedArgs.baseVertexLocation = baseVertexLocation;
			draw.draw.argsAsDraw.drawIndexedArgs.startIndexLocation = startIndexLocation;

			dx11FrameDependVector.push_back(draw);
		}
	}


	set->frameDependancyCount = static_cast<size_t>(dx11FrameDependVector.size());
	set->frameDependancy = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * dx11FrameDependVector.size());
	for (size_t i = 0; i < dx11FrameDependVector.size(); i++)
		new (set->frameDependancy + i) DX11PipelineDependancy(std::move(dx11FrameDependVector[i]));

	set->resizeDependancyCount = static_cast<size_t>(dx11ResizeDependVector.size());
	set->resizeDependancy = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * dx11ResizeDependVector.size());
	for (size_t i = 0; i < dx11ResizeDependVector.size(); i++)
		new (set->resizeDependancy + i) DX11PipelineDependancy(std::move(dx11ResizeDependVector[i]));

	set->initDependancyCount = static_cast<size_t>(dx11InitDependVector.size());
	set->initDependancy = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * dx11InitDependVector.size());
	for (size_t i = 0; i < dx11InitDependVector.size(); i++)
		new (set->initDependancy + i) DX11PipelineDependancy(std::move(dx11InitDependVector[i]));

#pragma endregion

	return S_OK;
}

HRESULT ReleaseResources(RenderResources* res, const Allocaters* allocs)
{
	SAFE_DEALLOC(res->geometryChunks, allocs->dealloc);
	SAFE_DEALLOC(res->shaderTex2Ds, allocs->dealloc);

	if (res->anims)
	{
		for (uint i = 0; i < res->animCount; i++)
			SAFE_DEALLOC(res->anims[i].animName, allocs->dealloc);
		allocs->dealloc(res->anims);
	}

	if (res->boneSets)
	{
		for (uint i = 0; i < res->boneSetCount; i++)
		{
			if (res->boneSets[i].bones)
			{
				for (uint j = 0; j < res->boneSets[i].boneCount; j++)
					SAFE_DEALLOC(res->boneSets[i].bones[j].name, allocs->dealloc);

				allocs->dealloc(res->boneSets[i].bones);
			}
		}

		allocs->dealloc(res->boneSets);
		res->boneSets = nullptr;
	}

	if (res->shaderFiles)
	{
		for (uint i = 0; i < res->shaderFileCount; i++)
		{
			RenderResources::ShaderFile* file = res->shaderFiles + i;
			for (uint j = 0; j < 6; j++)
				SAFE_DEALLOC(file->shaderIndices[j].indices, allocs->dealloc);
		}

		allocs->dealloc(res->shaderFiles);
	}

	ReleaseResources(&res->dx11, allocs);

	return S_OK;
}


HRESULT LoadDX11Resoureces(RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, DX11ResourceDesc* desc, const Allocaters* allocs, ID3D11Device* device)
{
	HRESULT hr = S_OK;

	hr = LoadMeshAndAnimsFromFBXByDX11(res, rawBuffer, allocs, desc->fbxChunkCount, desc->fbxChunks, desc->fbxMeshConfigs);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to load geometries by FBX..");

	hr = ReserveTex2DAndSRVFromFileByDX11(
		res, rawBuffer, allocs, desc->textureDirCount, desc->texturedirs
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to load textures..");

	DX11CompileDescToShader* dtosBuffer = (DX11CompileDescToShader*)alloca(sizeof(DX11CompileDescToShader) * desc->shaderCompileCount);;

	hr = ReserveShaderFromFileByDX11(
		res, rawBuffer, allocs,
		desc->shaderCompileCount, desc->shaderCompileDescs, dtosBuffer
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to compile shaders..");

	hr = ReserveSkinningInstances(
		res, rawBuffer, allocs, desc->skinningInstanceCount, desc->skinningInstances
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create dx11 input layout..");

	hr = ReserveLoadInputLayoutRefIndex(
		&res->dx11, rawBuffer, allocs,
		desc->shaderCompileCount, dtosBuffer, desc->inputLayoutCount, desc->inputLayoutDescs
	);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create dx11 input layout..");

	ALLOC_RANGE_ZEROMEM(
		res->dx11.constantBufferCount, desc->constantBufferCount,
		uint, res->dx11.constantBufferIndices, allocs->alloc
	);
	uint count = ReserveLoadConstantBuffers(rawBuffer, desc->constantBufferCount, desc->constantBufferSizes);
	for (uint i = 0; i < desc->constantBufferCount; i++)
		res->dx11.constantBufferIndices[i] = count + i;

	count = ReserveLoadSamplerStates(rawBuffer, desc->samplerCount, desc->samplerDescs);

	FAILED_ERROR_MESSAGE_RETURN(
		CreateDX11ResourcesByDesc(&res->dx11, rawBuffer, allocs, device, true),
		L"fail to create dx11 samplers.."
	);

	return hr;
}

HRESULT ReleaseResources(DX11Resources* res, const Allocaters* allocs)
{
	SAFE_DEALLOC(res->constantBufferIndices, allocs->dealloc);
	SAFE_DEALLOC(res->inputLayouts, allocs->dealloc);

	if (res->vertexLayouts)
	{
		for (uint i = 0; i < res->vertexLayoutCount; i++)
			SAFE_DEALLOC(res->vertexLayouts[i].descs, allocs->dealloc);

		allocs->dealloc(res->vertexLayouts);
		res->vertexLayouts = nullptr;
	}

	if (res->samplerStates)
	{
		for (uint i = 0; i < res->samplerCount; i++)
			SAFE_RELEASE(res->samplerStates[i]);

		allocs->dealloc(res->samplerStates);
		res->samplerStates = nullptr;
	}

	if (res->texture2Ds)
	{
		for (uint i = 0; i < res->texture2DCount; i++)
			SAFE_RELEASE(res->texture2Ds[i]);

		allocs->dealloc(res->texture2Ds);
		res->texture2Ds = nullptr;
	}

	if (res->buffers)
	{
		for (uint i = 0; i < res->bufferCount; i++)
			SAFE_RELEASE(res->buffers[i]);

		allocs->dealloc(res->buffers);
		res->buffers = nullptr;
	}

	if (res->srvs)
	{
		for (uint i = 0; i < res->srvCount; i++)
			SAFE_RELEASE(res->srvs[i]);

		allocs->dealloc(res->srvs);
		res->srvs = nullptr;
	}

	if (res->uavs)
	{
		for (uint i = 0; i < res->uavCount; i++)
			SAFE_RELEASE(res->uavs[i]);

		allocs->dealloc(res->uavs);
		res->uavs = nullptr;
	}

	for (uint i = 0; i < 6; i++)
	{
		for (uint j = 0; j < res->shadersByKind[i].shaderCount; j++)
		{
			SAFE_RELEASE(res->shadersByKind[i].shaders[j].shaderBlob);
			SAFE_RELEASE(res->shadersByKind[i].shaders[j].vs);
		}
		SAFE_DEALLOC(res->shadersByKind[i].shaders, allocs->dealloc);
	}

	return S_OK;
}


const char g_PositionSemanticName[] = "POSITION";
const char g_NormalSemanticName[] = "NORMAL";
const char g_TangentSemanticName[] = "TANGENT";
const char g_BinormalSemanticName[] = "BINORMAL";
const char g_UVSemanticName[] = "TEXCOORD";
const char g_BoneIndicesSemanticName[] = "BONEIDNCIES";
const char g_BoneWeightsSemanticName[] = "BONEWEIGHTS";

int BitSizeOfFormatElement(DXGI_FORMAT format);
int ByteSizeOfFormatElement(DXGI_FORMAT format);
bool EqualInputElementDesc(int descCount, D3D11_INPUT_ELEMENT_DESC* descArray0, D3D11_INPUT_ELEMENT_DESC* descArray1);

void SetDX11InputDescWithChunk(bool includeBone, int* descCounts, int* vertexSize/*[2]*/, D3D11_INPUT_ELEMENT_DESC* descBuffer, const FBXMeshChunk::FBXGeometryChunk* c)
{
	uint realAlignment = 0, vertexDataSize = 0;
	vertexSize[0] = 0;
	vertexSize[1] = 0;
	descCounts[0] = 0;

	descBuffer[descCounts[0]].SemanticName = g_PositionSemanticName;
	descBuffer[descCounts[0]].SemanticIndex = 0;
	descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	descBuffer[descCounts[0]].InputSlot = 0;
	descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
	descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	descBuffer[descCounts[0]].InstanceDataStepRate = 0;

	realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
	descCounts[0]++;

	if (c->normals)
	{
		descBuffer[descCounts[0]].SemanticName = g_NormalSemanticName;
		descBuffer[descCounts[0]].SemanticIndex = 0;
		descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descBuffer[descCounts[0]].InputSlot = 0;
		descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
		descCounts[0]++;
	}
	if (c->tangents)
	{
		descBuffer[descCounts[0]].SemanticName = g_TangentSemanticName;
		descBuffer[descCounts[0]].SemanticIndex = 0;
		descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descBuffer[descCounts[0]].InputSlot = 0;
		descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
		descCounts[0]++;
	}
	if (c->binormals)
	{
		descBuffer[descCounts[0]].SemanticName = g_BinormalSemanticName;
		descBuffer[descCounts[0]].SemanticIndex = 0;
		descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descBuffer[descCounts[0]].InputSlot = 0;
		descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
		descCounts[0]++;
	}
	for (uint uvi = 0; uvi < c->uvSlotCount; uvi++)
	{
		if (c->uvSlots[uvi])
		{
			descBuffer[descCounts[0]].SemanticName = g_UVSemanticName;
			descBuffer[descCounts[0]].SemanticIndex = uvi;
			descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32_FLOAT;
			descBuffer[descCounts[0]].InputSlot = 0;
			descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
			descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			descBuffer[descCounts[0]].InstanceDataStepRate = 0;

			realAlignment += ByteSizeOfFormatElement(descBuffer[descCounts[0]].Format);
			descCounts[0]++;
		}
	}
	if (includeBone && (c->boneIndices && c->boneWeights))
	{
		//descBuffer[descCounts[0]].SemanticName = g_BoneIndicesSemanticName;
		//descBuffer[descCounts[0]].SemanticIndex = 0;
		//descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32A32_UINT;
		//descBuffer[descCounts[0]].InputSlot = 0;
		//descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		//descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		//descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		vertexDataSize += ByteSizeOfFormatElement(DXGI_FORMAT_R32G32B32A32_UINT);
		//descCounts[0]++;

		//descBuffer[descCounts[0]].SemanticName = g_BoneWeightsSemanticName;
		//descBuffer[descCounts[0]].SemanticIndex = 0;
		//descBuffer[descCounts[0]].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		//descBuffer[descCounts[0]].InputSlot = 0;
		//descBuffer[descCounts[0]].AlignedByteOffset = realAlignment;
		//descBuffer[descCounts[0]].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		//descBuffer[descCounts[0]].InstanceDataStepRate = 0;

		vertexDataSize += ByteSizeOfFormatElement(DXGI_FORMAT_R32G32B32A32_FLOAT);
		//descCounts[0]++;
	}

	vertexSize[0] = realAlignment + vertexDataSize;
	vertexSize[1] = realAlignment; 
}

int FindEqualDescIndex(uint descCount, D3D11_INPUT_ELEMENT_DESC* descBuffer, uint vertexLayoutBufferCount, DX11Resources::DX11LayoutChunk* vertexLayoutBuffer, DX11Resources* res)
{
	for (int vli = 0; vli < (int)res->vertexLayoutCount; vli++)
		if (
			res->vertexLayouts[vli].descCount == descCount &&
			EqualInputElementDesc(descCount, descBuffer, res->vertexLayouts[vli].descs)
			)
			return vli;


	for (int vli = 0; vli < (int)vertexLayoutBufferCount; vli++)
		if (
			vertexLayoutBuffer[vli].descCount == descCount &&
			EqualInputElementDesc(descCount, descBuffer, vertexLayoutBuffer[vli].descs)
			)
			return vli + res->vertexLayoutCount;

	return -1;
}

HRESULT LoadMeshAndAnimsFromFBXByDX11(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint chunkCount, const FBXChunk* chunks, const FBXChunkConfig* configs
)
{
	int startLayoutCount = res->dx11.vertexLayoutCount,
		newGeometryCount = 0, newBufferCount = 0, newSRVUAVCount = 0;
	for (uint ci = 0; ci < chunkCount; ci++)
		for (uint mi = 0; mi < chunks[ci].meshCount; mi++)
		{
			if (configs[ci].meshConfigs[mi].isSkinned)
			{
				newBufferCount += 3;
				newSRVUAVCount++;
			}
			else
				newBufferCount += 2;

			newGeometryCount++;
		}

	ALLOC_RANGE_ZEROMEM(
		res->geometryCount, newGeometryCount,
		RenderResources::GeometryChunk, res->geometryChunks, allocs->alloc
	);

	int vertexLayoutBufferCount = 0;
	DX11Resources::DX11LayoutChunk* vertexLayoutBuffer =
		(DX11Resources::DX11LayoutChunk*)alloca(
			sizeof(DX11Resources::DX11LayoutChunk) * res->geometryCount
		);

	const int descBufferCapacity = 32;
	D3D11_INPUT_ELEMENT_DESC* descBuffer = (D3D11_INPUT_ELEMENT_DESC*)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity);
	memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity);

	uint totalAnimationCount = 0, totalBoneSetCount = 0;
	for (uint ci = 0; ci < chunkCount; ci++)
	{
		totalAnimationCount += chunks[ci].animationCount;
		totalBoneSetCount++;
	}

	ALLOC_RANGE_ZEROMEM(
		res->boneSetCapacity, totalBoneSetCount,
		RenderResources::BoneSet, res->boneSets, allocs->alloc
	);
	ALLOC_RANGE_ZEROMEM(
		res->animCount, totalAnimationCount,
		RenderResources::Animation, res->anims, allocs->alloc
	);

	for (
		uint ci = 0, geometryOffset = 0, bufferOffset = 0, uavOffset = 0, srvOffset = 0, animOffset = 0; 
		ci < chunkCount;
		ci++, memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity)
		)
	{
		const FBXChunk& c = chunks[ci];

#pragma region load meshes from FBX
		for (uint mi = 0; mi < c.meshCount; mi++, memset(descBuffer, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * descBufferCapacity))
		{
			FBXChunkConfig::FBXMeshConfig& mc = configs[ci].meshConfigs[mi];
			FBXMeshChunk& m = c.meshs[mi];
			RenderResources::GeometryChunk& g = res->geometryChunks[geometryOffset];
			g.bound = m.geometry.bound;

			// vertexlayout record start 

			int descCount, vertexSizes[2];
			SetDX11InputDescWithChunk(mc.isSkinned, &descCount, vertexSizes, descBuffer, &m.geometry);
			int vertexSize = mc.isSkinned ? vertexSizes[1] : vertexSizes[0];

			int findVertexLayoutIndex = FindEqualDescIndex(descCount, descBuffer, vertexLayoutBufferCount, vertexLayoutBuffer, &res->dx11);

			if (findVertexLayoutIndex < 0)
			{
				vertexLayoutBuffer[vertexLayoutBufferCount].vertexSize = vertexSize;
				vertexLayoutBuffer[vertexLayoutBufferCount].descCount = descCount;
				vertexLayoutBuffer[vertexLayoutBufferCount].descs =
					(D3D11_INPUT_ELEMENT_DESC*)allocs->alloc(
						sizeof(D3D11_INPUT_ELEMENT_DESC) * descCount
					);

				memcpy(
					vertexLayoutBuffer[vertexLayoutBufferCount].descs,
					descBuffer,
					sizeof(D3D11_INPUT_ELEMENT_DESC) * descCount
				);
				findVertexLayoutIndex = vertexLayoutBufferCount;
				vertexLayoutBufferCount++;
			}
			// vertexlayout record end / remain layout index for geomtry buffer

			// geometry create buffer start
			g.vertexLayoutIndex = findVertexLayoutIndex;
			g.indexCount = m.geometry.indexCount;
			g.vertexCount = m.geometry.vertexCount;
			g.isSkinned = configs[ci].meshConfigs[mi].isSkinned;

			auto vertexCopy = [=](void* vptr) -> void
			{
				const FBXMeshChunk::FBXGeometryChunk& g = m.geometry;
				byte* ptr = static_cast<byte*>(vptr);
				for (uint i = 0; i < g.vertexCount; i++)
				{
					memcpy(ptr, g.vertices + i, sizeof(g.vertices[i]));
					ptr += sizeof(g.vertices[i]);
					if (g.normals)
					{
						memcpy(ptr, g.normals + i, sizeof(g.normals[i]));
						ptr += sizeof(g.normals[i]);
					}
					if (g.tangents)
					{
						memcpy(ptr, g.tangents + i, sizeof(g.tangents[i]));
						ptr += sizeof(g.tangents[i]);
					}
					if (g.binormals)
					{
						memcpy(ptr, g.binormals + i, sizeof(g.binormals[i]));
						ptr += sizeof(g.binormals[i]);
					}
					for (uint j = 0; j < g.uvSlotCount; j++)
						if (g.uvSlots[j])
						{
							memcpy(ptr, g.uvSlots[j] + i, sizeof(g.uvSlots[j][i]));
							ptr += sizeof(g.uvSlots[j][i]);
						}
					if (g.boneIndices && g.boneWeights)
					{
						memcpy(ptr, g.boneIndices + i, sizeof(g.boneIndices[i]));
						ptr += sizeof(g.boneIndices[i]);
						memcpy(ptr, g.boneWeights + i, sizeof(g.boneWeights[i]));
						ptr += sizeof(g.boneWeights[i]);
					}
				}
			};

			if (!configs[ci].meshConfigs[mi].isSkinned)
			{
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = vertexSizes[0] * g.vertexCount;
					desc.buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;
					desc.copyToPtr = vertexCopy;

					g.vertexBufferIndex = ReserveLoadBuffer(rawBuffer, &desc);
				}
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = sizeof(m.geometry.indices[0]) * m.geometry.indexCount;
					desc.buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;
					desc.subres.pSysMem = m.geometry.indices;

					g.indexBufferIndex = ReserveLoadBuffer(rawBuffer, &desc);
				}
			}
			else
			{
				g.streamedVertexSize = vertexSizes[1];

				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = vertexSizes[0] * g.vertexCount;
					desc.buffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					desc.buffer.CPUAccessFlags = 0;
					desc.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
					desc.buffer.StructureByteStride = vertexSizes[0];
					desc.copyToPtr = vertexCopy;

					g.vertexDataBufferIndex = ReserveLoadBuffer(rawBuffer, &desc);
				}
				{
					DX11BufferDesc desc;
					memset(&desc, 0, sizeof(DX11BufferDesc));
					desc.buffer.Usage = D3D11_USAGE_DEFAULT;
					desc.buffer.ByteWidth = sizeof(m.geometry.indices[0]) * m.geometry.indexCount;
					desc.buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
					desc.buffer.CPUAccessFlags = 0;
					desc.subres.pSysMem = m.geometry.indices;
					g.indexBufferIndex = ReserveLoadBuffer(rawBuffer, &desc);
				}
				{
					DX11SRVDesc desc;
					memset(&desc, 0, sizeof(DX11SRVDesc));

					desc.bufferIndex = g.vertexDataBufferIndex;
					desc.view.Format = DXGI_FORMAT_UNKNOWN;
					desc.view.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
					desc.view.Buffer.FirstElement = 0;
					desc.view.Buffer.NumElements = g.vertexCount;

					g.vertexDataSRVIndex = ReserveLoadShaderResourceView(rawBuffer, &desc);
				}
			}

			// geometry create end

			geometryOffset++;
		}

#pragma endregion
#pragma region load animations from fBX

		int findBoneSetIndex = -1;
		for (uint i = 0; i < res->boneSetCount; i++)
		{
			RenderResources::BoneSet& boneSet = res->boneSets[i];

			if (boneSet.boneCount == c.hierarchyCount)
			{
				bool equal = true;
				for (uint j = 0; j < boneSet.boneCount; j++)
				{
					if (boneSet.bones[j].parentIndex != c.hierarchyNodes[j].parentIndex ||
						boneSet.bones[j].childCount != c.hierarchyNodes[j].childCount ||
						boneSet.bones[j].childIndexStart != c.hierarchyNodes[j].childIndexStart ||
						strcmp(boneSet.bones[j].name, c.hierarchyNodes[j].name) != 0 ||
						memcmp(
							&boneSet.bones[j].inverseGlobalTransformMatrix,
							&c.hierarchyNodes[j].inverseGlobalTransformMatrix,
							sizeof(boneSet.bones[j].inverseGlobalTransformMatrix)
						) != 0
						)
					{
						equal = false;
						break;
					}
				}

				if (equal)
				{
					findBoneSetIndex = i;
					break;
				}
			}
		}


		if (findBoneSetIndex < 0)
		{
			res->boneSetCount++;
			RenderResources::BoneSet& boneSet = res->boneSets[res->boneSetCount - 1];

			ALLOC_RANGE_ZEROMEM(
				boneSet.boneCount, c.hierarchyCount, 
				RenderResources::BoneSet::Bone, boneSet.bones, allocs->alloc
			);

			for (uint i = 0; i < c.hierarchyCount; i++)
				boneSet.bones[i].inverseGlobalTransformMatrix = 
					c.hierarchyNodes[i].inverseGlobalTransformMatrix;

			DX11BufferDesc bd;
			memset(&bd, 0, sizeof(DX11BufferDesc));

			bd.buffer.Usage = D3D11_USAGE_DEFAULT;
			bd.buffer.ByteWidth = sizeof(Matrix4x4) * c.hierarchyCount;
			bd.buffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bd.buffer.CPUAccessFlags = 0;
			bd.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bd.buffer.StructureByteStride = sizeof(Matrix4x4);
			bd.copyToPtr = [=](void* ptr) {
				Matrix4x4* matrixBuffer = static_cast<Matrix4x4*>(ptr);
				for (uint i = 0; i < boneSet.boneCount; i++)
					matrixBuffer[i] = boneSet.bones[i].inverseGlobalTransformMatrix;
			};
			res->boneSets[res->boneSetCount - 1].bindPoseTransformBufferIndex =		
				ReserveLoadBuffer(rawBuffer, &bd);

			DX11SRVDesc srvd;
			memset(&srvd, 0, sizeof(DX11SRVDesc));

			srvd.bufferIndex = res->boneSets[res->boneSetCount - 1].bindPoseTransformBufferIndex;
			srvd.view.Format = DXGI_FORMAT_UNKNOWN;
			srvd.view.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
			srvd.view.Buffer.FirstElement = 0;
			srvd.view.Buffer.NumElements = c.hierarchyCount;

			res->boneSets[res->boneSetCount - 1].binePoseTransformSRVIndex = 
				ReserveLoadShaderResourceView(rawBuffer, &srvd);

			findBoneSetIndex = res->boneSetCount - 1;
		}

		for (uint ai = 0; ai < c.animationCount; ai++)
		{
			RenderResources::Animation& anim = res->anims[ai + animOffset];
			FBXChunk::FBXAnimation& fbxAnim = c.animations[ai];

			ALLOC_AND_STRCPY(anim.animName, fbxAnim.animationName, allocs->alloc);
			anim.fpsCount = fbxAnim.fpsCount;
			anim.frameKeyCount = fbxAnim.frameKeyCount;
			anim.boneSetIndex = findBoneSetIndex;

			DX11BufferDesc bd;
			memset(&bd, 0, sizeof(DX11BufferDesc));

			bd.buffer.Usage = D3D11_USAGE_DEFAULT;
			bd.buffer.ByteWidth = sizeof(Matrix4x4) * fbxAnim.frameKeyCount * c.hierarchyCount;
			bd.buffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bd.buffer.CPUAccessFlags = 0;
			bd.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bd.buffer.StructureByteStride = sizeof(Matrix4x4);
			bd.subres.pSysMem = fbxAnim.globalAffineTransforms;

			anim.animPoseTransformBufferIndex = ReserveLoadBuffer(rawBuffer, &bd);

			DX11SRVDesc srvd;
			memset(&srvd, 0, sizeof(DX11SRVDesc));

			srvd.bufferIndex = anim.animPoseTransformBufferIndex;
			srvd.view.Format = DXGI_FORMAT_UNKNOWN;
			srvd.view.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
			srvd.view.Buffer.FirstElement = 0;
			srvd.view.Buffer.NumElements = fbxAnim.frameKeyCount * c.hierarchyCount;

			anim.animPoseTransformSRVIndex = ReserveLoadShaderResourceView(rawBuffer, &srvd);

			animOffset++;
		}

#pragma endregion

	}

	// allocate & copy vertex layout
	res->dx11.vertexLayoutCount = vertexLayoutBufferCount;
	res->dx11.vertexLayouts =
		(DX11Resources::DX11LayoutChunk*)allocs->alloc(
			sizeof(DX11Resources::DX11LayoutChunk) *
			(res->dx11.vertexLayoutCount + res->dx11.vertexLayoutCount)
		);
	memcpy(
		res->dx11.vertexLayouts,
		vertexLayoutBuffer,
		sizeof(DX11Resources::DX11LayoutChunk) * res->dx11.vertexLayoutCount
	);

	return S_OK;
}

HRESULT ReserveTex2DAndSRVFromFileByDX11(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint dirCount, const wchar_t** dirs, uint textureBufferSize, void* allocatedtextureBuffer
)
{
	ASSERT(dirs != nullptr);

	ALLOC_RANGE_ZEROMEM(
		res->shaderTex2DCount, dirCount,
		RenderResources::ShaderTexture2D, res->shaderTex2Ds, allocs->alloc
	);

	for (uint i = 0; i < dirCount; i++)
	{
		RenderResources::ShaderTexture2D& shaderTex2D = res->shaderTex2Ds[i];
		uint reservedTexture2DIndex = static_cast<uint>(rawBuffer->tex2DDescs.size());

		DX11SRVDesc srvd;
		memset(&srvd, 0, sizeof(srvd));
		srvd.texture2DIndex = reservedTexture2DIndex;
		srvd.view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.view.Texture2D.MipLevels = 1;
		srvd.view.Texture2D.MostDetailedMip = 0;
		srvd.setSRVDesc = [=](D3D11_SHADER_RESOURCE_VIEW_DESC* viewDesc) -> bool
		{
			if (
				res->dx11.texture2Ds &&
				reservedTexture2DIndex < res->dx11.texture2DCount &&
				res->dx11.texture2Ds[reservedTexture2DIndex]
				)
			{
				D3D11_TEXTURE2D_DESC td;
				res->dx11.texture2Ds[reservedTexture2DIndex]->GetDesc(&td);
				viewDesc->Format = td.Format;
				return true;
			}
			else
				return false;
		};
		shaderTex2D.srvIndex = ReserveLoadShaderResourceView(rawBuffer, &srvd);

		DX11Texture2DDesc texDesc;
		memset(&texDesc, 0, sizeof(texDesc));
		texDesc.loadFromFile = true;
		texDesc.fileName = dirs[i];
		texDesc.usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		texDesc.bindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.miscFlags = 0;
		texDesc.cpuAccessFlags = 0;
		texDesc.forceSRGB = false;

		shaderTex2D.tex2DIndex = ReserveLoadTexture2D(rawBuffer, &texDesc);
	}

	return S_OK;
}

HRESULT ReserveShaderFromFileByDX11(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint compileCount, const ShaderCompileDesc* descs, DX11CompileDescToShader* descToFileShader
)
{
	std::vector<RenderResources::ShaderFile> files = std::vector<RenderResources::ShaderFile>();
	std::vector<std::array<std::vector<uint>, 6>> shaderIndicesByFile = std::vector<std::array<std::vector<uint>, 6>>();

	for (uint i = 0; i < compileCount; i++)
	{
		const ShaderCompileDesc& dsc = descs[i];
		int fileIndex = -1;
		for (int j = 0; j < files.size(); j++)
			if (wcscmp(files[j].fileName, dsc.fileName) == 0)
			{
				fileIndex = j;
				break;
			}

		if (fileIndex < 0)
		{
			RenderResources::ShaderFile file;
			memset(&file, 0, sizeof(RenderResources::ShaderFile));

			size_t len = wcslen(dsc.fileName);
			wchar_t* tempFileName = (wchar_t*)allocs->alloc(sizeof(wchar_t) * (len + 1));
			wcscpy_s(tempFileName, (len + 1), dsc.fileName);
			file.fileName = tempFileName;

			fileIndex = (int)files.size();
			files.push_back(file);
			std::array<std::vector<uint>, 6> arr;

			shaderIndicesByFile.push_back(arr);
		}

		RenderResources::ShaderFile& file = files[fileIndex];
		ShaderKind s;
		uint index = ReserveLoadShader(rawBuffer, &dsc, &s);
		if (index == UINT_MAX)
			continue;

		descToFileShader[i].shaderFileIndex = fileIndex;
		descToFileShader[i].shaderIndexInFile = shaderIndicesByFile[fileIndex][(int)s].size();
		shaderIndicesByFile[fileIndex][(int)s].push_back(index);
		descToFileShader[i].shaderKindIndex = (uint)s;
		descToFileShader[i].shaderIndex = index;
	}

	ALLOC_RANGE_MEMCPY(
		res->shaderFileCount, files.size(),
		RenderResources::ShaderFile, res->shaderFiles, files.data(), allocs->alloc
	);

	for (uint i = 0; i < res->shaderFileCount; i++)
	{
		RenderResources::ShaderFile& file = res->shaderFiles[i];

		for (int j = 0; j < 6; j++)
		{
			std::vector<uint>& v = shaderIndicesByFile[i][j];
			file.shaderIndices[j].count = (uint)v.size();
			file.shaderIndices[j].indices = (uint*)allocs->alloc(sizeof(uint) * v.size());
			memcpy(file.shaderIndices[j].indices, shaderIndicesByFile[i][(int)j].data(), sizeof(uint) * v.size());
		}
	}

	return S_OK;
}

HRESULT ReserveSkinningInstances(
	RenderResources* res, DX11InternalResourceDescBuffer* rawBuffer, const Allocaters* allocs,
	uint skinningInstanceCount, const SkinningInstanceDesc* skinningInstances
)
{
	ALLOC_RANGE_ZEROMEM(
		res->skinningCount, skinningInstanceCount,
		RenderResources::SkinningInstance, res->skinningInstances, allocs->alloc
	);

	for (uint i = 0; i < res->skinningCount; i++)
	{
		auto& d = skinningInstances[i];
		auto& item = res->skinningInstances[i];
		auto& geometry = res->geometryChunks[d.geometryIndex];

		item.geometryIndex = d.geometryIndex;
		item.animationIndex = d.animationIndex;

		{
			DX11BufferDesc desc;
			memset(&desc, 0, sizeof(DX11BufferDesc));
			desc.buffer.Usage = D3D11_USAGE_DEFAULT;
			desc.buffer.ByteWidth = geometry.streamedVertexSize * geometry.vertexCount;
			desc.buffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			desc.buffer.CPUAccessFlags = 0;
			desc.buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			desc.buffer.StructureByteStride = geometry.streamedVertexSize;

			item.vertexStreamBufferIndex = ReserveLoadBuffer(rawBuffer, &desc);
		}
		{
			DX11BufferDesc desc;
			memset(&desc, 0, sizeof(DX11BufferDesc));
			desc.buffer.Usage = D3D11_USAGE_DEFAULT;
			desc.buffer.ByteWidth = geometry.streamedVertexSize * geometry.vertexCount;
			desc.buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.buffer.CPUAccessFlags = 0;

			item.vertexBufferIndex = ReserveLoadBuffer(rawBuffer, &desc);
		}

		{
			DX11UAVDesc desc;
			memset(&desc, 0, sizeof(DX11UAVDesc));

			desc.bufferIndex = item.vertexStreamBufferIndex;
			desc.view.Format = DXGI_FORMAT_UNKNOWN;
			desc.view.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_BUFFER;
			desc.view.Buffer.FirstElement = 0;
			desc.view.Buffer.NumElements = geometry.vertexCount;
			desc.view.Buffer.Flags = 0;

			item.vertexStreamUAVIndex = ReserveLoadUnorderedAccessView(rawBuffer, &desc);
		}
	}
	return S_OK;
}

#include "dx11res.h"
#include "dx11depend.h"

struct ShaderDesc
{
	const wchar_t* fileName;
	const char* entrypoint;
};
enum class ShaderResourceKind : uint
{
	SRV,
	CBUFFER,
	SAMPLER,
	UAV
};
struct ShaderResourceDesc
{
	ShaderResourceKind kind;
	uint slot;

};

struct StaticMeshTask
{
	wchar_t* fbxFilePath;
	uint fbxGeometryIndex;


	ShaderDesc vertexShader;
	ShaderDesc pixelShader;
};

struct SkeletalMeshTask
{
	uint fbxIndex;
	uint geometryIndex;
	uint vertexShaderIndex;
	uint pixelShaderIndex;
};

struct RenderTask
{
	int staticMeshTaskCount;
	StaticMeshTask* staticMeshTasks;

	int skeletalMeshTaskCount;
	SkeletalMeshTask* skeletalMeshTasks;
};

HRESULT LoadDependAndDX11Resource(IN const RenderTask* task, OUT DX11PipelineDependancy** depends, OUT DX11ResourceDesc* res)
{


	return S_OK;
}
