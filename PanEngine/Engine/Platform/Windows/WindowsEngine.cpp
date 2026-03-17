#include "WindowsEngine.h"
#include "../../Debug/EngineDebug.h"
#if defined(_WIN32)
#include "WindowsMessageProcessing.h"
int FWindowsEngine::PreInit(FWinMainCommandParameters InParams)
{
	//日志系统初始化
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	Engine_Log("Log Init");

	//处理命令

	if(InitWindows(InParams))
	{
		Engine_Log("Engine PreInit complete");
	}

	if(InitDirect3D())
	{
		Engine_Log("Init Direct3D complete");
	}

	
	return 0;
}

int FWindowsEngine::Init()
{
	Engine_Log("Engine initialization complete");
	return 0;
}

int FWindowsEngine::PostInit()
{
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

	RECT Rext = { 0, 0, 1280, 720 };

	//@rect 视口
	//WS_OVERLAPPEDWINDOW 视口风格
	//FALSE 是否有菜单
	AdjustWindowRect(&Rext, WS_OVERLAPPEDWINDOW, FALSE);//调整窗口大小

	int WindowWidth = Rext.right - Rext.left;
	int WindowHeight = Rext.bottom - Rext.top;

	HWND hWnd = CreateWindowEx(
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
	if(!hWnd)
	{
		Engine_Log_Error("Init windows failed");
		MessageBox(nullptr, L"Failed to create window!", L"Error", MB_OK);
		return false;
	}
	ShowWindow(hWnd, SW_SHOW);//显示窗口
	UpdateWindow(hWnd);//更新窗口
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
		IID_PPV_ARGS(CommandList.GetAddressOf())
	));

	CommandList->Close();//关闭命令列表
	return false;
}
#endif