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

	void ProfilingData::startElapsedNonMultithread(void)
	{
		m_clock.tick();
	}

	void ProfilingData::addElapsedMcs(size_t elapsed)
	{
		if (elapsed == -1)
		{
			m_clock.tock();
			elapsed = m_clock.duration().count();
		}
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		m_num++;
		m_elapsed += elapsed;
		if (elapsed > m_maxElapsed)
			m_maxElapsed = elapsed;
		if (elapsed < m_minElapsed || m_minElapsed == 0)
			m_minElapsed = elapsed;
	}

	void ProfilingData::reset(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		m_num = 0;
		m_elapsed = 0;
		m_minElapsed = 0;
		m_maxElapsed = 0;
	}

	size_t ProfilingData::num(void)
	{
		return m_num;
	}

	size_t ProfilingData::elapsedMcs(size_t& num)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Mtx);
		num = m_num;
		return m_elapsed;
	}

	size_t ProfilingData::minElapsedMcs(void)
	{
		return m_minElapsed;
	}

	size_t ProfilingData::maxElapsedMcs(void)
	{
		return m_maxElapsed;
	}

	size_t ProfilingData::averageElapsedMcs(size_t& num)
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
		std::string n = function.empty() ? name : function + "_***" + name + "***";
		//std::string n = function.empty() ? name : function + "_" + name;
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

	void ProfilerInternal::startElapsedNonMultithread(std::string function, std::string name)
	{
		getData(function, name)->startElapsedNonMultithread();
	}
		
	void ProfilerInternal::addElapsedMcs(std::string function, std::string name, size_t elapsed)
	{
		getData(function, name)->addElapsedMcs(elapsed);
	}

	void ProfilerInternal::reset(std::string function, std::string name)
	{
		getData(function, name)->reset();
	}

	size_t ProfilerInternal::num(std::string function, std::string name)
	{
		return getData(function, name)->num();
	}

	size_t ProfilerInternal::elapsedMcs(std::string function, std::string name, size_t& num)
	{
		return getData(function, name)->elapsedMcs(num);
	}

	size_t ProfilerInternal::averageElapsedMcs(std::string function, std::string name, size_t& num)
	{
		return getData(function, name)->averageElapsedMcs(num);
	}

	size_t ProfilerInternal::minElapsedMcs(std::string function, std::string name)
	{
		return getData(function, name)->minElapsedMcs();
	}

	size_t ProfilerInternal::maxElapsedMcs(std::string function, std::string name)
	{
		return getData(function, name)->maxElapsedMcs();
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

	void Profiler::startElapsedNonMultithread(std::string function, std::string name)
	{
		ProfilerInternal::get().startElapsedNonMultithread(function, name);
	}

	void Profiler::addElapsedMcs(std::string function, std::string name, size_t elapsed)
	{
		ProfilerInternal::get().addElapsedMcs(function, name, elapsed);
	}

	void Profiler::addElapsedMs(std::string function, std::string name, size_t elapsed)
	{
		ProfilerInternal::get().addElapsedMcs(function, name, elapsed * 1000);
	}

	void Profiler::reset(std::string function, std::string name)
	{
		ProfilerInternal::get().reset(function, name);
	}

	size_t Profiler::num(std::string function, std::string name)
	{
		return ProfilerInternal::get().num(function, name);
	}

	size_t Profiler::elapsedMcs(std::string function, std::string name, size_t& num)
	{
		return ProfilerInternal::get().elapsedMcs(function, name, num);
	}

	size_t Profiler::averageElapsedMcs(std::string function, std::string name, size_t& num)
	{
		return ProfilerInternal::get().averageElapsedMcs(function, name, num);
	}

	size_t Profiler::elapsedMs(std::string function, std::string name, size_t& num)
	{
		return ProfilerInternal::get().elapsedMcs(function, name, num) / 1000;
	}

	size_t Profiler::averageElapsedMs(std::string function, std::string name, size_t& num)
	{
		return ProfilerInternal::get().averageElapsedMcs(function, name, num) / 1000;
	}
	
	size_t Profiler::minElapsedMcs(std::string function, std::string name)
	{
		return ProfilerInternal::get().minElapsedMcs(function, name);
	}

	size_t Profiler::maxElapsedMcs(std::string function, std::string name)
	{
		return ProfilerInternal::get().maxElapsedMcs(function, name);
	}

	size_t Profiler::minElapsedMs(std::string function, std::string name)
	{
		return ProfilerInternal::get().minElapsedMcs(function, name) / 1000;
	}

	size_t Profiler::maxElapsedMs(std::string function, std::string name)
	{
		return ProfilerInternal::get().maxElapsedMcs(function, name) / 1000;
	}

}

