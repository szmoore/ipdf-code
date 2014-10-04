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
	}
	else if (actionType == "pxtranslate")
	{
		inp >> currentAction.ix >> currentAction.iy;
		currentAction.type = AT_TranslatePx;
	}
	else if (actionType == "pxzoom")
	{
		inp >> currentAction.ix >> currentAction.iy >> currentAction.iz;
		currentAction.type = AT_ZoomPx;
	}
	else if (actionType == "gpu")
	{
		currentAction.type = AT_SetGPURendering;
	}
	else if (actionType == "cpu")
	{
		currentAction.type = AT_SetCPURendering;
	}
	else if (actionType == "lazy")
	{
		currentAction.type = AT_EnableLazyRendering;
	}
	else if (actionType == "nolazy")
	{
		currentAction.type = AT_DisableLazyRendering;
	}
	else if (actionType == "quit")
	{
		currentAction.type = AT_Quit;
	}
	else if (actionType == "loadsvg")
	{
		currentAction.type = AT_LoadSVG;
		inp >> currentAction.textargs;
	}
	else if (actionType == "label")
	{
		currentAction.type = AT_Label;
		inp >> currentAction.textargs;
	}
	else if (actionType == "goto")
	{
		currentAction.type = AT_Goto;
		inp >> currentAction.textargs;
	}
	else if (actionType == "debug")
	{
		currentAction.type = AT_Debug;
		getline(inp,currentAction.textargs);
	}
	else if (actionType == "clear")
	{
		currentAction.type = AT_ClearDocument;
	}
	else if (actionType == "clearperf")
	{
		currentAction.type = AT_ClearPerformance;
	}
	else if (actionType == "printperf")
	{
		currentAction.type = AT_PrintPerformance;
	}
	else if (actionType == "recordperf")
	{
		currentAction.type = AT_RecordPerformance;
	}
	else if (actionType == "debugfont")
	{
		currentAction.type = AT_DebugFont;
		inp >> currentAction.textargs;
	}

}

bool DebugScript::Execute(View *view, Screen *scr)
{
	if (currentAction.loops <= 0)
	{
		if (m_index >= m_actions.size())
		{
			ParseAction();
			if (m_labels.size() > 0)
			{
				m_actions.push_back(currentAction);
				m_index++;
			}
				
		}
		else
			currentAction = m_actions[m_index++];
	}

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
	{
		#ifdef TRANSFORM_OBJECTS_NOT_VIEW
			view->Doc().LoadSVG(currentAction.textargs, Rect(Real(1)/Real(2),Real(1)/Real(2),Real(1)/Real(800),Real(1)/Real(600)));	
		#else
			const Rect & bounds = view->GetBounds();
			view->Doc().LoadSVG(currentAction.textargs, Rect(bounds.x+bounds.w/Real(2),bounds.y+bounds.h/Real(2),bounds.w/Real(800),bounds.h/Real(600)));
		#endif
		currentAction.type = AT_WaitFrame;
		view->ForceRenderDirty();
		view->ForceBufferDirty();
		view->ForceBoundsDirty();
		currentAction.loops = 1;
		break;
	}
	case AT_Label:
		m_labels[currentAction.textargs] = m_index;
		currentAction.type = AT_WaitFrame;
		currentAction.loops = 1;
		break;
	case AT_Goto:
		m_index = m_labels[currentAction.textargs];
		currentAction.loops = 1;
		break;
	case AT_Debug:
		Debug("View bounds: %s", view->GetBounds().Str().c_str());
		if (currentAction.textargs.size() > 0)
			Debug("%s", currentAction.textargs.c_str());
		break;
	case AT_ClearDocument:
		view->Doc().ClearObjects();
		currentAction.loops = 1;
		break;
	case AT_ClearPerformance:
		ClearPerformance(view, scr);
		currentAction.loops = 1;
		break;
	case AT_PrintPerformance:
		PrintPerformance(view, scr);
		currentAction.loops = 1;	
		break;
	case AT_RecordPerformance:
		PrintPerformance(view, scr);
		break;
	case AT_DebugFont:
		scr->ShowDebugFont(currentAction.textargs == "1" || currentAction.textargs == "on");
		currentAction.loops = 1;
		break;
	default:
		Fatal("Unknown script command in queue.");
	}
	currentAction.loops--;
	return false;
}

void DebugScript::ClearPerformance(View * view, Screen * scr)
{
	m_perf_start.clock = clock();
	m_perf_start.object_count = view->Doc().ObjectCount();
	m_perf_start.view_bounds = view->GetBounds();
	m_perf_last = m_perf_start;
}

void DebugScript::PrintPerformance(View * view, Screen * scr)
{
	DebugScript::PerformanceData now;
	now.clock = clock();
	now.object_count = view->Doc().ObjectCount();
	now.view_bounds = view->GetBounds();

	// object_count  clock  delta_clock  x  Log10(x)  y  Log10(y)  w  Log10(w)  Size(w)
	printf("%d\t%lu\t%lu\t%s\t%f\t%s\t%f\t%s\t%f\t%lu\n",
		now.object_count, (uint64_t)now.clock,
		(uint64_t)(now.clock - m_perf_last.clock),
		Str(now.view_bounds.x).c_str(), Log10(now.view_bounds.x),
		Str(now.view_bounds.y).c_str(), Log10(now.view_bounds.y),
		Str(now.view_bounds.w).c_str(), Log10(now.view_bounds.w),
		Size(now.view_bounds.w));
	m_perf_last = now;
}
