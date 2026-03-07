#include "EngineMinimal.h"
#include "EngineFactory.h"
#include "Debug/Log/SimpleLog.h"

int Init(FEngine* InEngine, HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR c, int nCmdShow)
{
	FWinMainCommandParameters CommandParameters(hInstance, hPrevInstance, c, nCmdShow);
	int ReturnValue = InEngine->PreInit(
#if defined(_WIN32)
		CommandParameters
#endif	
	);
	if (ReturnValue != 0)
	{
		Engine_Log_Error("PreInit Failed with error code [%i]", ReturnValue);
		return ReturnValue;
	}

	ReturnValue = InEngine->Init();
	if (ReturnValue != 0)
	{
		Engine_Log_Error("Init Failed with error code [%i]", ReturnValue);
		return ReturnValue;
	}

	ReturnValue = InEngine->PostInit();
	if (ReturnValue != 0)
	{
		Engine_Log_Error("PostInit Failed with error code [%i]", ReturnValue);
		return ReturnValue;
	}
}

void Tick(FEngine* InEngine)
{
	InEngine->Tick();
}

int Exit(FEngine* InEngine)
{
	int ReturnValue = InEngine->PreExit();
	if (ReturnValue != 0)
	{
		Engine_Log_Error("PreExit Failed with error code [%i]", ReturnValue);
		return ReturnValue;
	}
	ReturnValue = InEngine->Exit();
	if (ReturnValue != 0)
	{
		Engine_Log_Error("Exit Failed with error code [%i]", ReturnValue);
		return ReturnValue;
	}
	ReturnValue = InEngine->PostExit();
	if (ReturnValue != 0)
	{
		Engine_Log_Error("PostExit Failed with error code [%i]", ReturnValue);
		return ReturnValue;
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR c, int nCmdShow)
{
	int ReturnValue = 0;
	if (FEngine* Engine = FEngineFactory::CreateEngine())
	{
		//≥ı ºªØ
		ReturnValue = Init(Engine, hInstance, hPrevInstance, c, nCmdShow);
		//‰÷»æ
		while (true)
		{
			Tick(Engine);
		}
		//ÕÀ≥ˆ
		ReturnValue = Exit(Engine);
	}
	else
	{
		ReturnValue = 1;
	}
	Engine_Log("Engine exited with code [%i]", ReturnValue);
	return ReturnValue;
	
}