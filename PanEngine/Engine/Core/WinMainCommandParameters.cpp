#include "WinMainCommandParameters.h"

FWinMainCommandParameters::FWinMainCommandParameters(HINSTANCE InhInstance, HINSTANCE InhPrevInstance, LPSTR InlpCmdLine, int InnCmdShow)
	:HInstance(InhInstance), HPrevInstance(InhPrevInstance), LpCmdLine(InlpCmdLine), NCmdShow(InnCmdShow)
{

}
