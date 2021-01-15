#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <vector>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <mutex>
#include <pix3.h>
#include <dinput.h>
#include <functional>

using namespace DirectX;

#include "defined.h"
#include "dxentry.h"
#include "dx11util.h"
#include "renderres.h"
#include "dx11depend.h"

// windonw instances
HWND g_hWnd;
HINSTANCE g_hInstance;

// DXGI objects
IDXGIFactory* g_DXGIFactory;
IDXGISwapChain* g_DXGISwapChain;

/* D3D11 device objects */
ID3D11Device* g_D3D11Device;
ID3D11DeviceContext* g_D3D11ImmediateContext;
bool g_IsHDR;
UINT g_MaxFrameRate;
ID3D11RenderTargetView* g_D3D11RenderTargetView;
ID3D11Texture2D* g_D3D11DepthStencilTexture;
ID3D11DepthStencilView* g_D3D11DepthStencialView;
std::mutex g_ContextMutex;

IDirectInput8* g_Input;
IDirectInputDevice8* g_KeyInputDevice;
unsigned char g_KeyState[256];

/* D3D11 shader resources */
D3D11_VIEWPORT g_D3D11ViewPort;
XMMATRIX g_World;
XMMATRIX g_View;
XMMATRIX g_Projection;

GeneralAllocater g_GeneralAllocater;
RenderResources g_ExternalResources;
uint g_DependancyCount;
DX11PipelineDependancy* g_Dependancies;
RenderContextState g_ContextState;

int g_FrameCount = 0;
DWORD g_StartTick = GetTickCount();
double g_AvgFrameTime = 0.f;

// TODO:: DX OBJECT LEAK

_declspec(align(16)) struct ImmutableCB
{
	Vector4f position;
};

_declspec(align(16)) struct OnResizeCB
{
	DirectX::XMMATRIX projectionMatrix;
};

_declspec(align(16)) struct OnFrameCB
{
	DirectX::XMMATRIX transfromMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX worldMatrix;
	Vector4f view;
	Vector4f baseColor;
	Vector4f tintColor;
	Vector4f lightDir;
};

_declspec(align(16)) struct SkinningConfigCB
{
	uint vertexCount;
	uint poseOffset;
};

HRESULT DXDeviceInit(UINT width, UINT height, UINT maxFrameRate, bool debug)
{
	HRESULT hr = S_OK;

	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&g_DXGIFactory));
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create DXGIFactory2..");

	D3D_FEATURE_LEVEL maxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	UINT createDeviceFlags = 0;
	if (debug)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
		featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		&g_D3D11Device, &maxSupportedFeatureLevel, &g_D3D11ImmediateContext);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create D3D11Device..");

	hr = CreateSwapChainInline(g_hWnd, g_DXGIFactory, g_D3D11Device, &g_DXGISwapChain, width, height, g_MaxFrameRate = maxFrameRate, g_IsHDR = false);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create SwapChain..");

	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to get buffer from swapchain..");

	hr = g_D3D11Device->CreateRenderTargetView(backBuffer, nullptr, &g_D3D11RenderTargetView);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create rendertargetview..");

	hr = CreateDepthStencilInline(g_D3D11Device, &g_D3D11DepthStencilTexture, &g_D3D11DepthStencialView, width, height);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create depth-stencil buffer and view..");

	g_D3D11ViewPort.Width = (FLOAT)width;
	g_D3D11ViewPort.Height = (FLOAT)height;
	g_D3D11ViewPort.MinDepth = 0.0f;
	g_D3D11ViewPort.MaxDepth = 1.0f;
	g_D3D11ViewPort.TopLeftX = 0;
	g_D3D11ViewPort.TopLeftY = 0;
	
	hr = DirectInput8Create(g_hInstance, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (void**)&g_Input, nullptr);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create direct input..");
	hr = g_Input->CreateDevice(GUID_SysKeyboard, &g_KeyInputDevice, nullptr);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create direct key input device..");
	hr = g_KeyInputDevice->SetDataFormat(&c_dfDIKeyboard);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to set data format as keyboard..");
	hr = g_KeyInputDevice->Acquire();
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to aquired from keyboard..");

	return hr;
}

Vector4f
g_CurrentPos = Vector4f(-100.0f, 75.f, 150.0f, 0.0f),
g_ObjectPos = Vector4f(0.0f, 25.f, 0.0f, 0.0f);

void UpdateFrameCB(void* ptr, void* ref)
{
	DirectX::FXMVECTOR
		scale = XMVectorSet(0.2f, 0.2f, 0.2f, 0.2f),
		rotationOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		rotationQuat = XMVectorSet(0, 0, 0, 1),
		translate = XMVectorSet(0, 0, 0, 1);
	g_World = DirectX::XMMatrixAffineTransformation(
		scale, rotationOrigin, rotationQuat, translate
	);

	static float time = 0.f;
	static DWORD startTime = GetTickCount();
	DWORD currentTime = GetTickCount();
	time = (currentTime - startTime) / 1000.0f;

	XMVECTOR
		eye = XMLoadFloat4((XMFLOAT4*)&g_CurrentPos),
		at = XMLoadFloat4((XMFLOAT4*)&g_ObjectPos),
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);

	OnFrameCB* cbPtr = (OnFrameCB*)ptr;
	cbPtr->transfromMatrix = g_World * viewMatrix * g_Projection;
	cbPtr->viewMatrix = viewMatrix;
	cbPtr->worldMatrix = g_World;
	cbPtr->view = (g_ObjectPos - g_CurrentPos).normalized();
	cbPtr->baseColor = Vector4f(1, 0, 0, 0);
	cbPtr->tintColor = Vector4f::One();
	cbPtr->lightDir = Vector4f(1, 1, 1, 0).normalized();
}

void UpdateImmutableCB(void* ptr, void* ref)
{
	ImmutableCB* immutableCB = reinterpret_cast<ImmutableCB*>(ptr);
	immutableCB->position = Vector4f();
}

void UpdaterResizeCB(void* ptr, void* ref)
{
	OnResizeCB* resizeCB = reinterpret_cast<OnResizeCB*>(ptr);
	resizeCB->projectionMatrix = g_Projection = 
		DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, g_D3D11ViewPort.Width / g_D3D11ViewPort.Height, 0.01f, 1000000.0f);
}

// TODO:: naive resizing function..
HRESULT DXEntryResize(UINT width, UINT height)
{
	if (!g_D3D11ImmediateContext) return S_OK;

	std::lock_guard<std::mutex> lock(g_ContextMutex);

	HRESULT hr = S_OK;

	g_D3D11ImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	g_D3D11RenderTargetView->Release();
	g_DXGISwapChain->Release();

	hr = CreateSwapChainInline(g_hWnd, g_DXGIFactory, g_D3D11Device, &g_DXGISwapChain, width, height, g_MaxFrameRate, g_IsHDR);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create SwapChain..");

	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to get buffer from swapchain..");

	hr = g_D3D11Device->CreateRenderTargetView(backBuffer, nullptr, &g_D3D11RenderTargetView);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create resized RTV..");
	backBuffer->Release();

	g_D3D11DepthStencilTexture->Release();
	g_D3D11DepthStencialView->Release();

	hr = CreateDepthStencilInline(g_D3D11Device, &g_D3D11DepthStencilTexture, &g_D3D11DepthStencialView, width, height);
	FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create depth-stencil buffer and view..");

	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);

	g_D3D11ViewPort.Width = (FLOAT)width;
	g_D3D11ViewPort.Height = (FLOAT)height;
	g_D3D11ViewPort.MinDepth = 0.0f;
	g_D3D11ViewPort.MaxDepth = 1.0f;
	g_D3D11ViewPort.TopLeftX = 0;
	g_D3D11ViewPort.TopLeftY = 0;

	OnResizeCB resizeCB;
	UpdaterResizeCB(&resizeCB, nullptr);
	UploadDX11ConstantBuffer(&g_ExternalResources.dx11, g_D3D11ImmediateContext, 1, &resizeCB);

	return hr;
}

HRESULT PipelineDependancySet(RenderResources* res, uint* dependCount, DX11PipelineDependancy** depends, const Allocaters* allocs);

HRESULT RenderResourceInit(bool debug)
{
	//*/
	RenderInstance renderInstances[2];
	memset(renderInstances, 0, sizeof(renderInstances));

	renderInstances[0].isSkinDeform = true;
	renderInstances[0].shaderFlag = (ShaderFlags)(ShaderFlags::EnableVertex | ShaderFlags::EnablePixel);
	renderInstances[0].geometry.filePath = L"./char_max_triangulated.fbx";
	renderInstances[0].geometry.meshName = "Char_Max";
	renderInstances[0].anim.filePath = L"./char_max_triangulated.fbx";
	renderInstances[0].anim.animationName = "Take 001";

	ShaderParams computeParams[5];
	new (computeParams + 0) ShaderParams(0, ExistSRVKind::GeometryVertexBufferForSkin);
	new (computeParams + 1) ShaderParams(1, ExistSRVKind::BindPoseBufferForSkin);
	new (computeParams + 2) ShaderParams(2, ExistSRVKind::AnimationBufferForSkin);
	new (computeParams + 3) ShaderParams(0, ExistUAVKind::DeformedVertexBufferForSkin);
	std::function<void(void*, void*)> skinCBUpdate = [=](void* p, void* ref) -> void
	{
		int index = PtrToInt(ref);
		SkinningConfigCB* cb = static_cast<SkinningConfigCB*>(p);
		const RenderResources* res = &g_ExternalResources;
		const RenderResources::SkinningInstance* skinI = res->skinningInstances + index;
		const RenderResources::BoneSet* boneSet = res->boneSets + res->anims[skinI->animationIndex].boneSetIndex;
		const RenderResources::Animation* anim = res->anims + skinI->animationIndex;
		cb->vertexCount = res->geometryChunks[skinI->geometryIndex].vertexCount;
		cb->poseOffset =
			boneSet->boneCount * (((GetTickCount() - g_StartTick) / anim->fpsCount) % anim->frameKeyCount);
	};
	new (computeParams + 4) ShaderParams(
		0, L"SkinningConfigCB", sizeof(SkinningConfigCB), UpdateFrequency::PerFrame,
		ShaderParamCB::ExistParam::SkinningInstanceIndex, skinCBUpdate, true
	);

	renderInstances[0].skinCSParam.sd = { L"./lbs_compute.hlsl", "lbs", "cs_5_0" };
	renderInstances[0].skinCSParam.paramCount = ARRAYSIZE(computeParams);
	renderInstances[0].skinCSParam.params = computeParams;

	ShaderCompileDesc vsd = { L"./object.hlsl", "vertex", "vs_5_0" };
	ShaderParams vsParams[3];
	new (vsParams + 0) ShaderParams(
		0, L"Initialize", sizeof(ImmutableCB), UpdateFrequency::OnlyOnce, UpdateImmutableCB, false
	);
	new (vsParams + 1) ShaderParams(
		1, L"OnResize", sizeof(OnResizeCB), UpdateFrequency::OnResize, UpdaterResizeCB, false
	);
	new (vsParams + 2) ShaderParams(
		2, L"OnFrame", sizeof(OnFrameCB), UpdateFrequency::PerFrame, UpdateFrameCB, false
	);

	renderInstances[0].vs.sd = vsd;
	renderInstances[0].vs.paramCount = ARRAYSIZE(vsParams);
	renderInstances[0].vs.params = vsParams;

	ShaderParamTex2DSRV tex2D[2] = { { L"./char_max.png" }, { L"./face_00.png" } };
	ShaderParams psParams[3];
	new (psParams + 0) ShaderParams(
		2, L"OnFrame", sizeof(OnFrameCB), UpdateFrequency::PerFrame, UpdateFrameCB, false
	);
	psParams[1].slotIndex = 0;
	psParams[1].kind = ShaderParamKind::Texture2DSRV;
	psParams[1].tex2DSRV = tex2D[0];
	psParams[2].slotIndex = 0;
	psParams[2].kind = ShaderParamKind::SamplerState;
	psParams[2].sampler.isLinear = true;

	ShaderCompileDesc psd = { L"./object.hlsl", "pixel", "ps_5_0" };

	renderInstances[0].ps.sd = psd;
	renderInstances[0].ps.paramCount = ARRAYSIZE(psParams);
	renderInstances[0].ps.params = psParams;

	ShaderParams psParams2[3];
	memcpy(psParams2, psParams, sizeof(psParams));
	psParams2[1].tex2DSRV = tex2D[1];

	renderInstances[1] = renderInstances[0];
	renderInstances[1].geometry.meshName = "Face";
	renderInstances[1].ps.paramCount = ARRAYSIZE(psParams2);
	renderInstances[1].ps.params = psParams2;

	DX11InternalResourceDescBuffer buffer2;
	FAILED_ERROR_MESSAGE_RETURN(
		LoadResourceAndDependancyFromInstance(
			g_D3D11Device, &g_GeneralAllocater.persistant, ARRAYSIZE(renderInstances), renderInstances, 
			&g_ExternalResources, &buffer2, &g_DependancyCount, &g_Dependancies
		),
		L"fail to load resource & dependacies.."
	);

	/*/

	// load dx11 object 
	// external resource depandency
	FBXChunk c;
	memset(&c, 0, sizeof(FBXChunk));
	FALSE_ERROR_MESSAGE_RETURN_CODE(
		ImportFBX(L"./char_max_triangulated.fbx", c, nullptr, &g_GeneralAllocater.persistant), 
		L"fail to load fbxfile as \"ImportFBX\"", 
		E_FAIL
	);

	FBXChunkConfig::FBXMeshConfig meshConfigs[] = { true, true };
	FBXChunkConfig config { meshConfigs };

	const wchar_t* textureName[] = { L"./char_max.png", L"./face_00.png" };
	ShaderCompileDesc shaderDescArray[] = { { L"./object.hlsl", "vertex", "vs_5_0" }, { L"./object.hlsl", "pixel", "ps_5_0" }, { L"./lbs_compute.hlsl", "lbs", "cs_5_0" } };
	DX11InputLayoutDesc inputLayoutDescArray[] = { { 0, 0 } };
	D3D11_SAMPLER_DESC samplerDescs[] = { 
		{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, 
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			0,
			0,
			D3D11_COMPARISON_NEVER,
			{ 0, 0, 0, 0 },
			0,
			0
		} 
	};
	uint cbSize[] = { sizeof(ImmutableCB), sizeof(OnResizeCB), sizeof(OnFrameCB), sizeof(SkinningConfigCB) };
	SkinningInstanceDesc instances[] = { {0, 0}, {1, 0} };

	DX11ResourceDesc resDesc = {
		1, &c, &config,
		ARRAYSIZE(shaderDescArray), shaderDescArray,
		ARRAYSIZE(inputLayoutDescArray), inputLayoutDescArray,
		ARRAYSIZE(textureName), textureName,
		ARRAYSIZE(cbSize), cbSize,
		ARRAYSIZE(samplerDescs), samplerDescs,
		ARRAYSIZE(instances), instances
	};

	DX11InternalResourceDescBuffer buffer;
	FAILED_ERROR_MESSAGE_RETURN(
		LoadDX11Resoureces(
			&g_ExternalResources, &buffer, &resDesc, &g_GeneralAllocater.persistant, g_D3D11Device
		),
		L"fail to load dx11 resources.."
	);

	// pipeline dependancy from resources
	//*/

	//for (uint i = 0; i < g_DependancyCount; i++)
	//{
	//	std::cout << "> dep" << i << std::endl;
	//	PrintPipelineDependancy(">", g_Dependancies[i]);
	//}
	//PipelineDependancySet(&g_ExternalResources, &g_DependancyCount, &g_Dependancies, &g_GeneralAllocater.persistant);
	//for (uint i = 0; i < g_DependancyCount; i++)
	//{
	//	std::cout << "> dep" << i << std::endl;
	//	PrintPipelineDependancy(">", g_Dependancies[i]);
	//}

	// context prepare
	DependancyContextStatePrepare(&g_ContextState, &g_GeneralAllocater.persistant, g_DependancyCount, g_Dependancies);

	// upload constant buffer
	ImmutableCB immutableCB{ Vector4f() };
	UploadDX11ConstantBuffer(&g_ExternalResources.dx11, g_D3D11ImmediateContext, 0, &immutableCB);
	OnResizeCB resizeCB{
		g_Projection = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, g_D3D11ViewPort.Width / g_D3D11ViewPort.Height, 0.01f, 100000.0f)
	};
	UploadDX11ConstantBuffer(&g_ExternalResources.dx11, g_D3D11ImmediateContext, 1, &resizeCB);

	return S_OK;
}

HRESULT DXEntryInit(HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, UINT maxFrameRate, bool debug)
{
	DWORD startMilliSecond = GetTickCount();
	HRESULT hr = S_OK;
	{
		g_hWnd = hWnd;
		g_hInstance = hInstance;

		hr = DXDeviceInit(width, height, maxFrameRate, debug);
		FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to initialize device..");
		hr = RenderResourceInit(debug);
		FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create resource view..");
	}
	printf("intialize time : %dms\n", GetTickCount()- startMilliSecond);

	return hr;
}

void DXEntryClean()
{
	if (g_D3D11ImmediateContext) g_D3D11ImmediateContext->ClearState();

	if (g_D3D11RenderTargetView) g_D3D11RenderTargetView->Release();
	if (g_DXGISwapChain) g_DXGISwapChain->Release();
	if (g_D3D11ImmediateContext) g_D3D11ImmediateContext->Release();
	if (g_D3D11Device) g_D3D11Device->Release();

	if (g_KeyInputDevice) 
	{
		g_KeyInputDevice->Unacquire();
		g_KeyInputDevice->Release();
		g_KeyInputDevice = nullptr;
	}

	if (g_Input)
	{
		g_Input->Release();
		g_Input = nullptr;
	}

	ReleaseResources(&g_ExternalResources, &g_GeneralAllocater.persistant);
	ReleaseContext(&g_ContextState, &g_GeneralAllocater.persistant);

	if (g_Dependancies)
		ReleaseDX11Dependancy(g_DependancyCount, g_Dependancies, &g_GeneralAllocater.persistant);
}
void ReadKey();
void Render();
void RenderDependancy();

void DXEntryFrameUpdate()
{
	g_FrameCount++;
	auto start = std::chrono::high_resolution_clock::now();
	{
		ReadKey();

		if (g_KeyState[DIK_SPACE] & 0x80)
			Render();
		else
			RenderDependancy();
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	double tempTime = microseconds / 1000.0;
	g_AvgFrameTime = g_AvgFrameTime * (g_FrameCount - 1) / g_FrameCount + tempTime / g_FrameCount;
}

void ReadKey()
{
	HRESULT hr = g_KeyInputDevice->GetDeviceState(sizeof(g_KeyState), &g_KeyState);

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			g_KeyInputDevice->Acquire();
	}

	const float step = 0.1f;

	if (g_KeyState[DIK_W] & 0x80)
		g_CurrentPos.z += step;
	else if (g_KeyState[DIK_S] & 0x80)
		g_CurrentPos.z -= step;

	if (g_KeyState[DIK_A] & 0x80)
		g_CurrentPos.x += step;
	else if (g_KeyState[DIK_D] & 0x80)
		g_CurrentPos.x -= step;

	if (g_KeyState[DIK_Q] & 0x80)
		g_CurrentPos.y += step;
	else if (g_KeyState[DIK_E] & 0x80)
		g_CurrentPos.y -= step;
}

void RenderDependancy()
{
	std::lock_guard<std::mutex> lock(g_ContextMutex);

	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_STENCIL, 0.0f, 0);
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_D3D11ImmediateContext->ClearRenderTargetView(g_D3D11RenderTargetView, clearColor);

	g_D3D11ImmediateContext->RSSetViewports(1, &g_D3D11ViewPort);
	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);
	
	ExecuteExplicitlyDX11(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DependancyCount, g_Dependancies);

	g_DXGISwapChain->Present(0, 0);
}

using namespace std;

void Render()
{
	lock_guard<std::mutex> lock(g_ContextMutex);

	const RenderResources* res = &g_ExternalResources;
	const DX11Resources* dx11Res = &res->dx11;

	// upload constant buffer
	{
		ImmutableCB immutableCB{ Vector4f() };
		UploadDX11ConstantBuffer(&g_ExternalResources.dx11, g_D3D11ImmediateContext, 0, &immutableCB);
		OnResizeCB resizeCB{
			g_Projection = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, g_D3D11ViewPort.Width / g_D3D11ViewPort.Height, 0.01f, 100000.0f)
		};
		UploadDX11ConstantBuffer(&g_ExternalResources.dx11, g_D3D11ImmediateContext, 1, &resizeCB);
		OnFrameCB cb;
		UpdateFrameCB(&cb, nullptr);
		UploadDX11ConstantBuffer(&g_ExternalResources.dx11, g_D3D11ImmediateContext, 2, &cb);

	}
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_D3D11ImmediateContext->ClearDepthStencilView(g_D3D11DepthStencialView, D3D11_CLEAR_STENCIL, 0.0f, 0);
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_D3D11ImmediateContext->ClearRenderTargetView(g_D3D11RenderTargetView, clearColor);

	g_D3D11ImmediateContext->RSSetViewports(1, &g_D3D11ViewPort);
	g_D3D11ImmediateContext->OMSetRenderTargets(1, &g_D3D11RenderTargetView, g_D3D11DepthStencialView);

	auto immutableCB = dx11Res->buffers[dx11Res->constantBufferIndices[0]];
	auto resizeCB = dx11Res->buffers[dx11Res->constantBufferIndices[1]];
	auto frameCB = dx11Res->buffers[dx11Res->constantBufferIndices[2]];
	auto sampler = dx11Res->samplerStates[0];

	auto objectShaderFile = g_ExternalResources.shaderFiles[0];
	auto vertexShader = dx11Res->shaders.vss[objectShaderFile.vsIndices[0]];
	auto pixelShader = dx11Res->shaders.pss[objectShaderFile.psIndices[0]];

	auto anim = g_ExternalResources.anims[0];
	auto boneSet = g_ExternalResources.boneSets[0];
	auto boneBindPoseSRV = dx11Res->srvs[boneSet.binePoseTransformSRVIndex];
	auto bonePoseSRV = dx11Res->srvs[anim.animPoseTransformSRVIndex];

	for (uint i = 0; i < g_ExternalResources.skinningCount; i++)
	{
		auto skinningConfigCB = dx11Res->buffers[dx11Res->constantBufferIndices[3 + i]];
		auto skinningInstance = g_ExternalResources.skinningInstances[i];
		auto geometry = g_ExternalResources.geometryChunks[skinningInstance.geometryIndex];
		auto lbsCompute = dx11Res->shaders.css[0];
		auto skinningVertexData = dx11Res->srvs[geometry.vertexDataSRVIndex];
		auto skinningUAV = dx11Res->uavs[skinningInstance.vertexStreamUAVIndex];
		UINT offsets[2] = { 0, 0 };

		SkinningConfigCB cb;
		cb.vertexCount = geometry.vertexCount;
		cb.poseOffset = boneSet.boneCount * (((GetTickCount() - g_StartTick) / anim.fpsCount) % anim.frameKeyCount);

		g_D3D11ImmediateContext->UpdateSubresource(skinningConfigCB, 0, nullptr, &cb, 0, 0);

		g_D3D11ImmediateContext->CSSetShader(lbsCompute, nullptr, 0);
		g_D3D11ImmediateContext->CSSetConstantBuffers(0, 1, &skinningConfigCB);
		g_D3D11ImmediateContext->CSSetShaderResources(0, 1, &skinningVertexData);
		g_D3D11ImmediateContext->CSSetShaderResources(1, 1, &boneBindPoseSRV);
		g_D3D11ImmediateContext->CSSetShaderResources(2, 1, &bonePoseSRV);
		g_D3D11ImmediateContext->CSSetUnorderedAccessViews(0, 1, &skinningUAV, offsets);

		g_D3D11ImmediateContext->Dispatch(geometry.vertexCount, 1, 1);

		g_D3D11ImmediateContext->CopyResource(
			dx11Res->buffers[skinningInstance.vertexBufferIndex],
			dx11Res->buffers[skinningInstance.vertexStreamBufferIndex]
			);

		auto geometryLayout = dx11Res->vertexLayouts[geometry.vertexLayoutIndex];
		auto inputLayout = dx11Res->inputLayoutItems[dx11Res->inputLayouts[0].inputLayoutIndex];
		auto texSRV = dx11Res->srvs[g_ExternalResources.shaderTex2Ds[i].srvIndex];

		g_D3D11ImmediateContext->IASetInputLayout(inputLayout);
		uint offset = 0;
		g_D3D11ImmediateContext->IASetVertexBuffers(0, 1, &dx11Res->buffers[skinningInstance.vertexBufferIndex], &geometryLayout.vertexSize, &offset);
		g_D3D11ImmediateContext->IASetIndexBuffer(dx11Res->buffers[geometry.indexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
		g_D3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_D3D11ImmediateContext->VSSetShader(vertexShader, nullptr, 0);
		g_D3D11ImmediateContext->VSSetConstantBuffers(0, 1, &immutableCB);
		g_D3D11ImmediateContext->VSSetConstantBuffers(1, 1, &resizeCB);
		g_D3D11ImmediateContext->VSSetConstantBuffers(2, 1, &frameCB);

		g_D3D11ImmediateContext->PSSetShader(pixelShader, nullptr, 0);
		g_D3D11ImmediateContext->PSSetConstantBuffers(2, 1, &frameCB);
		g_D3D11ImmediateContext->PSSetShaderResources(0, 1, &texSRV);
		g_D3D11ImmediateContext->PSSetSamplers(0, 1, &sampler);

		g_D3D11ImmediateContext->DrawIndexed(geometry.indexCount, 0, 0);
	}

	g_DXGISwapChain->Present(0, 0);
}

HRESULT PipelineDependancySet(RenderResources* res, uint* dependCount, DX11PipelineDependancy** depends, const Allocaters* allocs)
{
	const RenderResources::BoneSet& boneSet = res->boneSets[0];
	uint offset = 0;

	*dependCount = res->skinningCount * 4 + 1;
	*depends = (DX11PipelineDependancy*)allocs->alloc(sizeof(DX11PipelineDependancy) * g_DependancyCount);
	memset(*depends, 0, sizeof(DX11PipelineDependancy) * g_DependancyCount);

	{
		(*depends)[offset].pipelineKind = PipelineKind::Copy;
		DX11CopyDependancy& cpy = (*depends)[offset].copy;
		cpy.kind = CopyKind::UpdateSubResource;
		cpy.args.updateSubRes.resKind = ResourceKind::Buffer;
		cpy.args.updateSubRes.resIndex = res->dx11.constantBufferIndices[2];
		cpy.args.updateSubRes.dstSubres = 0;
		cpy.args.updateSubRes.dataBufferSize = sizeof(OnFrameCB);
		cpy.args.updateSubRes.copyToBufferFunc = UpdateFrameCB;
		cpy.args.updateSubRes.srcRowPitch = 0;
		cpy.args.updateSubRes.srcDepthPitch = 0;
		offset++;
	}

	for (uint i = 0; i < res->skinningCount; i++)
	{
		const RenderResources::SkinningInstance& skinningInstance = res->skinningInstances[i];
		const RenderResources::GeometryChunk& geomChunk = res->geometryChunks[skinningInstance.geometryIndex];

		{
			(*depends)[4 * i + 0 + offset].pipelineKind = PipelineKind::Copy;
			DX11CopyDependancy& cpy = ((*depends))[4 * i + 0 + offset].copy;
			cpy.kind = CopyKind::UpdateSubResource;
			cpy.args.updateSubRes.resIndex = res->dx11.constantBufferIndices[3 + i];
			cpy.args.updateSubRes.dstSubres = 0;
			cpy.args.updateSubRes.getBoxFunc = 
				[](D3D11_BOX* box) -> const D3D11_BOX* 
				{ 
					return (const D3D11_BOX*)nullptr; 
				};
			cpy.args.updateSubRes.dataBufferSize = sizeof(SkinningConfigCB);
			cpy.args.updateSubRes.copyToBufferFunc =
				[=](void* ptr, void* ref) -> void 
				{
					SkinningConfigCB* cb = reinterpret_cast<SkinningConfigCB*>(ptr);
					cb->vertexCount = geomChunk.vertexCount;
					cb->poseOffset = boneSet.boneCount * (((GetTickCount() - g_StartTick) / res->anims[0].fpsCount) % res->anims[0].frameKeyCount);
				};
			cpy.args.updateSubRes.srcRowPitch = 0;
			cpy.args.updateSubRes.srcDepthPitch = 0;
		}
		{
			(*depends)[4 * i + 1 + offset].pipelineKind = PipelineKind::Compute;
			DX11ComputePipelineDependancy& compute = ((*depends))[4 * i + 1 + offset].compute;
			DX11ShaderResourceDependancy& srd = compute.resources;

			srd.shaderFileIndex = 1;
			srd.shaderIndex = 0;

			srd.srvCount = 3;
			srd.srvs =
				(DX11ShaderResourceDependancy::DX11SRVRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * srd.srvCount
				);
			srd.srvs[0].indexCount = 1;
			srd.srvs[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
			srd.srvs[0].indices[0] = geomChunk.vertexDataSRVIndex;
			srd.srvs[0].slotOrRegister = 0;
			srd.srvs[1].indexCount = 1;
			srd.srvs[1].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
			srd.srvs[1].indices[0] = boneSet.binePoseTransformSRVIndex;
			srd.srvs[1].slotOrRegister = 1;
			srd.srvs[2].indexCount = 1;
			srd.srvs[2].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
			srd.srvs[2].indices[0] = res->anims[0].animPoseTransformSRVIndex;
			srd.srvs[2].slotOrRegister = 2;

			srd.uavCount = 1;
			srd.uavs =
				(DX11ShaderResourceDependancy::DX11UAVRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11UAVRef) * srd.uavCount
				);
			srd.uavs[0].indexCount = 1;
			srd.uavs[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
			srd.uavs[0].indices[0] = skinningInstance.vertexStreamUAVIndex;
			srd.uavs[0].slotOrRegister = 0;

			srd.constantBufferCount = 1;
			srd.constantBuffers =
				(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * srd.constantBufferCount
				);
			srd.constantBuffers[0].slotOrRegister = 0;
			srd.constantBuffers[0].indexCount = 1;
			srd.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint) * 1);
			srd.constantBuffers[0].indices[0] = 3 + i;
			srd.constantBuffers[0].slotOrRegister = 0;

			compute.dispatchType = DX11ComputePipelineDependancy::DispatchType::Dispatch;
			compute.argsAsDispatch.dispatch.threadGroupCountX = geomChunk.vertexCount;
			compute.argsAsDispatch.dispatch.threadGroupCountY = 1;
			compute.argsAsDispatch.dispatch.threadGroupCountZ = 1;
		}
		{
			(*depends)[4 * i + 2 + offset].pipelineKind = PipelineKind::Copy;
			DX11CopyDependancy& cpy = ((*depends))[4 * i + 2 + offset].copy;
			cpy.kind = CopyKind::CopyResource;
			cpy.args.copyRes.srcBufferIndex = skinningInstance.vertexStreamBufferIndex;
			cpy.args.copyRes.dstBufferIndex = skinningInstance.vertexBufferIndex;
		}
		{
			(*depends)[4 * i + 3 + offset].pipelineKind = PipelineKind::Draw;
			DX11DrawPipelineDependancy& draw = (*depends)[4 * i + 3 + offset].draw;
			new (&draw) DX11DrawPipelineDependancy();

			draw.input.geometryIndex = skinningInstance.geometryIndex;
			draw.input.vertexBufferIndex = geomChunk.isSkinned? skinningInstance.vertexBufferIndex: geomChunk.vertexBufferIndex;
			draw.input.vertexSize = res->dx11.vertexLayouts[geomChunk.vertexLayoutIndex].vertexSize;
			draw.input.inputLayoutIndex = res->dx11.inputLayouts[0].inputLayoutIndex;
			draw.input.vertexBufferOffset = 0;
			draw.input.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			draw.vs.shaderFileIndex = 0;
			draw.vs.shaderIndex = 0;
			draw.vs.constantBufferCount = 3;
			draw.vs.constantBuffers =
				(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * draw.vs.constantBufferCount
				);
			draw.vs.constantBuffers[0].slotOrRegister = 0;
			draw.vs.constantBuffers[0].indexCount = 1;
			draw.vs.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint));
			draw.vs.constantBuffers[0].indices[0] = 0;
			draw.vs.constantBuffers[1].slotOrRegister = 1;
			draw.vs.constantBuffers[1].indexCount = 1;
			draw.vs.constantBuffers[1].indices = (uint*)allocs->alloc(sizeof(uint));
			draw.vs.constantBuffers[1].indices[0] = 1;
			draw.vs.constantBuffers[2].slotOrRegister = 2;
			draw.vs.constantBuffers[2].indexCount = 1;
			draw.vs.constantBuffers[2].indices = (uint*)allocs->alloc(sizeof(uint));
			draw.vs.constantBuffers[2].indices[0] = 2;

			draw.ps.shaderFileIndex = 0;
			draw.ps.shaderIndex = 0;

			draw.ps.constantBufferCount = 1;
			draw.ps.constantBuffers =
				(DX11ShaderResourceDependancy::DX11ConstantBufferRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11ConstantBufferRef) * draw.ps.constantBufferCount
				);
			draw.ps.constantBuffers[0].slotOrRegister = 2;
			draw.ps.constantBuffers[0].indexCount = 1;
			draw.ps.constantBuffers[0].indices = (uint*)allocs->alloc(sizeof(uint));
			draw.ps.constantBuffers[0].indices[0] = 2;

			draw.ps.srvCount = 1;
			draw.ps.srvs =
				(DX11ShaderResourceDependancy::DX11SRVRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11SRVRef) * draw.ps.srvCount
				);
			draw.ps.srvs[0].slotOrRegister = 0;
			draw.ps.srvs[0].indexCount = 1;
			draw.ps.srvs[0].indices = (uint*)allocs->alloc(sizeof(uint));
			draw.ps.srvs[0].indices[0] = res->shaderTex2Ds[i].srvIndex;

			draw.ps.samplerCount = 1;
			draw.ps.samplers =
				(DX11ShaderResourceDependancy::DX11SamplerRef*)allocs->alloc(
					sizeof(DX11ShaderResourceDependancy::DX11SamplerRef) * draw.ps.samplerCount
				);
			draw.ps.samplers[0].slotOrRegister = 0;
			draw.ps.samplers[0].indexCount = 1;
			draw.ps.samplers[0].indices = (uint*)allocs->alloc(sizeof(uint));
			draw.ps.samplers[0].indices[0] = 0;

			draw.drawType = DX11DrawPipelineDependancy::DrawType::DrawIndexed;
			draw.argsAsDraw.drawIndexedArgs.indexCount = geomChunk.indexCount;
			draw.argsAsDraw.drawIndexedArgs.startIndexLocation = 0;
			draw.argsAsDraw.drawIndexedArgs.baseVertexLocation = 0;
		}
	}

	return S_OK;
}