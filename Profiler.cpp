#include "pch.h"

#define MYAPI   __declspec( dllexport )
#include "Profiler.h"

namespace TheWorld_Utils
{
	ProfilerInternal s_profiler;

	ProfilingData::ProfilingData()
	{
		reset();
	}

	ProfilingData::~ProfilingData()
	{
	}

	void ProfilingData::startElapsed(void)
	{
		m_clock.tick();
	}

	void ProfilingData::addElapsed(size_t elapsed)
	{
		if (elapsed == -1)
		{
			m_clock.tock();
			elapsed = m_clock.duration().count();
		}
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		m_num++;
		m_elapsed += elapsed;
	}

	void ProfilingData::reset(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		m_num = 0;
		m_elapsed = 0;
	}

	size_t ProfilingData::num(void)
	{
		return m_num;
	}

	size_t ProfilingData::elapsed(size_t& num)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		num = m_num;
		return m_elapsed;
	}

	size_t ProfilingData::averageElapsed(size_t& num)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		num = m_num;
		return m_num > 0 ? m_elapsed / m_num : 0;
	}

	ProfilerInternal::ProfilerInternal(void)
	{
	}

	ProfilerInternal::~ProfilerInternal(void)
	{
	}

	ProfilerInternal& ProfilerInternal::get()
	{
		return s_profiler;
	}

	ProfilingData* ProfilerInternal::getData(std::string function, std::string name)
	{
		std::string n = function.empty() ? name : function + "_" + name;
		std::lock_guard<std::recursive_mutex> lock(m_mapProfilingDataMtx);
		if (!m_mapProfilingData.contains(n))
		{
			m_mapProfilingData[n] = std::make_unique<ProfilingData>();
		}
		return m_mapProfilingData[n].get();
	}

	std::vector<std::string> ProfilerInternal::names(std::string function)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mapProfilingDataMtx);
		std::vector<std::string> names;
		if (function.empty())
		{
			names.resize(m_mapProfilingData.size());
			size_t idx = 0;
			for (const auto& item : m_mapProfilingData)
			{
				names[idx] = item.first;
				idx++;
			}
		}
		else
		{
			for (const auto& item : m_mapProfilingData)
				if (item.first.starts_with(function + "_"))
					names.push_back(item.first);
		}
		
		std::sort(names.begin(), names.end());
		
		return names;
	}

	void ProfilerInternal::startElapsed(std::string function, std::string name)
	{
		getData(function, name)->startElapsed();
	}
		
	void ProfilerInternal::addElapsed(std::string function, std::string name, size_t elapsed)
	{
		getData(function, name)->addElapsed(elapsed);
	}

	void ProfilerInternal::reset(std::string function, std::string name)
	{
		getData(function, name)->reset();
	}

	size_t ProfilerInternal::num(std::string function, std::string name)
	{
		return getData(function, name)->num();
	}

	size_t ProfilerInternal::elapsed(std::string function, std::string name, size_t& num)
	{
		return getData(function, name)->elapsed(num);
	}

	size_t ProfilerInternal::averageElapsed(std::string function, std::string name, size_t& num)
	{
		return getData(function, name)->averageElapsed(num);
	}

	Profiler::Profiler(void)
	{
	}

	Profiler::~Profiler(void)
	{
	}

	//ProfilingData* Profiler::getData(std::string name)
	//{
	//	return ProfilerInternal::get()->getData(name);
	//}

	std::vector<std::string> Profiler::names(std::string function)
	{
		return ProfilerInternal::get().names(function);
	}

	void Profiler::startElapsed(std::string function, std::string name)
	{
		ProfilerInternal::get().startElapsed(function, name);
	}

	void Profiler::addElapsed(std::string function, std::string name, size_t elapsed)
	{
		ProfilerInternal::get().addElapsed(function, name, elapsed);
	}

	void Profiler::reset(std::string function, std::string name)
	{
		ProfilerInternal::get().reset(function, name);
	}

	size_t Profiler::num(std::string function, std::string name)
	{
		return ProfilerInternal::get().num(function, name);
	}

	size_t Profiler::elapsed(std::string function, std::string name, size_t& num)
	{
		return ProfilerInternal::get().elapsed(function, name, num);
	}

	size_t Profiler::averageElapsed(std::string function, std::string name, size_t& num)
	{
		return ProfilerInternal::get().averageElapsed(function, name, num);
	}
}

