#pragma once

class FEngine
{
public:
	virtual int PreInit();
	virtual int Init();
	virtual int PostInit();

	virtual void Tick();
	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();
};