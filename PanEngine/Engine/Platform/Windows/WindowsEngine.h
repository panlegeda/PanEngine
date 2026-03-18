#pragma once
#if defined(_WIN32)
#include "../../Core/Engine.h"

class FWindowsEngine : public FEngine
{
public:
	virtual int PreInit(FWinMainCommandParameters InParams);
	virtual int Init();
	virtual int PostInit();

	virtual void Tick();
	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

private:
	bool InitWindows(const FWinMainCommandParameters &InParams);
	bool InitDirect3D();
protected:
	ComPtr<IDXGIFactory4> DXGIFactory;
	ComPtr<ID3D12Device> D3DDevice;
	ComPtr<ID3D12Fence> Fence;//同步一个cpu和多个gpu线程

	ComPtr<ID3D12CommandQueue> CommandQueue;//队列
	ComPtr<ID3D12CommandAllocator> CommandAllocator;//存储
	ComPtr<ID3D12GraphicsCommandList> CommandList;//命令列表

	ComPtr<IDXGISwapChain> SwapChain;//交换链

protected:
	HWND MainWindowsHandle;//窗口句柄
};
#endif