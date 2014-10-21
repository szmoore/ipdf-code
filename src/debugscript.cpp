#include "debugscript.h"
#include "profiler.h"

#include <string>

using namespace IPDF;
using namespace std;

void DebugScript::ParseAction(View * view, Screen * scr)
{
	*m_input >> std::ws;
	if (m_input == NULL || !m_input->good())
		return;
	istream & inp = *m_input;
	Debug("Get action type...");
	std::string actionType;
	inp >> actionType;
	Debug("Action type: %s", actionType.c_str());
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
	else if (actionType == "approachz") // approach zenoistically
	{
		currentAction.type = AT_ApproachBoundsZeno;
		std::string _x, _y, _w, _h, _z;
		inp >> _x >> _y >> _w >> _h >> _z;
		currentAction.x = RealFromStr(_x.c_str());
		currentAction.y = RealFromStr(_y.c_str());
		currentAction.w = RealFromStr(_w.c_str());
		currentAction.h = RealFromStr(_h.c_str());
		currentAction.z = RealFromStr(_z.c_str());
	}
	else if (actionType == "approachl") // approach linearly
	{
		currentAction.type = AT_ApproachBoundsLinear;
		std::string _x, _y, _w, _h, _z;
		inp >> _x >> _y >> _w >> _h >> _z;
		currentAction.x = RealFromStr(_x.c_str());
		currentAction.y = RealFromStr(_y.c_str());
		currentAction.w = RealFromStr(_w.c_str());
		currentAction.h = RealFromStr(_h.c_str());
		currentAction.z = RealFromStr(_z.c_str());
		currentAction.x = (currentAction.x - view->GetBounds().x)/currentAction.z;
		currentAction.y = (currentAction.y - view->GetBounds().y)/currentAction.z;
		currentAction.w = (currentAction.w - view->GetBounds().w)/currentAction.z;
		currentAction.h = (currentAction.h - view->GetBounds().h)/currentAction.z;
	}
	else if (actionType == "setbounds")
	{
		currentAction.type = AT_SetBounds;
		std::string _x, _y, _w, _h;
		inp >> _x >> _y >> _w >> _h;
		currentAction.x = RealFromStr(_x.c_str());
		currentAction.y = RealFromStr(_y.c_str());
		currentAction.w = RealFromStr(_w.c_str());
		currentAction.h = RealFromStr(_h.c_str());
	}
	else if (actionType == "querygpubounds")
	{
		currentAction.type = AT_QueryGPUBounds;
		inp >> currentAction.textargs;
		currentAction.loops = 1;
	}
	else if (actionType == "screenshot")
	{
		currentAction.type = AT_ScreenShot;
		inp >> currentAction.textargs;	
	}
	else if (actionType == "printfps")
	{
		currentAction.type = AT_PrintFPS;
		currentAction.iz = currentAction.loops;
		m_fps_cpu_mean = 0;
		m_fps_gpu_mean = 0;
		m_fps_cpu_stddev = 0;
		m_fps_gpu_stddev = 0;
	}
	else if (actionType == "printbounds")
	{
		currentAction.type = AT_PrintBounds;
	}
	else if (actionType == "profileon")
	{
		currentAction.type = AT_ProfileDisplay;
		currentAction.iz = 1;
	}
	else if (actionType == "profileoff")
	{
		currentAction.type = AT_ProfileDisplay;
		currentAction.iz = 0;
	}
	else
		Fatal("Unknown action %s", actionType.c_str());

}

bool DebugScript::Execute(View *view, Screen *scr)
{
	if (currentAction.loops <= 0)
	{
		if (m_index >= m_actions.size())
		{
			ParseAction(view, scr);
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
#ifndef QUADTREE_DISABLED
		view->Doc().SetQuadtreeInsertNode(view->GetCurrentQuadtreeNode());
#endif
		#ifdef TRANSFORM_OBJECTS_NOT_VIEW
			view->Doc().LoadSVG(currentAction.textargs, Rect(Real(1)/Real(2),Real(1)/Real(2),Real(1)/Real(800),Real(1)/Real(600)));	
		#else
			const Rect & bounds = view->GetBounds();
			view->Doc().LoadSVG(currentAction.textargs, Rect(bounds.x+bounds.w/Real(2),bounds.y+bounds.h/Real(2),bounds.w/Real(800),bounds.h/Real(600)));
		#endif
#ifndef QUADTREE_DISABLED
		view->Doc().PropagateQuadChanges(view->GetCurrentQuadtreeNode());
		view->Doc().PropagateQuadChanges(view->Doc().GetQuadTree().GetNeighbour(view->GetCurrentQuadtreeNode(), 0, 1, 0));
		view->Doc().PropagateQuadChanges(view->Doc().GetQuadTree().GetNeighbour(view->GetCurrentQuadtreeNode(), 1, 0, 0));
		view->Doc().PropagateQuadChanges(view->Doc().GetQuadTree().GetNeighbour(view->GetCurrentQuadtreeNode(), 1, 1, 0));
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
		
	case AT_ApproachBoundsZeno:
	{	
		VRect target(currentAction.x, currentAction.y, currentAction.w, currentAction.h);
		if (currentAction.z != VReal(1))
		{
			target.x = view->GetBounds().x + (target.x-view->GetBounds().x)/VReal(currentAction.z);
			target.y = view->GetBounds().y + (target.y-view->GetBounds().y)/VReal(currentAction.z);
			target.w = view->GetBounds().w + (target.w-view->GetBounds().w)/VReal(currentAction.z);
			target.h = view->GetBounds().h + (target.h-view->GetBounds().h)/VReal(currentAction.z);
		}
		

		VReal s = target.w/(view->GetBounds().w);
		if (Real(s) != 1)
		{
			VReal x0;
			VReal y0;
			x0 = (view->GetBounds().x - target.x)/((s - VReal(1))*view->GetBounds().w);
			y0 = (view->GetBounds().y - target.y)/((s - VReal(1))*view->GetBounds().h);
			view->ScaleAroundPoint(x0, y0, s);
			currentAction.loops++;
		}
		else
		{
			Debug("Already at target view; Waiting for remaining %d frames", currentAction.loops);
			currentAction.type = AT_WaitFrame;
		}
		break;
	}
	case AT_ApproachBoundsLinear:
	{
		VRect target(currentAction.x, currentAction.y, currentAction.w, currentAction.h);
		target.x += view->GetBounds().x;
		target.y += view->GetBounds().y;
		target.w += view->GetBounds().w;
		target.h += view->GetBounds().h;
		VReal s = target.w/(view->GetBounds().w);
		if (Real(s) != 1)
		{
			VReal x0;
			VReal y0;
			x0 = (view->GetBounds().x - target.x)/((s - VReal(1))*view->GetBounds().w);
			y0 = (view->GetBounds().y - target.y)/((s - VReal(1))*view->GetBounds().h);
			view->ScaleAroundPoint(x0, y0, s);
			currentAction.loops++;
		}
		else
		{
			Debug("Already at target view; Waiting for remaining %d frames", currentAction.loops);
			currentAction.type = AT_WaitFrame;
		}
		break;
	}
	case AT_SetBounds:
	{
		VRect target(currentAction.x, currentAction.y, currentAction.w, currentAction.h);
		view->SetBounds(target);
		break;
	}
	
	case AT_QueryGPUBounds:
	{
		view->QueryGPUBounds(currentAction.textargs.c_str(), "w");
		currentAction.loops = 1;
		break;
	}
	case AT_ScreenShot:
	{
		view->SaveBMP(currentAction.textargs.c_str());
		currentAction.loops = 1;
		break;
	}
	case AT_PrintFPS:
	{
		// Using a (apparently) Soviet trick to calculate the stddev in one pass
		// This was my favourite algorithm in my Physics honours project
		// Ah the memories
		// The horrible horrible memories
		// At least things won't get that bad
		// Right?
		if (currentAction.loops <= 1)
		{
			double n = double(currentAction.iz);
			m_fps_cpu_mean /= n;
			m_fps_gpu_mean /= n;
			
			m_fps_cpu_stddev = sqrt(m_fps_cpu_stddev / n - m_fps_cpu_mean*m_fps_cpu_mean);
			m_fps_gpu_stddev = sqrt(m_fps_gpu_stddev / n - m_fps_gpu_mean*m_fps_gpu_mean);
			
			
			
			printf("%d\t%f\t%f\t%f\t%f\n", currentAction.iz,
				m_fps_gpu_mean, m_fps_gpu_stddev,
				m_fps_cpu_mean, m_fps_cpu_stddev);
		}
		else
		{
			
			double fps_cpu = 1.0/scr->GetLastFrameTimeCPU();
			double fps_gpu = 1.0/scr->GetLastFrameTimeGPU();
			
			m_fps_cpu_mean += fps_cpu;
			m_fps_gpu_mean += fps_gpu;
			
			m_fps_cpu_stddev += fps_cpu*fps_cpu;
			m_fps_gpu_stddev += fps_gpu*fps_gpu;
		}
		break;
	}
	case AT_PrintBounds:
	{
		printf("%s\t%s\t%s\t%s\n", Str(view->GetBounds().x).c_str(), Str(view->GetBounds().y).c_str(), Str(view->GetBounds().w).c_str(), Str(view->GetBounds().h).c_str());
		break;
	}
	case AT_ProfileDisplay:
	{
		g_profiler.Enable(currentAction.iz);
		break;
	}
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
	#ifdef QUADTREE_DISABLED
	printf("%d\t%llu\t%llu\t%s\t%s\t%s\t%s\t%s\t%s\t%u\n",
		now.object_count, (long long unsigned)now.clock,
		(long long unsigned)(now.clock - m_perf_last.clock),
		Str(now.view_bounds.x).c_str(), Str(Log10(Abs(now.view_bounds.x))).c_str(),
		Str(now.view_bounds.y).c_str(), Str(Log10(Abs(now.view_bounds.y))).c_str(),
		Str(now.view_bounds.w).c_str(), Str(Log10(now.view_bounds.w)).c_str(),
		(unsigned)Size(now.view_bounds.w));
	#endif
	m_perf_last = now;
}
