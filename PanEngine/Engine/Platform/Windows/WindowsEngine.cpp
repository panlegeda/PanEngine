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
	//日志系统初始化
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	Engine_Log("Log Init");

	//处理命令

	
	return 0;
}

int FWindowsEngine::Init(FWinMainCommandParameters InParams)
{
	if (InitWindows(InParams))
	{
		Engine_Log("Init Windows complete");
	}

	if (InitDirect3D())
	{
		Engine_Log("Init Direct3D complete");
	}
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

	SwapChain->ResizeBuffers(
		FEngineRenderConfig::GetRenderConfig()->SwapChainCount,
		FEngineRenderConfig::GetRenderConfig()->ScreenWidth,
		FEngineRenderConfig::GetRenderConfig()->ScreenHeight,
		BufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	//拿到描述的size
	RTVDescriptorSize = D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();
	HeapHandle.ptr = 0;
	for (UINT i = 0; i < FEngineRenderConfig::GetRenderConfig()->SwapChainCount; i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
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
	//注册窗口
	WNDCLASSEX WindowClass;
	WindowClass.cbSize = sizeof(WNDCLASSEX);//该对象实际占用多大内存
	WindowClass.cbClsExtra = 0;//窗口类额外的内存
	WindowClass.cbWndExtra = 0;//窗口额外的内存
	WindowClass.hbrBackground = nullptr;//窗口背景画刷
	WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);//窗口光标
	WindowClass.hIcon = nullptr;//窗口图标
	WindowClass.hIconSm = NULL;//窗口小图标
	WindowClass.hInstance = InParams.HInstance;//窗口实例
	WindowClass.lpszClassName = L"PanPanEngine";//窗口名称
	WindowClass.lpszMenuName = nullptr;//窗口菜单
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;//窗口样式
	WindowClass.lpfnWndProc = EngineWindowProc;//窗口消息处理函数

	ATOM RegisterAtom = RegisterClassEx(&WindowClass);//注册窗口
	if (!RegisterAtom)
	{
		Engine_Log_Error("Register windows class failed");
		MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_OK);
	}

	RECT Rext = { 0, 0, FEngineRenderConfig::GetRenderConfig()->ScreenWidth, FEngineRenderConfig::GetRenderConfig()->ScreenHeight };

	//@rect 视口
	//WS_OVERLAPPEDWINDOW 视口风格
	//FALSE 是否有菜单
	AdjustWindowRect(&Rext, WS_OVERLAPPEDWINDOW, FALSE);//调整窗口大小

	int WindowWidth = Rext.right - Rext.left;
	int WindowHeight = Rext.bottom - Rext.top;

	MainWindowsHandle = CreateWindowEx(
		NULL,//窗口名称
		L"PanPanEngine",
		L"PanPanEngine",
		WS_OVERLAPPEDWINDOW,//窗口风格
		100, 100,//窗口位置
		WindowWidth, WindowHeight,
		nullptr, //副窗口句柄
		nullptr,//菜单句柄
		InParams.HInstance,//窗口实例
		nullptr);
	if(!MainWindowsHandle)
	{
		Engine_Log_Error("Init windows failed");
		MessageBox(nullptr, L"Failed to create window!", L"Error", MB_OK);
		return false;
	}
	ShowWindow(MainWindowsHandle, SW_SHOW);//显示窗口
	UpdateWindow(MainWindowsHandle);//更新窗口
	Engine_Log("Init windows complete");

	return true;
}
bool FWindowsEngine::InitDirect3D()
{
	//HRESULT
	//S_OK				OXOOOOOO00
	//E_UNEXPECTED		0x8000FFFF 意外的失败
	//E_NOTIMPL			0x80004001 未实现
	//E_OUTOFMEMORY		0x8007000E 内存不足
	//E_INVALIDARG		0x80070057 参数无效
	//E_FAIL			0x80004005 失败
	//E_ACCESSDENIED		0x80070005 访问被拒绝
	//E_HANDLE			0x80070006 句柄无效
	//E_ABORT			0x80004004 操作被中止
	//E_POINTER			0x80004003 指针无效
	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));
	
	//D3D_FEATURE_LEVEL_11_0 代表Direct3D 11.0功能级别
	//D3D_FEATURE_LEVEL_11_1 代表Direct3D 11.1功能级别
	HRESULT D3DDeviceResult = D3D12CreateDevice(
		nullptr, //默认适配器
		D3D_FEATURE_LEVEL_11_0, //最低功能级别
		IID_PPV_ARGS(&D3DDevice)
	);
	if (FAILED(D3DDeviceResult))
	{
		//wrap 高级光栅化平台
		ComPtr<IDXGIAdapter> WarpAdapter;
		ANALYSIS_HRESULT(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));
		ANALYSIS_HRESULT(D3D12CreateDevice(
			WarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&D3DDevice)
		));
	
	}
	

	ANALYSIS_HRESULT(D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	//初始化命令对象
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//直接命令列表
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//命令队列标志
	ANALYSIS_HRESULT(D3DDevice->CreateCommandQueue(
		&QueueDesc,
		IID_PPV_ARGS(&CommandQueue)
	));


	ANALYSIS_HRESULT(D3DDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CommandAllocator)
	));
	ANALYSIS_HRESULT(D3DDevice->CreateCommandList(
		0,//默认单个gpu线程
		D3D12_COMMAND_LIST_TYPE_DIRECT,//直接类型
		CommandAllocator.Get(),//将command allocator与command list关联
		nullptr,//管线状态对象
		IID_PPV_ARGS(&CommandList)
	));

	CommandList->Close();//关闭命令列表

	//多重采样
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MultiSampleQualityLevels = {};
	MultiSampleQualityLevels.SampleCount = 4;//采样数量
	MultiSampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;//采样质量标志
	MultiSampleQualityLevels.NumQualityLevels = 0;//采样质量数量
	ANALYSIS_HRESULT(D3DDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&MultiSampleQualityLevels,
		sizeof(MultiSampleQualityLevels)
	));
	M4XNumQualityLevels = MultiSampleQualityLevels.NumQualityLevels;//采样质量数量

	//交换链
	SwapChain.Reset();
	//DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	//SwapChainDesc.BufferDesc.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;//交换链缓冲区宽度
	//SwapChainDesc.BufferDesc.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHeight;
	//SwapChainDesc.BufferDesc.RefreshRate.Numerator = FEngineRenderConfig::GetRenderConfig()->RefreshRate;
	//SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//SwapChainDesc.BufferCount = FEngineRenderConfig::GetRenderConfig()->SwapChainCount;//交换链缓冲区数量
	//SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;//交换链扫描线顺序
	//SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//交换链缓冲区使用方式
	//SwapChainDesc.OutputWindow = MainWindowsHandle;//交换链关联的窗口
	//SwapChainDesc.Windowed = TRUE;//是否窗口化
	////多重采样设置
	//SwapChainDesc.SampleDesc.Count = b4XMSAAEnabled? 4:1;//交换链采样数量
	//SwapChainDesc.SampleDesc.Quality = b4XMSAAEnabled ? M4XNumQualityLevels-1 : 0;//交换链采样质量
	//SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;//交换链交换效果
	//SwapChainDesc.Flags = 0;//交换链标志
	//SwapChainDesc.BufferDesc.Format = BufferFormat;//格式纹理

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 1280;
	sd.BufferDesc.Height = 720;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = MainWindowsHandle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // 旧版兼容模式
	sd.Flags = 0;
	
	HRESULT hr = DXGIFactory->CreateSwapChain(
		CommandQueue.Get(),//交换链关联的命令队列
		&sd,
		&SwapChain
	);

	if(hr==DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HRESULT DeviceRemovedReason = D3DDevice->GetDeviceRemovedReason();
		wchar_t ErrorMessage[256];
		swprintf_s(ErrorMessage, L"Device removed or reset! Reason: 0x%X", DeviceRemovedReason);
		OutputDebugStringW(ErrorMessage);
	}	

	//ANALYSIS_HRESULT(DXGIFactory->CreateSwapChain(
	//	CommandQueue.Get(),//交换链关联的命令队列
	//	&SwapChainDesc,
	//	&SwapChain
	//));

	//资源描述符
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc = {};
	RTVDescriptorHeapDesc.NumDescriptors = FEngineRenderConfig::GetRenderConfig()->SwapChainCount;//描述符数量
	RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//渲染目标视图描述符堆
	RTVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//描述符堆标志
	RTVDescriptorHeapDesc.NodeMask = 0;//默认单个gpu线程
	ANALYSIS_HRESULT(D3DDevice->CreateDescriptorHeap(
		&RTVDescriptorHeapDesc, 
		IID_PPV_ARGS(&RTVHeap)));

	//DSV
	D3D12_DESCRIPTOR_HEAP_DESC DSVDescriptorHeapDesc = {};
	DSVDescriptorHeapDesc.NumDescriptors = 1;//描述符数量
	DSVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//渲染目标视图描述符堆
	DSVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//描述符堆标志
	DSVDescriptorHeapDesc.NodeMask = 0;//默认单个gpu线程
	ANALYSIS_HRESULT(D3DDevice->CreateDescriptorHeap(
		&DSVDescriptorHeapDesc, 
		IID_PPV_ARGS(&DSVHeap)));

	return false;
}
#endif