#include "profiler.h"
#include "log.h"

#include "SDL.h" 

using namespace IPDF;

Profiler IPDF::g_profiler;

void Profiler::BeginZone(std::string name)
{
	if (!m_zones.count(name))
		m_zones[name] = ProfileZone{0,0,0,0,0,0};
	m_zones[name].tics_begin = SDL_GetPerformanceCounter();
	m_zone_stack.push(name);
}

void Profiler::EndZone()
{
	std::string name = m_zone_stack.top();
	m_zone_stack.pop();
	m_zones[name].tics_end = SDL_GetPerformanceCounter();
	m_zones[name].tics_frame += m_zones[name].tics_end - m_zones[name].tics_begin;
	m_zones[name].tics_total += m_zones[name].tics_end - m_zones[name].tics_begin;
	m_zones[name].calls_frame++;
	m_zones[name].calls_total++;
}

void Profiler::EndFrame()
{
	// Zero all of the frame counts
	for (auto& it : m_zones)
	{
		if (m_enabled)
		{
			printf("perf_zone\t\"%s\"\t%lu %lu\t%lu %lu\n", it.first.c_str(), it.second.tics_frame * 1000 / SDL_GetPerformanceFrequency(), it.second.calls_frame, it.second.tics_total * 1000 / SDL_GetPerformanceFrequency(), it.second.calls_total);
		}
		it.second.tics_frame = 0;
		it.second.calls_frame = 0;
	}
}
