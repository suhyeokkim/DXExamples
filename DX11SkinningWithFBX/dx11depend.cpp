#include "dx11depend.h"

HRESULT DependancyContextStatePrepare(RenderContextState* state, const Allocaters* allocs, uint dependCount, const DX11PipelineDependancy* depends)
{
	uint maxCount = state->bufferCount;
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11PipelineDependancy& depend = depends[i];

		switch (depend.pipelineKind)
		{
		case PIPELINE_KIND::DRAW:
			for (int k = 0; k < 5; k++)
			{
				const DX11ShaderResourceDependancy& shaderDep = depend.draw.dependants[k];

				for (uint j = 0; j < shaderDep.constantBufferCount; j++)
				{
					auto ref = shaderDep.constantBuffers[j];
					maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
				for (uint j = 0; j < shaderDep.samplerCount; j++)
				{
					auto ref = shaderDep.samplers[j];
					maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
				for (uint j = 0; j < shaderDep.srvCount; j++)
				{
					auto ref = shaderDep.srvs[j];
					maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
				for (uint j = 0; j < shaderDep.uavCount; j++)
				{
					auto ref = shaderDep.uavs[j];
					maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
				}
			}
			break;
		case PIPELINE_KIND::COMPUTE:
		{
			const DX11ShaderResourceDependancy& shaderDep = depend.compute.resources;

			for (uint j = 0; j < shaderDep.constantBufferCount; j++)
			{
				auto ref = shaderDep.constantBuffers[j];
				maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
			for (uint j = 0; j < shaderDep.samplerCount; j++)
			{
				auto ref = shaderDep.samplers[j];
				maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
			for (uint j = 0; j < shaderDep.srvCount; j++)
			{
				auto ref = shaderDep.srvs[j];
				maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
			for (uint j = 0; j < shaderDep.uavCount; j++)
			{
				auto ref = shaderDep.uavs[j];
				maxCount = max(ref.indexCount * sizeof(ref.indices[0]), maxCount);
			}
		}
		break;
		case PIPELINE_KIND::COPY:
			if (depend.copy.kind == CopyKind::UPDATE_SUBRESOURCE)
			{
				maxCount = max(depend.copy.dataBufferSize + sizeof(D3D11_BOX), maxCount);
			}
			break;
		}
	}

	if (state->bufferCount < maxCount)
	{
		state->bufferCount = maxCount;
		state->bufferPtrBuffer = (void**)allocs->realloc(state->bufferPtrBuffer, maxCount);
		state->numberBuffer = (uint*)allocs->realloc(state->numberBuffer, sizeof(uint) * 2);
	}

	return S_OK;
}
HRESULT CopyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11CopyDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		const DX11CopyDependancy& cd = depends[i];

		switch (depends[i].kind)
		{
		case CopyKind::COPY_RESOURCE:
			deviceContext->CopyResource(res->dx11.buffers[depends[i].dstBufferIndex], res->dx11.buffers[depends[i].srcBufferIndex]);
			break;
		case CopyKind::UPDATE_SUBRESOURCE:
		{
			const D3D11_BOX* destBox = cd.getBoxFunc((D3D11_BOX*)state->bufferPtrBuffer);
			void* dataPtr = ((byte*)state->bufferPtrBuffer + sizeof(D3D11_BOX));
			cd.copyToBufferFunc(dataPtr);
			deviceContext->UpdateSubresource(res->dx11.buffers[cd.dstBufferIndex], cd.dstSubres, destBox, dataPtr, cd.srcRowPitch, cd.srcDepthPitch);
			break;
		}
		}
	}

	return S_OK;
}

HRESULT ExecuteExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11PipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		switch (depends[i].pipelineKind)
		{
		case PIPELINE_KIND::DRAW:
			DrawExplicitlyDX11(deviceContext, state, res, 1, &depends[i].draw);
			break;
		case PIPELINE_KIND::COMPUTE:
			ComputeExplicitlyDX11(deviceContext, state, res, 1, &depends[i].compute);
			break;
		case PIPELINE_KIND::COPY:
			CopyDX11(deviceContext, state, res, 1, &depends[i].copy);
			break;
		}
	}

	return S_OK;
}
HRESULT ExecuteImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11PipelineDependancy* depends)
{
	for (uint i = 0; i < dependCount; i++)
	{
		switch (depends[i].pipelineKind)
		{
		case PIPELINE_KIND::DRAW:
			DrawImplicitlyDX11(deviceContext, state, res, 1, &depends[i].draw);
			break;
		case PIPELINE_KIND::COMPUTE:
			ComputeImplicitlyDX11(deviceContext, state, res, 1, &depends[i].compute);
			break;
		case PIPELINE_KIND::COPY:
			CopyDX11(deviceContext, state, res, 1, &depends[i].copy);
			break;
		}
	}

	return S_OK;
}
HRESULT ComputeExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11ComputePipelineDependancy* depends)
{
	const DX11Resources* dx11Res = &res->dx11;

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
		case DX11ComputePipelineDependancy::DispatchType::DISPATCH:
			deviceContext->Dispatch(
				cpd.argsAsDispatch.dispatch.threadGroupCountX,
				cpd.argsAsDispatch.dispatch.threadGroupCountY,
				cpd.argsAsDispatch.dispatch.threadGroupCountZ
			);
			break;
		case DX11ComputePipelineDependancy::DispatchType::DISPATCH_INDIRECT:
			deviceContext->DispatchIndirect(
				dx11Res->buffers[cpd.argsAsDispatch.dispatchIndirect.bufferIndex],
				cpd.argsAsDispatch.dispatchIndirect.bufferIndex
			);
			break;
		}
	}

	return S_OK;
}
HRESULT ComputeImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11ComputePipelineDependancy* depends)
{
	return E_NOTIMPL;
}

HRESULT DrawExplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11DrawPipelineDependancy* depends)
{
	const DX11Resources* dx11Res = &res->dx11;

	for (uint i = 0; i < dependCount; i++)
	{
		const DX11DrawPipelineDependancy& depend = depends[i];
		const auto& depIn = depend.input;
		const auto& resGeo = res->geometryChunks[depIn.geometryIndex];

		deviceContext->IASetInputLayout(dx11Res->inputLayoutItems[depIn.inputLayoutIndex]);
		deviceContext->IASetVertexBuffers(
			0,
			1,
			&dx11Res->buffers[depIn.vertexBufferIndex],
			&depIn.vertexSize,
			&depIn.vertexBufferOffset);
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
		case DX11DrawPipelineDependancy::DrawType::DRAW:
			deviceContext->Draw(
				depend.argsAsDraw.drawArgs.vertexCount, depend.argsAsDraw.drawArgs.startVertexLocation
			);
			break;
		case DX11DrawPipelineDependancy::DrawType::DRAW_INDEXED:
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
HRESULT DrawImplicitlyDX11(ID3D11DeviceContext* deviceContext, RenderContextState* state, const RenderResources* res, uint dependCount, const DX11DrawPipelineDependancy* depends)
{
	return E_NOTIMPL;
}
HRESULT ReleaseShaderDependancy(DX11ShaderResourceDependancy* dependancy, const Allocaters* allocs);
HRESULT ReleaseDrawDependancy(DX11DrawPipelineDependancy* dependancy, const Allocaters* allocs);
HRESULT ReleaseComputeDependancy(DX11ComputePipelineDependancy* dependancy, const Allocaters* allocs);
HRESULT ReleaseCopyDependancy(DX11CopyDependancy* dependancy, const Allocaters* allocs);

HRESULT ReleaseDX11Dependancy(uint dependCount, DX11PipelineDependancy* dependancy, const Allocaters* allocs)
{
	if (dependancy)
	{
		for (uint i = 0; i < dependCount; i++)
		{
			switch (dependancy[i].pipelineKind)
			{
			case PIPELINE_KIND::DRAW:
				ReleaseDrawDependancy(&dependancy[i].draw, allocs);
				break;
			case PIPELINE_KIND::COMPUTE:
				ReleaseComputeDependancy(&dependancy[i].compute, allocs);
				break;
			case PIPELINE_KIND::COPY:
				ReleaseCopyDependancy(&dependancy[i].copy, allocs);
				break;
			}

		}

		allocs->dealloc(dependancy);
	}

	return S_OK;
}
HRESULT ReleaseShaderDependancy(DX11ShaderResourceDependancy* dependancy, const Allocaters* allocs)
{
	for (uint i = 0; i < dependancy->srvCount; i++)
		SAFE_DEALLOC(dependancy->srvs[i].indices, allocs->dealloc);
	SAFE_DEALLOC(dependancy->srvs, allocs->dealloc);

	for (uint i = 0; i < dependancy->uavCount; i++)
		SAFE_DEALLOC(dependancy->uavs[i].indices, allocs->dealloc);
	SAFE_DEALLOC(dependancy->uavs, allocs->dealloc);

	for (uint i = 0; i < dependancy->samplerCount; i++)
		SAFE_DEALLOC(dependancy->samplers[i].indices, allocs->dealloc);
	SAFE_DEALLOC(dependancy->samplers, allocs->dealloc);

	for (uint i = 0; i < dependancy->constantBufferCount; i++)
		SAFE_DEALLOC(dependancy->constantBuffers[i].indices, allocs->dealloc);
	SAFE_DEALLOC(dependancy->constantBuffers, allocs->dealloc);

	return S_OK;
}
HRESULT ReleaseDrawDependancy(DX11DrawPipelineDependancy* dependancy, const Allocaters* allocs)
{
	for (int i = 0; i < 5; i++)
		ReleaseShaderDependancy(dependancy->dependants + i, allocs);

	return S_OK;
}
HRESULT ReleaseComputeDependancy(DX11ComputePipelineDependancy* dependancy, const Allocaters* allocs)
{
	ReleaseShaderDependancy(&dependancy->resources, allocs);
	return S_OK;
}
HRESULT ReleaseCopyDependancy(DX11CopyDependancy* dependancy, const Allocaters* allocs)
{
	return S_OK;
}
HRESULT ReleaseContext(RenderContextState* context, const Allocaters* allocs)
{
	SAFE_DEALLOC(context->bufferPtrBuffer, allocs->dealloc);
	SAFE_DEALLOC(context->numberBuffer, allocs->dealloc);
	return S_OK;
}
