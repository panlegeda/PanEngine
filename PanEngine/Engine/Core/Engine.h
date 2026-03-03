#pragma once

class FEngine
{
public:
	FEngine();
	~FEngine();
	void Run();
	virtual int PreInit();
	virtual int Init();
	virtual int PostInit();

	virtual void Tick();
	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();
};