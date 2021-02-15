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
#include "dx11depend.h"
#include "dx11resdesc.h"
#include "dx11res.h"
#include "renderinstance.h"

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

DX11Resources g_ExternalResources;
DX11PipelineDependancySet g_DepSet;
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
g_ObjectPos = Vector4f(0.0f, 0, 0.0f, 0.0f);

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

	// resize refresh data
	ExecuteExplicitlyDX11(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DepSet.resizeDependancyCount, g_DepSet.resizeDependancy);

	return hr;
}

HRESULT RenderResourceInit(bool debug)
{
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
		const DX11Resources* res = &g_ExternalResources;
		const DX11Resources::SkinningInstance* skinI = res->skinningInstances + index;
		const DX11Resources::BoneSet* boneSet = res->boneSets + res->anims[skinI->animationIndex].boneSetIndex;
		const DX11Resources::Animation* anim = res->anims + skinI->animationIndex;
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

	ShaderCompileDesc vsd = { L"./object_srv.hlsl", "vertex", "vs_5_0" };
	ShaderParams vsParams[4];
	new (vsParams + 0) ShaderParams(
		0, L"Initialize", sizeof(ImmutableCB), UpdateFrequency::OnlyOnce, UpdateImmutableCB, false
	);
	new (vsParams + 1) ShaderParams(
		1, L"OnResize", sizeof(OnResizeCB), UpdateFrequency::OnResize, UpdaterResizeCB, false
	);
	new (vsParams + 2) ShaderParams(
		2, L"OnFrame", sizeof(OnFrameCB), UpdateFrequency::PerFrame, UpdateFrameCB, false
	);
	new (vsParams + 3) ShaderParams(
		0, ExistSRVKind::DeformedVertexBufferForSkin
	);

	renderInstances[0].vs.sd = vsd;
	renderInstances[0].vs.paramCount = ARRAYSIZE(vsParams);
	renderInstances[0].vs.params = vsParams;

	ShaderParamTex2DSRV tex2D[2] = { { L"./char_max.png" }, { L"./face_00.png" } };
	ShaderParams psParams[3];
	new (psParams + 0) ShaderParams(
		2, L"OnFrame", sizeof(OnFrameCB), UpdateFrequency::PerFrame, UpdateFrameCB, false
	);
	psParams[1].slotIndex = 1;
	psParams[1].kind = ShaderParamKind::Texture2DSRV;
	psParams[1].tex2DSRV = tex2D[0];
	psParams[2].slotIndex = 0;
	psParams[2].kind = ShaderParamKind::SamplerState;
	psParams[2].sampler.isLinear = true;

	ShaderCompileDesc psd = { L"./object_srv.hlsl", "pixel", "ps_5_0" };

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
			g_D3D11Device, ARRAYSIZE(renderInstances), renderInstances, 
			&g_ExternalResources, &buffer2, &g_DepSet
		),
		L"fail to load resource & dependacies.."
	);

	// context prepare
	DependancyContextStatePrepare(&g_ContextState, &g_DepSet);

	// init refresh data
	ExecuteExplicitlyDX11(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DepSet.initDependancyCount, g_DepSet.initDependancy);
	ExecuteExplicitlyDX11(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DepSet.resizeDependancyCount, g_DepSet.resizeDependancy);

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
#if defined _DEBUG | DEBUG
	ID3D11Debug *dbg;
	g_D3D11Device->QueryInterface(IID_ID3D11Debug, (void**)(&dbg));
	if (dbg)
	{
		dbg->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		dbg->Release();
	}
#endif

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

	ReleaseResources(&g_ExternalResources);
	ReleaseContext(&g_ContextState);

	ReleaseDX11Dependancy(&g_DepSet);
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
	
	ExecuteExplicitlyDX11(g_D3D11ImmediateContext, &g_ContextState, &g_ExternalResources, g_DepSet.frameDependancyCount, g_DepSet.frameDependancy);

	g_DXGISwapChain->Present(0, 0);
}
