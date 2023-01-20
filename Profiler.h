#pragma once

#include "Utils.h"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>

#ifndef MYAPI 
	#define MYAPI   __declspec( dllimport )
#endif 

namespace TheWorld_Utils
{

	class ProfilingData
	{
	public:
		ProfilingData();
		~ProfilingData();

		void startElapsedNonMultithread(void);
		void addElapsedMcs(size_t elapsed = -1);
		void reset(void);
		size_t num(void);
		size_t elapsedMcs(size_t& num);
		size_t averageElapsedMcs(size_t& num);
		size_t minElapsedMcs(void);
		size_t maxElapsedMcs(void);

	private:
		size_t m_num;
		size_t m_elapsed;
		size_t m_minElapsed;
		size_t m_maxElapsed;
		TheWorld_Utils::TimerMcs m_clock;

		std::recursive_mutex m_Mtx;
	};

	class ProfilerInternal
	{
	public:
		ProfilerInternal(void);
		~ProfilerInternal(void);

		ProfilingData* getData(std::string function, std::string name);
		std::vector<std::string> names(std::string function);
		void startElapsedNonMultithread(std::string function, std::string name);
		void addElapsedMcs(std::string function, std::string name, size_t elapsed = -1);
		void reset(std::string function, std::string name);
		size_t num(std::string function, std::string name);
		size_t elapsedMcs(std::string function, std::string name, size_t& num);
		size_t averageElapsedMcs(std::string function, std::string name, size_t& num);
		size_t minElapsedMcs(std::string function, std::string name);
		size_t maxElapsedMcs(std::string function, std::string name);

		static ProfilerInternal& get();

	private:
		std::recursive_mutex m_mapProfilingDataMtx;
		std::map<std::string, std::unique_ptr<ProfilingData>> m_mapProfilingData;
	};

	class MYAPI Profiler
	{
	public:
		Profiler(void);
		~Profiler(void);

		static std::vector<std::string> names(std::string function);
		static void startElapsedNonMultithread(std::string function, std::string name);
		static void addElapsedMcs(std::string function, std::string name, size_t elapsed);
		static void addElapsedMs(std::string function, std::string name, size_t elapsed);
		static void reset(std::string function, std::string name);
		static size_t num(std::string function, std::string name);
		static size_t elapsedMcs(std::string function, std::string name, size_t& num);
		static size_t averageElapsedMcs(std::string function, std::string name, size_t& num);
		static size_t elapsedMs(std::string function, std::string name, size_t& num);
		static size_t averageElapsedMs(std::string function, std::string name, size_t& num);
		static size_t minElapsedMcs(std::string function, std::string name);
		static size_t maxElapsedMcs(std::string function, std::string name);
		static size_t minElapsedMs(std::string function, std::string name);
		static size_t maxElapsedMs(std::string function, std::string name);

	private:
	};

	class GuardProfiler
	{
	public:
		GuardProfiler(std::string function, std::string name)
		{
			m_function = function;
			m_name = name;
			m_clock.tick();
		}
		~GuardProfiler(void)
		{
			m_clock.tock();
			if (!m_name.empty())
				TheWorld_Utils::Profiler::addElapsedMcs(m_function, m_name, m_clock.duration().count());
		}

	private:
		std::string m_function;
		std::string m_name;
		TheWorld_Utils::TimerMcs m_clock;
	};
}
