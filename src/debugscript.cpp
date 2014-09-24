#include "debugscript.h"

#include <string>

using namespace IPDF;

void DebugScript::ParseAction()
{
	std::string actionType;
	inp >> actionType;
	// Skip comments
	while (actionType[0] == '#')
	{
		std::string tmp;
		std::getline(inp, tmp);
		inp >> std::ws >> actionType;
	}
	if (actionType == "loop")
	{
		inp >> currentAction.loops >> actionType;
	}
	else
	{
		currentAction.loops = 1;
	}

	if (actionType == "wait")
	{
		currentAction.type = AT_WaitFrame;
		return;
	}
	else if (actionType == "translate")
	{
		std::string _x, _y;
		inp >> _x >> _y;
		currentAction.type = AT_Translate;
		currentAction.x = RealFromStr(_x.c_str());
		currentAction.y = RealFromStr(_y.c_str());
		return;
	}
	else if (actionType == "zoom")
	{
		std::string _x, _y, _z;
		inp >> _x >> _y >> _z;
		currentAction.type = AT_Zoom;
		currentAction.x = RealFromStr(_x.c_str());
		currentAction.y = RealFromStr(_y.c_str());
		currentAction.z = RealFromStr(_z.c_str());
		return;
	}
	else if (actionType == "pxtranslate")
	{
		inp >> currentAction.ix >> currentAction.iy;
		currentAction.type = AT_TranslatePx;
		return;
	}
	else if (actionType == "pxzoom")
	{
		inp >> currentAction.ix >> currentAction.iy >> currentAction.iz;
		currentAction.type = AT_ZoomPx;
		return;
	}
	else if (actionType == "quit")
	{
		currentAction.type = AT_Quit;
	}
}

bool DebugScript::Execute(View *view, Screen *scr)
{
	if (currentAction.loops == 0)
		ParseAction();

	switch(currentAction.type)
	{
	case AT_Quit:
		return true;
	case AT_WaitFrame:
		break;
	case AT_Translate:
		view->Translate(currentAction.x, currentAction.y);
		break;
	case AT_TranslatePx:
		view->Translate(Real(currentAction.ix)/Real(scr->ViewportWidth()), Real(currentAction.iy)/Real(scr->ViewportHeight()));
		break;
	case AT_Zoom:
		view->ScaleAroundPoint(currentAction.x, currentAction.y, currentAction.z);
		break;
	case AT_ZoomPx:
		view->ScaleAroundPoint(Real(currentAction.ix)/Real(scr->ViewportWidth()),Real(currentAction.iy)/Real(scr->ViewportHeight()), Real(expf(-currentAction.iz/20.f)));
		break;
	default:
		Fatal("Unknown script command in queue.");
	}
	currentAction.loops--;
	return false;
}
