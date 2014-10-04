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
	DebugScript() : inp(), currentAction(), m_actions(), m_labels(), m_index(0) {}
	virtual ~DebugScript() {}
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
		AT_LoadSVG,
		AT_Label,
		AT_Goto,
		AT_Debug,
		AT_ClearDocument,
		AT_ClearPerformance,
		AT_PrintPerformance,
		AT_RecordPerformance,
		AT_DebugFont,
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
		std::string textargs;
		Action() : type(AT_WaitFrame), x(0), y(0), ix(0), iy(0), z(0), loops(0), textargs("") {}
	};

	std::ifstream inp;

	Action currentAction;
	std::vector<Action> m_actions;
	std::map<std::string, int> m_labels;
	unsigned m_index;
	
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
	
	void ParseAction();
};

}

#endif
