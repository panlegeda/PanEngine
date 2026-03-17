#include "WindowsEngine.h"
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
	return false;
}
#endif