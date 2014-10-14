#ifndef _PROFILER_H
#define _PROFILER_H

#include <map>
#include <string>
#include <stack>

#define PROFILE_SCOPE(name) IPDF::ProfilerScope _localProfZone(name)

namespace IPDF
{
	class Profiler
	{
	public:
		Profiler() {}
		
		void BeginZone(std::string name);
		void EndZone();

		void EndFrame();
	private:
		struct ProfileZone
		{
			uint64_t tics_begin;
			uint64_t tics_end;
			uint64_t tics_frame;
			uint64_t tics_total;
			uint64_t calls_frame;
			uint64_t calls_total;
		};

		std::map<std::string, ProfileZone> m_zones;
		std::stack<std::string> m_zone_stack;
	};

	extern Profiler g_profiler;
	struct ProfilerScope
	{
		ProfilerScope(std::string name) { g_profiler.BeginZone(name); }
		~ProfilerScope() { g_profiler.EndZone(); }
	};

}

#endif
