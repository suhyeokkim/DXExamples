#include "dx11depend.h"
#include "dx11res.h"
#include "defined_alloc_macro.h"

#include <iostream>
#include <windows.h>

HRESULT DependancyContextStatePrepare(RenderContextState* state, const DX11PipelineDependancySet* set)
{
	DependancyContextStatePrepare(state, set->frameDependancyCount, set->frameDependancy);
	DependancyContextStatePrepare(state, set->initDependancyCount, set->initDependancy);
	DependancyContextStatePrepare(state, set->resizeDependancyCount, set->resizeDependancy);

	return S_OK;
}
HRESULT DependancyContextStatePrepare(RenderContextState* state, uint dependCount, const DX11PipelineDependancy* depends)
{
	uint maxCount = state->bufferCount;
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11PipelineDependancy& depend = depends[i];

		switch (depend.pipelineKind)
		{
		case PipelineKind::Draw:
			for (int k = 0; k < 5; k++)
			{
				const DX11ShaderResourceDependancy& shaderDep = depend.draw.dependants[k];

				for (uint j = 0; j < shaderDep.constantBufferCount; j++)
				{
					auto ref = shaderDep.constantBuffers[j];
					maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
				for (uint j = 0; j < shaderDep.samplerCount; j++)
				{
					auto ref = shaderDep.samplers[j];
					maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
				for (uint j = 0; j < shaderDep.srvCount; j++)
				{
					auto ref = shaderDep.srvs[j];
					maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
				for (uint j = 0; j < shaderDep.uavCount; j++)
				{
					auto ref = shaderDep.uavs[j];
					maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
			}
			break;
		case PipelineKind::Compute:
		{
			const DX11ShaderResourceDependancy& shaderDep = depend.compute.resources;

			for (uint j = 0; j < shaderDep.constantBufferCount; j++)
			{
				auto ref = shaderDep.constantBuffers[j];
				maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
			for (uint j = 0; j < shaderDep.samplerCount; j++)
			{
				auto ref = shaderDep.samplers[j];
				maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
			for (uint j = 0; j < shaderDep.srvCount; j++)
			{
				auto ref = shaderDep.srvs[j];
				maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
			for (uint j = 0; j < shaderDep.uavCount; j++)
			{
				auto ref = shaderDep.uavs[j];
				maxCount = MAX(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
		}
		break;
		case PipelineKind::Copy:
			if (depend.copy.kind == CopyKind::UpdateSubResource)
			{
				maxCount = MAX(depend.copy.args.updateSubRes.dataBufferSize + sizeof(D3D11_BOX), maxCount);
			}
			break;
		}
	}

	state->bufferCount = maxCount;
	ALLOC_OVERLOADED_SIZED(EASTL_PERSISTANT_NAME, state->bufferPtrBuffer, void*, maxCount);
	ALLOC_OVERLOADED_SIZED(EASTL_PERSISTANT_NAME, state->numberBuffer, uint, sizeof(uint) * 2);

	return S_OK;
}
HRESULT CopyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11CopyDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11CopyDependancy& cd = depends[i];

		switch (depends[i].kind)
		{
		case CopyKind::CopyResource:
			deviceContext->CopyResource(res->buffers[depends[i].args.copyRes.dstBufferIndex], res->buffers[depends[i].args.copyRes.srcBufferIndex]);
			break;
		case CopyKind::UpdateSubResource:
		{
			const D3D11_BOX* destBox = nullptr;
			if ((bool)cd.args.updateSubRes.getBoxFunc)
				destBox = cd.args.updateSubRes.getBoxFunc((D3D11_BOX*)state->bufferPtrBuffer);

			void* dataPtr = ((byte*)state->bufferPtrBuffer + sizeof(D3D11_BOX));
			if ((bool)cd.args.updateSubRes.copyToBufferFunc)
				cd.args.updateSubRes.copyToBufferFunc(dataPtr, cd.args.updateSubRes.param);

			deviceContext->UpdateSubresource(
				res->buffers[cd.args.updateSubRes.resIndex], 
				cd.args.updateSubRes.dstSubres, 
				destBox, 
				dataPtr, 
				cd.args.updateSubRes.srcRowPitch, 
				cd.args.updateSubRes.srcDepthPitch
			);
			break;
		}
		}
	}

	return S_OK;
}

HRESULT ExecuteExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11PipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		switch (depends[i].pipelineKind)
		{
		case PipelineKind::Draw:
			DrawExplicitlyDX11(deviceContext, state, res, 1, &depends[i].draw);
			break;
		case PipelineKind::Compute:
			ComputeExplicitlyDX11(deviceContext, state, res, 1, &depends[i].compute);
			break;
		case PipelineKind::Copy:
			CopyDX11(deviceContext, state, res, 1, &depends[i].copy);
			break;
		}
	}

	return S_OK;
}
HRESULT ComputeExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11ComputePipelineDependancy* depends)
{
	const DX11Resources* dx11Res = res;

	for (uint i = 0; i < dependCount; i++)
	{
		const DX11ComputePipelineDependancy& cpd = depends[i];
		const DX11ShaderResourceDependancy& srd = cpd.resources;

		if (srd.shaderFileIndex < 0) continue;

		deviceContext->CSSetShader(dx11Res->shaders.css[res->shaderFiles[srd.shaderFileIndex].csIndices[srd.shaderIndex]].cs, nullptr, 0);

		for (uint j = 0; j < srd.constantBufferCount; j++)
		{
			auto ref = srd.constantBuffers[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = dx11Res->buffers[dx11Res->constantBufferIndices[ref.indices[k]]];
			deviceContext->CSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
		}
		for (uint j = 0; j < srd.samplerCount; j++)
		{
			auto ref = srd.samplers[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = dx11Res->samplerStates[ref.indices[k]];
			deviceContext->CSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
		}
		for (uint j = 0; j < srd.srvCount; j++)
		{
			auto ref = srd.srvs[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = dx11Res->srvs[ref.indices[k]];
			deviceContext->CSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
		}
		for (uint j = 0; j < srd.uavCount; j++)
		{
			auto ref = srd.uavs[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = dx11Res->uavs[ref.indices[k]];
			// TODO:: set uav argument for append, consume buffer
			state->numberBuffer[0] = state->numberBuffer[1] = 0;
			deviceContext->CSSetUnorderedAccessViews(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11UnorderedAccessView *const *>(state->bufferPtrBuffer), state->numberBuffer);
		}

		switch (cpd.dispatchType)
		{
		case DX11ComputePipelineDependancy::DispatchType::Dispatch:
			deviceContext->Dispatch(
				cpd.argsAsDispatch.dispatch.threadGroupCountX,
				cpd.argsAsDispatch.dispatch.threadGroupCountY,
				cpd.argsAsDispatch.dispatch.threadGroupCountZ
			);
			break;
		case DX11ComputePipelineDependancy::DispatchType::DispatchIndirect:
			deviceContext->DispatchIndirect(
				dx11Res->buffers[cpd.argsAsDispatch.dispatchIndirect.bufferIndex],
				cpd.argsAsDispatch.dispatchIndirect.bufferIndex
			);
			break;
		}

		for (uint j = 0; j < srd.uavCount; j++)
		{
			auto ref = srd.uavs[j];
			for (uint k = 0; k < ref.indexCount; k++)
				state->bufferPtrBuffer[k] = nullptr;
			// TODO:: set uav argument for append, consume buffer
			state->numberBuffer[0] = state->numberBuffer[1] = 0;
			deviceContext->CSSetUnorderedAccessViews(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11UnorderedAccessView *const *>(state->bufferPtrBuffer), state->numberBuffer);
		}
	}

	return S_OK;
}
HRESULT DrawExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const DX11Resources* res, uint dependCount, const DX11DrawPipelineDependancy* depends)
{
	const DX11Resources* dx11Res = res;

	for (uint i = 0; i < dependCount; i++)
	{
		const DX11DrawPipelineDependancy& depend = depends[i];
		const auto& depIn = depend.input;
		const auto& resGeo = res->geometryChunks[depIn.geometryIndex];

		deviceContext->IASetInputLayout(dx11Res->inputLayoutItems[depIn.inputLayoutIndex]);
		if (depIn.vertexBufferIndex < UINT_MAX)
			deviceContext->IASetVertexBuffers(
				0, 1, &dx11Res->buffers[depIn.vertexBufferIndex], &depIn.vertexSize, &depIn.vertexBufferOffset
			);
		else
		{
			ID3D11Buffer* buffer = nullptr;
			uint size = 0, offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &buffer, &size, &offset);
		}
		deviceContext->IASetIndexBuffer(dx11Res->buffers[resGeo.indexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(depIn.topology);

		if (depend.vs.shaderFileIndex >= 0)
		{
			deviceContext->VSSetShader(dx11Res->shaders.vss[res->shaderFiles[depend.vs.shaderFileIndex].vsIndices[depend.vs.shaderIndex]].vs, nullptr, 0);

			for (uint j = 0; j < depend.vs.constantBufferCount; j++)
			{
				auto ref = depend.vs.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->buffers[dx11Res->constantBufferIndices[ref.indices[k]]];
				deviceContext->VSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.vs.samplerCount; j++)
			{
				auto ref = depend.vs.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->samplerStates[ref.indices[k]];
				deviceContext->VSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.vs.srvCount; j++)
			{
				auto ref = depend.vs.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->srvs[ref.indices[k]];
				deviceContext->VSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.ps.shaderFileIndex >= 0)
		{
			deviceContext->PSSetShader(dx11Res->shaders.pss[res->shaderFiles[depend.ps.shaderFileIndex].psIndices[depend.ps.shaderIndex]].ps, nullptr, 0);

			for (uint j = 0; j < depend.ps.constantBufferCount; j++)
			{
				auto ref = depend.ps.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->buffers[dx11Res->constantBufferIndices[ref.indices[k]]];
				deviceContext->PSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ps.samplerCount; j++)
			{
				auto ref = depend.ps.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->samplerStates[ref.indices[k]];
				deviceContext->PSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ps.srvCount; j++)
			{
				auto ref = depend.ps.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->srvs[ref.indices[k]];
				deviceContext->PSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}


		if (depend.gs.shaderFileIndex >= 0)
		{
			deviceContext->GSSetShader(dx11Res->shaders.gss[res->shaderFiles[depend.gs.shaderFileIndex].gsIndices[depend.gs.shaderIndex]].gs, nullptr, 0);

			for (uint j = 0; j < depend.gs.constantBufferCount; j++)
			{
				auto ref = depend.gs.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->buffers[dx11Res->constantBufferIndices[ref.indices[k]]];
				deviceContext->GSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.gs.samplerCount; j++)
			{
				auto ref = depend.gs.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->samplerStates[ref.indices[k]];
				deviceContext->GSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.gs.srvCount; j++)
			{
				auto ref = depend.gs.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->srvs[ref.indices[k]];
				deviceContext->GSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.hs.shaderFileIndex >= 0)
		{
			deviceContext->HSSetShader(dx11Res->shaders.hss[res->shaderFiles[depend.hs.shaderFileIndex].hsIndices[depend.hs.shaderIndex]].hs, nullptr, 0);

			for (uint j = 0; j < depend.hs.constantBufferCount; j++)
			{
				auto ref = depend.hs.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->buffers[dx11Res->constantBufferIndices[ref.indices[k]]];
				deviceContext->HSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.hs.samplerCount; j++)
			{
				auto ref = depend.hs.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->samplerStates[ref.indices[k]];
				deviceContext->HSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.hs.srvCount; j++)
			{
				auto ref = depend.hs.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->srvs[ref.indices[k]];
				deviceContext->HSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		if (depend.ds.shaderFileIndex >= 0)
		{
			deviceContext->DSSetShader(dx11Res->shaders.dss[res->shaderFiles[depend.ds.shaderFileIndex].dsIndices[depend.ds.shaderIndex]].ds, nullptr, 0);

			for (uint j = 0; j < depend.ds.constantBufferCount; j++)
			{
				auto ref = depend.ds.constantBuffers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->buffers[dx11Res->constantBufferIndices[ref.indices[k]]];
				deviceContext->DSSetConstantBuffers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11Buffer *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ds.samplerCount; j++)
			{
				auto ref = depend.ds.samplers[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->samplerStates[ref.indices[k]];
				deviceContext->DSSetSamplers(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11SamplerState *const *>(state->bufferPtrBuffer));
			}
			for (uint j = 0; j < depend.ds.srvCount; j++)
			{
				auto ref = depend.ds.srvs[j];
				for (uint k = 0; k < ref.indexCount; k++)
					state->bufferPtrBuffer[k] = dx11Res->srvs[ref.indices[k]];
				deviceContext->DSSetShaderResources(ref.slotOrRegister, ref.indexCount, reinterpret_cast<ID3D11ShaderResourceView *const *>(state->bufferPtrBuffer));
			}
		}

		switch (depend.drawType)
		{
		case DX11DrawPipelineDependancy::DrawType::Draw:
			deviceContext->Draw(
				depend.argsAsDraw.drawArgs.vertexCount, depend.argsAsDraw.drawArgs.startVertexLocation
			);
			break;
		case DX11DrawPipelineDependancy::DrawType::DrawIndexed:
			deviceContext->DrawIndexed(
				depend.argsAsDraw.drawIndexedArgs.indexCount,
				depend.argsAsDraw.drawIndexedArgs.startIndexLocation,
				depend.argsAsDraw.drawIndexedArgs.baseVertexLocation
			);
			break;
		}
	}
	return S_OK;
}
HRESULT ReleaseShaderDependancy(DX11ShaderResourceDependancy* dependancy);
HRESULT ReleaseDrawDependancy(DX11DrawPipelineDependancy* dependancy);
HRESULT ReleaseComputeDependancy(DX11ComputePipelineDependancy* dependancy);
HRESULT ReleaseCopyDependancy(DX11CopyDependancy* dependancy);

HRESULT ReleaseDX11Dependancy(DX11PipelineDependancySet* set)
{
	ReleaseDX11Dependancy(set->frameDependancyCount, set->frameDependancy);
	ReleaseDX11Dependancy(set->initDependancyCount, set->initDependancy);
	ReleaseDX11Dependancy(set->resizeDependancyCount, set->resizeDependancy);

	return S_OK;
}
HRESULT ReleaseDX11Dependancy(uint dependCount, DX11PipelineDependancy* dependancy)
{
	if (dependancy)
	{
		for (uint i = 0; i < dependCount; i++)
		{
			switch (dependancy[i].pipelineKind)
			{
			case PipelineKind::Draw:
				ReleaseDrawDependancy(&dependancy[i].draw);
				break;
			case PipelineKind::Compute:
				ReleaseComputeDependancy(&dependancy[i].compute);
				break;
			case PipelineKind::Copy:
				ReleaseCopyDependancy(&dependancy[i].copy);
				break;
			}
		}

		SAFE_DELETE_OVERLOADED(dependancy, EASTL_PERSISTANT_NAME);
	}

	return S_OK;
}
HRESULT ReleaseShaderDependancy(DX11ShaderResourceDependancy* dependancy)
{
	for (uint i = 0; i < dependancy->srvCount; i++)
		SAFE_DELETE_OVERLOADED(dependancy->srvs[i].indices, EASTL_PERSISTANT_NAME);
	SAFE_DELETE_OVERLOADED(dependancy->srvs, EASTL_PERSISTANT_NAME);

	for (uint i = 0; i < dependancy->uavCount; i++)
		SAFE_DELETE_OVERLOADED(dependancy->uavs[i].indices, EASTL_PERSISTANT_NAME);
	SAFE_DELETE_OVERLOADED(dependancy->uavs, EASTL_PERSISTANT_NAME);

	for (uint i = 0; i < dependancy->samplerCount; i++)
		SAFE_DELETE_OVERLOADED(dependancy->samplers[i].indices, EASTL_PERSISTANT_NAME);
	SAFE_DELETE_OVERLOADED(dependancy->samplers, EASTL_PERSISTANT_NAME);

	for (uint i = 0; i < dependancy->constantBufferCount; i++)
		SAFE_DELETE_OVERLOADED(dependancy->constantBuffers[i].indices, EASTL_PERSISTANT_NAME);
	SAFE_DELETE_OVERLOADED(dependancy->constantBuffers, EASTL_PERSISTANT_NAME);

	return S_OK;
}
HRESULT ReleaseDrawDependancy(DX11DrawPipelineDependancy* dependancy)
{
	for (int i = 0; i < 5; i++)
		ReleaseShaderDependancy(dependancy->dependants + i);

	return S_OK;
}
HRESULT ReleaseComputeDependancy(DX11ComputePipelineDependancy* dependancy)
{
	ReleaseShaderDependancy(&dependancy->resources);
	return S_OK;
}
HRESULT ReleaseCopyDependancy(DX11CopyDependancy* dependancy)
{
	return S_OK;
}
HRESULT ReleaseContext(RenderContextState* context)
{
	SAFE_DELETE_OVERLOADED(context->bufferPtrBuffer, EASTL_PERSISTANT_NAME);
	SAFE_DELETE_OVERLOADED(context->numberBuffer, EASTL_PERSISTANT_NAME);
	return S_OK;
}

using namespace std;

void PrintShaderResourceDependancy(const char* prefix0, const char* prefix1, const DX11ShaderResourceDependancy& srd)
{
	cout << prefix0 << prefix1 << "shaderFileIndex :: " << srd.shaderFileIndex << ", shaderIndex :: " << srd.shaderIndex << endl;

	cout << prefix0 << prefix1 << "constantBufferCount :: " << srd.constantBufferCount << endl;
	for (uint i = 0; i < srd.constantBufferCount; i++)
	{
		cout << prefix0 << prefix1 << "cb[" << i << "].indexCount :: " << srd.constantBuffers[i].indexCount << endl;
		cout << prefix0 << prefix1 << "cb[" << i << "].slotOrRegister :: " << srd.constantBuffers[i].slotOrRegister << endl;
		for (uint j = 0; j < srd.constantBuffers[i].indexCount; j++)
			cout << prefix0 << prefix1 << "cb[" << i << "].indcices[" << j << "] :: " << srd.constantBuffers[i].indices[j] << endl;
	}

	cout << prefix0 << prefix1 << "srvCount :: " << srd.srvCount << endl;
	for (uint i = 0; i < srd.srvCount; i++)
	{
		cout << prefix0 << prefix1 << "srv[" << i << "].indexCount :: " << srd.srvs[i].indexCount << endl;
		cout << prefix0 << prefix1 << "srv[" << i << "].slotOrRegister :: " << srd.srvs[i].slotOrRegister << endl;
		for (uint j = 0; j < srd.srvs[i].indexCount; j++)
			cout << prefix0 << prefix1 << "srv[" << i << "].indcices[" << j << "] :: " << srd.srvs[i].indices[j] << endl;
	}

	cout << prefix0 << prefix1 << "samplerCount :: " << srd.samplerCount << endl;
	for (uint i = 0; i < srd.samplerCount; i++)
	{
		cout << prefix0 << prefix1 << "sampler[" << i << "].indexCount :: " << srd.samplers[i].indexCount << endl;
		cout << prefix0 << prefix1 << "sampler[" << i << "].slotOrRegister :: " << srd.samplers[i].slotOrRegister << endl;
		for (uint j = 0; j < srd.samplers[i].indexCount; j++)
			cout << prefix0 << prefix1 << "sampler[" << i << "].indcices[" << j << "] :: " << srd.samplers[i].indices[j] << endl;
	}

	cout << prefix0 << prefix1 << "uavCount :: " << srd.uavCount << endl;
	for (uint i = 0; i < srd.uavCount; i++)
	{
		cout << prefix0 << prefix1 << "uav[" << i << "].indexCount :: " << srd.uavs[i].indexCount << endl;
		cout << prefix0 << prefix1 << "uav[" << i << "].slotOrRegister :: " << srd.uavs[i].slotOrRegister << endl;
		for (uint j = 0; j < srd.uavs[i].indexCount; j++)
			cout << prefix0 << prefix1 << "uav[" << i << "].indcices[" << j << "] :: " << srd.uavs[i].indices[j] << endl;
	}
}

void PrintPipelineDependancy(const char* prefix, const DX11PipelineDependancy& d)
{
	switch (d.pipelineKind)
	{
	case PipelineKind::Draw:
	{
		const DX11DrawPipelineDependancy& dr = d.draw;

		cout << prefix << "> " << "drawType :: " << (uint)dr.drawType << endl;

		switch (dr.drawType)
		{
		case DX11DrawPipelineDependancy::DrawType::Draw:
			cout << prefix << "> " << "drawArgs.vertexCount :: " << dr.argsAsDraw.drawArgs.vertexCount << endl;
			cout << prefix << "> " << "drawArgs.startVertexLocation :: " << dr.argsAsDraw.drawArgs.startVertexLocation << endl;
			break;
		case DX11DrawPipelineDependancy::DrawType::DrawIndexed:
			cout << prefix << "> " << "drawIndexedArgs.indexCount :: " << dr.argsAsDraw.drawIndexedArgs.indexCount << endl;
			cout << prefix << "> " << "drawIndexedArgs.startIndexLocation :: " << dr.argsAsDraw.drawIndexedArgs.startIndexLocation << endl;
			cout << prefix << "> " << "drawIndexedArgs.baseVertexLocation :: " << dr.argsAsDraw.drawIndexedArgs.baseVertexLocation << endl;
			break;
		}

		cout << prefix << "> " << "input.inputlayout :: " << dr.input.inputLayoutIndex << endl;
		cout << prefix << "> " << "input.geometry :: " << dr.input.geometryIndex << endl;
		cout << prefix << "> " << "input.topology :: " << dr.input.topology << endl;
		cout << prefix << "> " << "input.vertexBufferIndex :: " << dr.input.vertexBufferIndex << endl;
		cout << prefix << "> " << "input.vertexBufferOffset :: " << dr.input.vertexBufferOffset << endl;
		cout << prefix << "> " << "input.vertexSize :: " << dr.input.vertexSize << endl;

		for (int i = 0; i < 5; i++)
		{
			const DX11ShaderResourceDependancy& srd = dr.dependants[i];
			cout << prefix << "> " << "dependants[" << i << "]" <<  endl;
			PrintShaderResourceDependancy(prefix, ">> ", srd);
		}
	}
		break;
	case PipelineKind::Compute:
	{
		const DX11ComputePipelineDependancy& cm = d.compute;
		cout << prefix << "> " << "dispatchType :: " << (uint)cm.dispatchType << endl;

		switch (cm.dispatchType)
		{
		case DX11ComputePipelineDependancy::DispatchType::Dispatch:
			cout << prefix << "> " << "dispatch.threadGroupCount :: " << cm.argsAsDispatch.dispatch.threadGroupCountX;
			cout << ", " << cm.argsAsDispatch.dispatch.threadGroupCountY << ", ";
			cout << cm.argsAsDispatch.dispatch.threadGroupCountZ << endl;
			break;
		case DX11ComputePipelineDependancy::DispatchType::DispatchIndirect:
			cout << prefix << "> " << "dispatchIndirect.bufferIndex :: " << cm.argsAsDispatch.dispatchIndirect.bufferIndex << endl;
			cout << prefix << "> " << "dispatchIndirect.alignedByteOffset :: " << cm.argsAsDispatch.dispatchIndirect.alignedByteOffset << endl;
			break;
		}

		PrintShaderResourceDependancy(prefix, ">> ", cm.resources);
	}
		break;
	case PipelineKind::Copy:
	{
		const DX11CopyDependancy& cp = d.copy;
		cout << prefix << "> " << "copy kind :: " << (uint)cp.kind << endl;
		switch (cp.kind)
		{
		case CopyKind::CopyResource:
			cout << prefix << "> CopyResource :: ";
			cout << cp.args.copyRes.srcBufferIndex << " ->> " << cp.args.copyRes.dstBufferIndex << endl;
			break;
		case CopyKind::UpdateSubResource:
			cout << prefix << "> updateSubRes.resKind :: " << (uint)cp.args.updateSubRes.resKind << endl;
			cout << prefix << "> updateSubRes.resIndex :: " << cp.args.updateSubRes.resIndex << endl;
			cout << prefix << "> updateSubRes.dstSubres :: " << cp.args.updateSubRes.dstSubres << endl;
			cout << prefix << "> updateSubRes.getBoxFunc :: " << (bool)cp.args.updateSubRes.getBoxFunc << endl;
			cout << prefix << "> updateSubRes.param :: " << (lint)cp.args.updateSubRes.param << endl;
			cout << prefix << "> updateSubRes.copyToBufferFunc :: " << (bool)cp.args.updateSubRes.copyToBufferFunc << endl;
			cout << prefix << "> updateSubRes.srcRowPitch :: " << cp.args.updateSubRes.srcRowPitch << endl;
			cout << prefix << "> updateSubRes.srcDepthPitch :: " << cp.args.updateSubRes.srcDepthPitch << endl;
			break;
		}
	}
		break;
	}
}