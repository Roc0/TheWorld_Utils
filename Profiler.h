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

		void startElapsed(void);
		void addElapsed(size_t elapsed = -1);
		void reset(void);
		size_t num(void);
		size_t elapsed(size_t& num);
		size_t averageElapsed(size_t& num);

	private:
		size_t m_num;
		size_t m_elapsed;
		TheWorld_Utils::TimerMs m_clock;

		std::recursive_mutex m_Mtx;
	};

	class ProfilerInternal
	{
	public:
		ProfilerInternal(void);
		~ProfilerInternal(void);

		ProfilingData* getData(std::string function, std::string name);
		std::vector<std::string> names(std::string function);
		void startElapsed(std::string function, std::string name);
		void addElapsed(std::string function, std::string name, size_t elapsed = -1);
		void reset(std::string function, std::string name);
		size_t num(std::string function, std::string name);
		size_t elapsed(std::string function, std::string name, size_t& num);
		size_t averageElapsed(std::string function, std::string name, size_t& num);

		static ProfilerInternal& get();

	private:
		std::recursive_mutex m_mapProfilingDataMtx;
		std::map<std::string, std::unique_ptr<ProfilingData>> m_mapProfilingData;
	};

	class MYAPI Profiler
	{
	public:
		Profiler(void);
		~Profiler();

		static std::vector<std::string> names(std::string function);
		static void startElapsed(std::string function, std::string name);
		static void addElapsed(std::string function, std::string name, size_t elapsed = -1);
		static void reset(std::string function, std::string name);
		static size_t num(std::string function, std::string name);
		static size_t elapsed(std::string function, std::string name, size_t& num);
		static size_t averageElapsed(std::string function, std::string name, size_t& num);

	private:
	};
}
