#include "WindowsMessageProcessing.h"

LRESULT EngineWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
			return 0;
	}
	//交给windows自己完成默认的消息处理
	return DefWindowProc(hWnd, message, wParam, lParam);
}
