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
	else if (actionType == "gpu")
	{
		currentAction.type = AT_SetGPURendering;
		return;
	}
	else if (actionType == "cpu")
	{
		currentAction.type = AT_SetCPURendering;
		return;
	}
	else if (actionType == "lazy")
	{
		currentAction.type = AT_EnableLazyRendering;
		return;
	}
	else if (actionType == "nolazy")
	{
		currentAction.type = AT_DisableLazyRendering;
		return;
	}
	else if (actionType == "quit")
	{
		currentAction.type = AT_Quit;
	}
	else if (actionType == "loadsvg")
	{
		currentAction.type = AT_LoadSVG;
		inp >> currentAction.filename;
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
	case AT_SetGPURendering:
		view->SetGPURendering(true);
		break;
	case AT_SetCPURendering:
		view->SetGPURendering(false);
		break;
	case AT_EnableLazyRendering:
		view->SetLazyRendering(true);
		break;
	case AT_DisableLazyRendering:
		view->SetLazyRendering(false);
		break;
	case AT_LoadSVG:
		#ifdef TRANSFORM_OBJECTS_NOT_VIEW
			view->Doc().LoadSVG(currentAction.filename, Rect(Real(1)/Real(2),Real(1)/Real(2),Real(1)/Real(800),Real(1)/Real(600)));	
		#else
			Rect & bounds = view->GetBounds();
			view->Doc().LoadSVG(currentAction.filename, Rect(bounds.x+bounds.w/Real(2),bounds.y+bounds.h/Real(2),bounds.w/Real(800),bounds.h/Real(600)));
		#endif
		currentAction.type = AT_WaitFrame;
		view->ForceRenderDirty();
		view->ForceBufferDirty();
		view->ForceBoundsDirty();
		currentAction.loops = 0;
		break;
	default:
		Fatal("Unknown script command in queue.");
	}
	currentAction.loops--;
	return false;
}
