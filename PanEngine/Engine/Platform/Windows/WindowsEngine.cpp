#include "WindowsEngine.h"
#include "../../Debug/EngineDebug.h"
#if defined(_WIN32)
#include "WindowsMessageProcessing.h"
#include "../../Config/EngineRenderConfig.h"

//class FVector
//{
//	unsigned char r;
//	unsigned char g;
//	unsigned char b;
//	unsigned char a;
//};

FWindowsEngine::FWindowsEngine()
	:M4XNumQualityLevels(0),
	b4XMSAAEnabled(FALSE),
	BufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
	DepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
{
	for(int i=0;i<FEngineRenderConfig::GetRenderConfig()->SwapChainCount;i++)
	{
		SwapChainBuffer.push_back(ComPtr<ID3D12Resource>());
	}

}
int FWindowsEngine::PreInit(FWinMainCommandParameters InParams)
{
	//??????????
	// Always place logs next to the executable: <exe_dir>\log
	char ModulePath[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(nullptr, ModulePath, MAX_PATH) == 0)
	{
		init_log_system("log");
	}
	else
	{
		char* LastSlash = strrchr(ModulePath, '\\');
		if (LastSlash != nullptr)
		{
			*LastSlash = '\0';
		}
		char LogPath[MAX_PATH] = { 0 };
		strcpy_s(LogPath, ModulePath);
		strcat_s(LogPath, "\\log");
		init_log_system(LogPath);
	}
	Engine_Log("Log Init");

	//????????

	
	return 0;
}

int FWindowsEngine::Init(FWinMainCommandParameters InParams)
{
	if (!InitWindows(InParams))
	{
		return -1;
	}
	Engine_Log("Init Windows complete");

	if (!InitDirect3D())
	{
		return -2;
	}
	Engine_Log("Init Direct3D complete");
	Engine_Log("Engine initialization complete");
	return 0;
}

int FWindowsEngine::PostInit()
{
	for(int i=0;i<FEngineRenderConfig::GetRenderConfig()->SwapChainCount;i++)
	{
		SwapChainBuffer[i].Reset();
	}
	DepthStencilBuffer.Reset();
	if (SwapChain)
	{
		SwapChain->ResizeBuffers(
			FEngineRenderConfig::GetRenderConfig()->SwapChainCount,
			FEngineRenderConfig::GetRenderConfig()->ScreenWidth,
			FEngineRenderConfig::GetRenderConfig()->ScreenHeight,
			BufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		);
	}
	

	//?????????size
	RTVDescriptorSize = D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();
	HeapHandle.ptr = 0;
	for (UINT i = 0; i < FEngineRenderConfig::GetRenderConfig()->SwapChainCount; i++)
	{
		if (SwapChain)
		{
			SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
		}
		D3DDevice->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, HeapHandle);
		HeapHandle.ptr += RTVDescriptorSize;
	}
	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC ResourceDesc = {};
	ResourceDesc.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;
	ResourceDesc.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHeight;
	ResourceDesc.Alignment = 0;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.SampleDesc.Count = b4XMSAAEnabled? 4 : 1 ;
	ResourceDesc.SampleDesc.Quality = b4XMSAAEnabled ? (M4XNumQualityLevels - 1) : 0;
	ResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	CD3DX12_CLEAR_VALUE ClearValue = {};
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;
	ClearValue.Format = DepthStencilFormat;

	D3DDevice->CreateCommittedResource(
		&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&ClearValue,
		IID_PPV_ARGS(&DepthStencilBuffer)
	);


	D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
	DSVDesc.Format = DepthStencilFormat;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
	D3DDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DSVDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());

	Engine_Log("Engine post initialization complete");
	return 0;
}

void FWindowsEngine::Tick()
{
}

int FWindowsEngine::PreExit()
{
	Engine_Log("Engine pre exit complete");
	return 0;
}

int FWindowsEngine::Exit()
{
	Engine_Log("Engine exit complete");
	return 0;
}

int FWindowsEngine::PostExit()
{
	FEngineRenderConfig::Destroy();
	Engine_Log("Engine post exit complete");
	return 0;
}
bool FWindowsEngine::InitWindows(const FWinMainCommandParameters& InParams)
{
	//?????
	WNDCLASSEX WindowClass;
	WindowClass.cbSize = sizeof(WNDCLASSEX);//????????????????
	WindowClass.cbClsExtra = 0;//?????????????
	WindowClass.cbWndExtra = 0;//???????????
	WindowClass.hbrBackground = nullptr;//??????????
	WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);//??????
	WindowClass.hIcon = nullptr;//???????
	WindowClass.hIconSm = NULL;//?????????
	WindowClass.hInstance = InParams.HInstance;//???????
	WindowClass.lpszClassName = L"PanPanEngine";//????????
	WindowClass.lpszMenuName = nullptr;//??????
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;//???????
	WindowClass.lpfnWndProc = EngineWindowProc;//???????????????

	ATOM RegisterAtom = RegisterClassEx(&WindowClass);//?????
	if (!RegisterAtom)
	{
		Engine_Log_Error("Register windows class failed");
		MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_OK);
	}

	RECT Rext = { 0, 0, FEngineRenderConfig::GetRenderConfig()->ScreenWidth, FEngineRenderConfig::GetRenderConfig()->ScreenHeight };

	//@rect ???
	//WS_OVERLAPPEDWINDOW ?????
	//FALSE ????????
	AdjustWindowRect(&Rext, WS_OVERLAPPEDWINDOW, FALSE);//???????????

	int WindowWidth = Rext.right - Rext.left;
	int WindowHeight = Rext.bottom - Rext.top;

	MainWindowsHandle = CreateWindowEx(
		NULL,//????????
		L"PanPanEngine",
		L"PanPanEngine",
		WS_OVERLAPPEDWINDOW,//??????
		100, 100,//????????
		WindowWidth, WindowHeight,
		nullptr, //????????
		nullptr,//??????
		InParams.HInstance,//???????
		nullptr);
	if(!MainWindowsHandle)
	{
		Engine_Log_Error("Init windows failed");
		MessageBox(nullptr, L"Failed to create window!", L"Error", MB_OK);
		return false;
	}
	ShowWindow(MainWindowsHandle, SW_SHOW);//???????
	UpdateWindow(MainWindowsHandle);//???????
	Engine_Log("Init windows complete");

	return true;
}
bool FWindowsEngine::InitDirect3D()
{
	//HRESULT
	//S_OK				OXOOOOOO00
	//E_UNEXPECTED		0x8000FFFF ????????
	//E_NOTIMPL			0x80004001 ?????
	//E_OUTOFMEMORY		0x8007000E ??????
	//E_INVALIDARG		0x80070057 ????????
	//E_FAIL			0x80004005 ???
	//E_ACCESSDENIED		0x80070005 ????????
	//E_HANDLE			0x80070006 ???????
	//E_ABORT			0x80004004 ?????????
	//E_POINTER			0x80004003 ???????
	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));
	
	//D3D_FEATURE_LEVEL_11_0 ????Direct3D 11.0???????
	//D3D_FEATURE_LEVEL_11_1 ????Direct3D 11.1???????
	HRESULT D3DDeviceResult = D3D12CreateDevice(
		nullptr, //?????????
		D3D_FEATURE_LEVEL_11_0, //?????????
		IID_PPV_ARGS(&D3DDevice)
	);
	if (FAILED(D3DDeviceResult))
	{
		//wrap ??????????
		ComPtr<IDXGIAdapter> WarpAdapter;
		ANALYSIS_HRESULT(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));
		ANALYSIS_HRESULT(D3D12CreateDevice(
			WarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&D3DDevice)
		));
	
	}
	

	ANALYSIS_HRESULT(D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	//????????????
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//???????????
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//??????????
	ANALYSIS_HRESULT(D3DDevice->CreateCommandQueue(
		&QueueDesc,
		IID_PPV_ARGS(&CommandQueue)
	));


	ANALYSIS_HRESULT(D3DDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CommandAllocator)
	));
	ANALYSIS_HRESULT(D3DDevice->CreateCommandList(
		0,//??????gpu???
		D3D12_COMMAND_LIST_TYPE_DIRECT,//???????
		CommandAllocator.Get(),//??command allocator??command list????
		nullptr,//??????????
		IID_PPV_ARGS(&CommandList)
	));

	CommandList->Close();//???????????

	//???????
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MultiSampleQualityLevels = {};
	MultiSampleQualityLevels.SampleCount = 4;//????????
	MultiSampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;//???????????
	MultiSampleQualityLevels.Format = BufferFormat;//??????????????
	MultiSampleQualityLevels.NumQualityLevels = 0;//????????????
	ANALYSIS_HRESULT(D3DDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&MultiSampleQualityLevels,
		sizeof(MultiSampleQualityLevels)
	));
	M4XNumQualityLevels = MultiSampleQualityLevels.NumQualityLevels;//????????????

	//??????
	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	SwapChainDesc.BufferDesc.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;//????????????????
	SwapChainDesc.BufferDesc.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHeight;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = FEngineRenderConfig::GetRenderConfig()->RefreshRate;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferCount = FEngineRenderConfig::GetRenderConfig()->SwapChainCount;//????????????????
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;//??????????????
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//??????????????????
	SwapChainDesc.OutputWindow = MainWindowsHandle;//???????????????
	SwapChainDesc.Windowed = TRUE;//?????
	//???????????
	SwapChainDesc.SampleDesc.Count = b4XMSAAEnabled? 4:1;//??????????????
	SwapChainDesc.SampleDesc.Quality = b4XMSAAEnabled ? M4XNumQualityLevels-1 : 0;//??????????????
	// D3D12 requires flip model. DISCARD can return DXGI_ERROR_INVALID_CALL.
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//??????????????
	SwapChainDesc.Flags = 0;//?????????
	SwapChainDesc.BufferDesc.Format = BufferFormat;//???????
	
	ANALYSIS_HRESULT(DXGIFactory->CreateSwapChain(
		CommandQueue.Get(),//???????????????????
		&SwapChainDesc,
		&SwapChain
	));

	//?????????
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc = {};
	RTVDescriptorHeapDesc.NumDescriptors = FEngineRenderConfig::GetRenderConfig()->SwapChainCount;//??????????
	RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//?????????????????
	RTVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//??????????
	RTVDescriptorHeapDesc.NodeMask = 0;//??????gpu???
	ANALYSIS_HRESULT(D3DDevice->CreateDescriptorHeap(
		&RTVDescriptorHeapDesc, 
		IID_PPV_ARGS(&RTVHeap)));

	//DSV
	D3D12_DESCRIPTOR_HEAP_DESC DSVDescriptorHeapDesc = {};
	DSVDescriptorHeapDesc.NumDescriptors = 1;//??????????
	DSVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//?????????????????
	DSVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//??????????
	DSVDescriptorHeapDesc.NodeMask = 0;//??????gpu???
	ANALYSIS_HRESULT(D3DDevice->CreateDescriptorHeap(
		&DSVDescriptorHeapDesc, 
		IID_PPV_ARGS(&DSVHeap)));

	return true;
}
#endif