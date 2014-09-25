#ifndef _DEBUGSCRIPT_H
#define _DEBUGSCRIPT_H

#include "real.h"
#include "view.h"
#include "screen.h"
#include <iostream>
#include <fstream>

namespace IPDF
{

class DebugScript
{
public:
	void Load(const char *filename)
	{
		inp.open(filename);
	}
	bool Execute(View *view, Screen *scr);
private:
	enum ActionType
	{
		AT_WaitFrame,
		AT_Translate,
		AT_Zoom,
		AT_TranslatePx,
		AT_ZoomPx,
		AT_SetCPURendering,
		AT_SetGPURendering,
		AT_EnableLazyRendering,
		AT_DisableLazyRendering,
		AT_Quit
	};

	struct Action
	{
		ActionType type;
		Real x, y;
		int ix, iy;
		Real z;
		int iz;
		int loops;
		Action() : type(AT_WaitFrame), x(0), y(0), ix(0), iy(0), z(0), loops(0) {}
	};

	std::ifstream inp;

	Action currentAction;

	void ParseAction();
};

}

#endif
