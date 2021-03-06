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
	DebugScript(std::istream * in) : m_input(in), currentAction(), m_actions(), m_labels(), m_index(0) {}
	virtual ~DebugScript() {}

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
		AT_LoadSVG,
		AT_Label,
		AT_Goto,
		AT_Debug,
		AT_ClearDocument,
		AT_ClearPerformance,
		AT_PrintPerformance,
		AT_RecordPerformance,
		AT_DebugFont,
		AT_ApproachBoundsZeno,
		AT_ApproachBoundsLinear,
		AT_SetBounds,
		AT_QueryGPUBounds, // query bounds of Beziers when transformed to GPU
		AT_ScreenShot, // take screenshot
		AT_PrintSPF, // Print FPS statistics about the frames
		AT_PrintBounds, // Print bounds
		AT_ProfileDisplay,
		AT_Quit
	};

	struct Action
	{
		ActionType type;
		VReal x, y;
		int ix, iy;
		VReal z;
		int iz;
		int loops;
		VReal w, h;
		std::string textargs;
		Action() : type(AT_WaitFrame), x(0), y(0), ix(0), iy(0), z(0), loops(0), textargs("") {}
	};

	std::istream * m_input;

	Action currentAction;
	std::vector<Action> m_actions;
	std::map<std::string, int> m_labels;
	unsigned m_index;
	
	double m_spf_cpu_mean;
	double m_spf_gpu_mean;
	double m_spf_cpu_stddev;
	double m_spf_gpu_stddev;
	
	struct PerformanceData
	{
		clock_t clock;
		unsigned object_count;
		VRect view_bounds;
	};
	
	PerformanceData m_perf_start;
	PerformanceData m_perf_last;
	
	void PrintPerformance(View * view, Screen * scr);
	void ClearPerformance(View * view, Screen * scr);
	
	void ParseAction(View * view, Screen * scr);
};

}

#endif
