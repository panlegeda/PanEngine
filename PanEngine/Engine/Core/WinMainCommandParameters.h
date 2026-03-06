#pragma once


#if defined(_WIN32)
#include "../EngineMinimal.h"
class FWinMainCommandParameters
{
public:
	FWinMainCommandParameters(HINSTANCE InhInstance, HINSTANCE InhPrevInstance, LPSTR InlpCmdLine, int InnCmdShow);
	HINSTANCE HInstance;
	HINSTANCE HPrevInstance;
	LPSTR LpCmdLine;
	int NCmdShow;

};
#elif defined(_linux__)
#endif // 0

