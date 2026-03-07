#include "EngineMinimal.h"
#include "EngineFactory.h"
#include "Debug/Log/SimpleLog.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR c, int nCmdShow)
{
	int ReturnValue = 0;
	if (FEngine* Engine = FEngineFactory::CreateEngine())
	{
		FWinMainCommandParameters CommandParameters(hInstance, hPrevInstance, c, nCmdShow);
		ReturnValue=Engine->PreInit(
#if defined(_WIN32)
			CommandParameters
#endif	
		);
		if (ReturnValue != 0)
		{
			Engine_Log_Error("PreInit Failed with error code [%i]", ReturnValue);
			return ReturnValue;
		}

		ReturnValue = Engine->Init();
		if (ReturnValue !=  0)
		{
			Engine_Log_Error("Init Failed with error code [%i]", ReturnValue);
			return ReturnValue;
		}

		ReturnValue = Engine->PostInit();
		if (ReturnValue != 0)
		{
			Engine_Log_Error("PostInit Failed with error code [%i]", ReturnValue);
			return ReturnValue;
		}

		while (true)
		{
			Engine->Tick();
		}

		ReturnValue = Engine->PreExit();
		if (ReturnValue != 0)
		{
			Engine_Log_Error("PreExit Failed with error code [%i]", ReturnValue);
			return ReturnValue;
		}

		ReturnValue = Engine->Exit();
		if (ReturnValue != 0)
		{
			Engine_Log_Error("Exit Failed with error code [%i]", ReturnValue);
			return ReturnValue;
		}

		ReturnValue = Engine->PostExit();
		if (ReturnValue != 0)
		{
			Engine_Log_Error("PostExit Failed with error code [%i]", ReturnValue);
			return ReturnValue;
		}

		ReturnValue = 0;
	}
	else
	{
		ReturnValue = 1;
	}
	Engine_Log("Engine exited with code [%i]", ReturnValue);
	return ReturnValue;
	
}