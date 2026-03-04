#include "EngineFactory.h"
#include "Platform/Windows/WindowsEngine.h"

FEngineFactory::FEngineFactory()
{

}

FEngine* FEngineFactory::CreateEngine()
{
	return new FWindowsEngine();
}
