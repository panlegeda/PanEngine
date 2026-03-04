//#include "EngineMinimal.h"
#include <Windows.h>
#include "EngineFactory.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (FEngine* Engine = FEngineFactory::CreateEngine())
	{
		Engine->PreExit();

		Engine->Init();

		Engine->PostInit();

		while (true)
		{
			Engine->Tick();
		}

		Engine->PreExit();
		Engine->Exit();
		Engine->PostExit();

		return 0;
	}
	return 1;
	
}