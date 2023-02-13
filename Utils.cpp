//#include "pch.h"

#define MYAPI   __declspec( dllexport )

//#define _THEWORLD_CLIENT

//#ifdef _THEWORLD_CLIENT
//	#include <Godot.hpp>
//	#include <ResourceLoader.hpp>
//	#include <File.hpp>
//#endif

#define NOMINMAX
#include <cstdio>
#include "Utils.h"
#include "Profiler.h"
#include "half.h"
#include <filesystem>
#include <string>
#include <algorithm>
#include <plog/Initializers/RollingFileInitializer.h>
#include <Eigen/Dense>
#include <Rpc.h>
#include <cmath>
#include "FastNoiseLite.h"

namespace fs = std::filesystem;

namespace TheWorld_Utils
{
	TerrainEdit::TerrainEdit(enum class TerrainEdit::TerrainType terrainType)
	{
		size = sizeof(TerrainEdit);
		needUploadToServer = false;

		TerrainEdit::terrainType = terrainType;
		init(terrainType);

		minHeight = 0;
		maxHeight = 0;
	}

	std::string TerrainEdit::terrainTypeString(enum class TerrainEdit::TerrainType terrainType)
	{
		switch (terrainType)
		{
		case TerrainEdit::TerrainType::noise_1:
		{
			return "noise_1";
		}
		break;
		case TerrainEdit::TerrainType::campaign_1:
		{
			return "campaign_1";
		}
		break;
		case TerrainEdit::TerrainType::campaign_2:
		{
			return "campaign_2";
		}
		break;
		default:
		{
			return "Unknown";
		}
		break;
		}
	}

	enum class TerrainEdit::TerrainType terrainTypeEnum(std::string& terrainType)
	{
		if (terrainType == "noise_1")
			return TerrainEdit::TerrainType::noise_1;
		else if (terrainType == "campaign_1")
			return TerrainEdit::TerrainType::campaign_1;
		else if (terrainType == "campaign_2")
			return TerrainEdit::TerrainType::campaign_2;
		else
			return TerrainEdit::TerrainType::unknown;
	}

	void TerrainEdit::serialize(TheWorld_Utils::MemoryBuffer& buffer)
	{
		buffer.reserve(sizeof(TerrainEdit));
		buffer.set((BYTE*)this, sizeof(TerrainEdit));
	}

	void TerrainEdit::deserialize(TheWorld_Utils::MemoryBuffer& buffer)
	{
		size_t minLen = (buffer.size() <= size ? buffer.size() : size);
		//assert(buffer.len() == sizeof(TerrainEdit));
		size_t saveSize = size;
		memcpy(this, buffer.ptr(), minLen);
		size = saveSize;
	}

	MemoryBuffer::MemoryBuffer(void)
	{
		m_ptr = nullptr;
		m_size = 0;
		m_bufferSize = 0;
	}

	MemoryBuffer::MemoryBuffer(BYTE* in, size_t len)
	{
		m_ptr = nullptr;
		m_size = 0;
		m_bufferSize = 0;

		set(in, len);
	}

	MemoryBuffer::~MemoryBuffer(void)
	{
		clear();
	}

	void MemoryBuffer::set(BYTE* in, size_t size)
	{
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSet 1 ") + std::to_string(len) + " " + __FUNCTION__, "reserve");
			reserve(size);
		}
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSet 2 ") + std::to_string(len) + " " + __FUNCTION__, "memcpy");
			memcpy(m_ptr, in, size);
		}
		m_size = size;
	}

	void MemoryBuffer::append(BYTE* in, size_t size)
	{
		size_t newLen = m_size + size;
		reserve(newLen);
		memcpy(m_ptr + m_size, in, size);
		m_size += size;
	}

	void MemoryBuffer::reserve(size_t size)
	{
		size_t newLen = size;
		if (newLen > m_bufferSize)
		{
			BYTE* ptr = (BYTE*)calloc(1, newLen);
			if (ptr == nullptr)
				throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
			m_bufferSize = newLen;

			memcpy(ptr, m_ptr, m_size);
			::free(m_ptr);
			m_ptr = ptr;
		}
	}

	size_t MemoryBuffer::reserved(void)
	{
		return m_bufferSize;
	}

	void MemoryBuffer::reset(void)
	{
		m_size = 0;
	}

	void MemoryBuffer::clear(void)
	{
		if (m_ptr != nullptr)
		{
			::free(m_ptr);
			m_ptr = nullptr;
			m_size = 0;
			m_bufferSize = 0;
		}
	}
	
	BYTE* MemoryBuffer::ptr()
	{
		return m_ptr;
	}
	size_t MemoryBuffer::size(void)
	{
		return m_size;
	}
	bool MemoryBuffer::empty(void)
	{
		return m_size == 0;
	}
	void MemoryBuffer::adjustSize(size_t size)
	{
		m_size = size;
	}

	void MemoryBuffer::populateFloatVector(std::vector<float>& v)
	{
		size_t numElements = size() / sizeof(float);
		my_assert(size() == numElements * sizeof(float));
		v.assign((float*)m_ptr, (float*)m_ptr + numElements);
	}

	void MemoryBuffer::populateUint16Vector(std::vector<uint16_t>& v)
	{
		size_t numElements = size() / sizeof(uint16_t);
		my_assert(size() == numElements * sizeof(uint16_t));
		v.assign((uint16_t*)m_ptr, (uint16_t*)m_ptr + numElements);
	}

	void Utils::plogInit(plog::Severity sev, plog::IAppender* appender)
	{
		plog::init(sev, appender);

		PLOG(plog::get()->getMaxSeverity()) << "***************";
		PLOG(plog::get()->getMaxSeverity()) << "Log initilized!";
		PLOG(plog::get()->getMaxSeverity()) << "***************";
	}

	void Utils::plogDenit(void)
	{
		PLOG(plog::get()->getMaxSeverity()) << "*****************";
		PLOG(plog::get()->getMaxSeverity()) << "Log Terminated!";
		PLOG(plog::get()->getMaxSeverity()) << "*****************";
	}

	void Utils::plogSetMaxSeverity(plog::Severity sev)
	{
		plog::get()->setMaxSeverity(sev);
		PLOG(plog::get()->getMaxSeverity()) << "Log severity changed to: " << std::to_string(sev);
	}

	MeshCacheBuffer::MeshCacheBuffer(void)
	{
		m_gridStepInWU = -1;
		m_numVerticesPerSize = -1;
		m_level = 0;
		m_lowerXGridVertex = 0;
		m_lowerZGridVertex = 0;
	}

	MeshCacheBuffer::~MeshCacheBuffer(void)
	{
	}
		
	MeshCacheBuffer::MeshCacheBuffer(std::string cacheDir, float gridStepInWU, size_t numVerticesPerSize, int level, float lowerXGridVertex, float lowerZGridVertex)
	{
		m_gridStepInWU = gridStepInWU;
		m_numVerticesPerSize = numVerticesPerSize;
		m_level = level;
		m_lowerXGridVertex = lowerXGridVertex;
		m_lowerZGridVertex = lowerZGridVertex;

		m_cacheDir = std::string(cacheDir) + "\\" + "Cache" + "\\" + "ST-" + std::to_string(gridStepInWU) + "_SZ-" + std::to_string(numVerticesPerSize) + "\\L-" + std::to_string(level);
		if (!fs::exists(m_cacheDir))
		{
			fs::create_directories(m_cacheDir);
		}
		std::string cacheFileName = "X-" + std::to_string(lowerXGridVertex) + "_Z-" + std::to_string(lowerZGridVertex) + ".mesh";
		m_meshFilePath = m_cacheDir + "\\" + cacheFileName;

	}
	
	MeshCacheBuffer::MeshCacheBuffer(const MeshCacheBuffer& c)
	{
		*this = c;
	}

	void MeshCacheBuffer::operator=(const MeshCacheBuffer& c)
	{
		m_meshFilePath = c.m_meshFilePath;
		m_cacheDir = c.m_cacheDir;
		m_meshId = c.m_meshId;
		//m_buffer = c.m_buffer;
		m_gridStepInWU = c.m_gridStepInWU;
		m_numVerticesPerSize = c.m_numVerticesPerSize;
		m_level = c.m_level;
		m_lowerXGridVertex = c.m_lowerXGridVertex;
		m_lowerZGridVertex = c.m_lowerZGridVertex;
	}

	std::string MeshCacheBuffer::generateNewMeshId(void)
	{
		GUID newId;
		RPC_STATUS ret_val = ::UuidCreate(&newId);
		if (ret_val != RPC_S_OK)
		{
			std::string msg = "UuidCreate in error with rc " + std::to_string(ret_val);
			PLOG_ERROR << msg;
			throw(GDN_TheWorld_Exception(__FUNCTION__, msg.c_str()));
		}

		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		tm local_tm;
		localtime_s(&local_tm , &now_time_t);
		BYTE shortBuffer[256 + 1];
		size_t size = 0;
		//TheWorld_Utils::serializeToByteStream<std::time_t>(now_time_t, shortBuffer, size);
		TheWorld_Utils::serializeToByteStream<std::chrono::system_clock::time_point>(now, shortBuffer, size);
		size_t n_zero = 2;
		std::string year = std::to_string(local_tm.tm_year + 1900);
		std::string month = std::to_string(local_tm.tm_mon + 1);
		month = std::string(n_zero - std::min(n_zero, month.length()), '0') + month;
		std::string day = std::to_string(local_tm.tm_mday);
		day = std::string(n_zero - std::min(n_zero, day.length()), '0') + day;
		std::string hours = std::to_string(local_tm.tm_hour);
		hours = std::string(n_zero - std::min(n_zero, hours.length()), '0') + hours;
		std::string minutes = std::to_string(local_tm.tm_min);
		minutes = std::string(n_zero - std::min(n_zero, minutes.length()), '0') + minutes;
		std::string seconds = std::to_string(local_tm.tm_sec);
		seconds = std::string(n_zero - std::min(n_zero, seconds.length()), '0') + seconds;
		std::string meshId = ToString(&newId)
			+ ":" + year + "-" + month  + "-" + day + "_" + hours + "-" + minutes + "-" + seconds
			+ ":" + std::string((char*)shortBuffer, size);

		return meshId;
	}

	bool MeshCacheBuffer::firstMeshIdMoreRecent(std::string firstMeshId, std::string secondMeshId)
	{
		size_t size = 0;
		//BYTE shortBuffer[256 + 1];

		std::chrono::system_clock::time_point firstTime;
		std::vector<std::string> v = TheWorld_Utils::Utils::split(firstMeshId, ':');
		assert(v.size() >= 1);
		if (v.size() > 1)
		{
			assert(v.size() > 2);
			size_t headerToDiscardSize = v[0].size() + v[1].size() + 2;
			std::string s = firstMeshId.substr(headerToDiscardSize);
			firstTime = TheWorld_Utils::deserializeFromByteStream<std::chrono::system_clock::time_point>((BYTE*)s.c_str(), size);
		}

		////time_t firstTime = 0;
		//size_t pos = firstMeshId.find(":");
		//if (pos != std::string::npos && pos + 1 < firstMeshId.size())
		//{
		//	memcpy(shortBuffer, &firstMeshId[pos + 1], firstMeshId.size() - pos - 1);
		//	//firstTime = TheWorld_Utils::deserializeFromByteStream<time_t>(shortBuffer, size);
		//	firstTime = TheWorld_Utils::deserializeFromByteStream<std::chrono::system_clock::time_point>(shortBuffer, size);
		//}

		std::chrono::system_clock::time_point secondTime;
		v = TheWorld_Utils::Utils::split(secondMeshId, ':');
		assert(v.size() >= 1);
		if (v.size() > 1)
		{
			assert(v.size() > 2);
			size_t headerToDiscardSize = v[0].size() + v[1].size() + 2;
			std::string s = secondMeshId.substr(headerToDiscardSize);
			secondTime = TheWorld_Utils::deserializeFromByteStream<std::chrono::system_clock::time_point>((BYTE*)s.c_str(), size);
		}

		////time_t secondTime = 0;
		//pos = secondMeshId.find(":");
		//if (pos != std::string::npos && pos + 1 < secondMeshId.size())
		//{
		//	memcpy(shortBuffer, &secondMeshId[pos + 1], secondMeshId.size() - pos - 1);
		//	//secondTime = TheWorld_Utils::deserializeFromByteStream<time_t>(shortBuffer, size);
		//	secondTime = TheWorld_Utils::deserializeFromByteStream<std::chrono::system_clock::time_point>(shortBuffer, size);
		//}

		//if (secondTime == 0)
		if (secondTime == std::chrono::system_clock::time_point())
			return true;

		//if (firstTime == 0)
		if (firstTime == std::chrono::system_clock::time_point())
			return false;

		if (firstTime == secondTime)
			return true;

		if (firstTime > secondTime)
			return true;
		else
			return false;

		//double diff = difftime(firstTime, secondTime);
		//size_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(firstTime - secondTime).count();

		//if (diff > 0)
		//	return true;
		//else
		//	return false;
	}

	std::string MeshCacheBuffer::getMeshIdFromCache(void)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer getMeshIdFromCache 1") + __FUNCTION__, "ALL");

		if (!fs::exists(m_meshFilePath))
			return "";
		
		BYTE shortBuffer[256 + 1];
		size_t size;

		// get size of a size_t
		size_t size_t_size = sizeof(size_t);

		FILE* inFile = nullptr;
		errno_t err = fopen_s(&inFile, m_meshFilePath.c_str(), "rb");
		if (err != 0)
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Open " + m_meshFilePath + " in errore - Err=" + std::to_string(err)).c_str()));

		if (fread(shortBuffer, size_t_size, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 2!").c_str()));
		}
		// and deserialize it
		size_t bufferSize = TheWorld_Utils::deserializeFromByteStream<size_t>(shortBuffer, size);

		if (fread(shortBuffer, 1, 1, inFile) != 1)	// "0"
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 1!").c_str()));
		}

		// read the serialized size of the mesh id
		if (fread(shortBuffer, size_t_size, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 2!").c_str()));
		}
		// and deserialize it
		size_t meshIdSize = TheWorld_Utils::deserializeFromByteStream<size_t>(shortBuffer, size);

		// read the mesh id
		if (fread(shortBuffer, meshIdSize, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 3!").c_str()));
		}
		// and deserialize it
		m_meshId = std::string((char*)shortBuffer, meshIdSize);

		fclose(inFile);

		return m_meshId;
	}

	bool MeshCacheBuffer::refreshMapsFromCache(std::string _meshId, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1 ") + __FUNCTION__, "ALL");

		TheWorld_Utils::MemoryBuffer buffer;

		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1.1  ") + __FUNCTION__, "readBufferFromCache");
			readBufferFromCache(_meshId, buffer);
		}

		if (buffer.size() == 0)
			return false;

		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1.2  ") + __FUNCTION__, "refreshMapsFromBuffer");
			std::string meshIdFromBuffer;
			refreshMapsFromBuffer(buffer, meshIdFromBuffer, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, false);
			assert(meshIdFromBuffer == _meshId);
			if (meshIdFromBuffer != _meshId)
			{
				throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("meshId from cache buffer (") + meshIdFromBuffer + ") not equal to meshId in input (" + _meshId).c_str()));
			}
		}

		return true;
	}

	void MeshCacheBuffer::refreshMapsFromBuffer(const BYTE* buffer, const size_t bufferSize, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool updateCache)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromBuffer 1 ") + __FUNCTION__, "ALL");

		if (bufferSize == 0)
		{
			meshIdFromBuffer.clear();
			return;
		}

		size_t size = 0;

		const char* movingStreamBuffer = (const char*)buffer;
		const char* endOfBuffer = movingStreamBuffer + bufferSize;

		movingStreamBuffer++;	// bypass "0"

		size_t meshIdSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		meshIdFromBuffer = std::string(movingStreamBuffer, meshIdSize);
		movingStreamBuffer += meshIdSize;

		m_meshId = meshIdFromBuffer;

		size_t terrainEditValuesSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		if (terrainEditValuesSize > 0)
		{
			terrainEditValues.set((BYTE*)movingStreamBuffer, terrainEditValuesSize);
			movingStreamBuffer += terrainEditValuesSize;
		}
		else
			terrainEditValues.reset();

		size_t vectSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16

		size_t float_size = sizeof(float);

		if (vectSize > 0)
		{
			minAltitude = TheWorld_Utils::deserializeFromByteStream<float>((BYTE*)movingStreamBuffer, size);
			movingStreamBuffer += size;

			maxAltitude = TheWorld_Utils::deserializeFromByteStream<float>((BYTE*)movingStreamBuffer, size);
			movingStreamBuffer += size;

			size_t float16HeightMapSize = vectSize * uint16_t_size;
			float16HeigthsBuffer.set((BYTE*)movingStreamBuffer, float16HeightMapSize);
			movingStreamBuffer += float16HeightMapSize;

			size_t float32HeightMapSize = vectSize * float_size;
			float32HeigthsBuffer.set((BYTE*)movingStreamBuffer, float32HeightMapSize);
			movingStreamBuffer += float32HeightMapSize;

			struct TheWorld_Utils::_RGB* rgb = (struct TheWorld_Utils::_RGB*)movingStreamBuffer;
			if (rgb->r == 0 && rgb->g == 0 && rgb->b == 0)
			{
				normalsBuffer.clear();
			}
			else
			{
				size_t normalMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGB);
				normalsBuffer.set((BYTE*)movingStreamBuffer, normalMapSize);
				movingStreamBuffer += normalMapSize;
			}

			if (updateCache)
				writeBufferToCache(buffer, bufferSize);
		}
		else
		{
			minAltitude = maxAltitude = 0;
			float16HeigthsBuffer.clear();
			float32HeigthsBuffer.clear();
			normalsBuffer.clear();
		}
	}

	void MeshCacheBuffer::refreshMapsFromBuffer(TheWorld_Utils::MemoryBuffer& buffer, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool updateCache)
	{
		refreshMapsFromBuffer(buffer.ptr(), buffer.size(), meshIdFromBuffer, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, updateCache);
	}

	void MeshCacheBuffer::refreshMapsFromBuffer(std::string& buffer, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool updateCache)
	{
		refreshMapsFromBuffer((BYTE*)buffer.c_str(), buffer.size(), meshIdFromBuffer, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, updateCache);
	}

	void MeshCacheBuffer::readBufferFromCache(std::string _meshId, TheWorld_Utils::MemoryBuffer& buffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer readBufferFromCache 1 ") + __FUNCTION__, "ALL");

		BYTE shortBuffer[256 + 1];
		size_t size;

		// read vertices from local cache
		if (!fs::exists(m_meshFilePath))
			return;
			//throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("not found current quadrant in cache").c_str()));

		size_t size_t_size = sizeof(size_t);	// get size of a size_t
		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16
		size_t float_size = sizeof(float);

		FILE* inFile = nullptr;
		errno_t err = fopen_s(&inFile, m_meshFilePath.c_str(), "rb");
		if (err != 0)
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Open " + m_meshFilePath + " in errore - Err=" + std::to_string(err)).c_str()));

		if (fread(shortBuffer, size_t_size, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 2!").c_str()));
		}
		// and deserialize it
		size_t streamBufferSize = TheWorld_Utils::deserializeFromByteStream<size_t>(shortBuffer, size);

		if (fread(shortBuffer, 1, 1, inFile) != 1)	// "0"
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 1!").c_str()));
		}

		// read the serialized size of the mesh id
		if (fread(shortBuffer, size_t_size, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 2!").c_str()));
		}
		// and deserialize it
		size_t meshIdSize = TheWorld_Utils::deserializeFromByteStream<size_t>(shortBuffer, size);

		// read the mesh id
		if (fread(shortBuffer, meshIdSize, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 3!").c_str()));
		}
		// and deserialize it
		std::string meshId((char*)shortBuffer, meshIdSize);

		if (meshId != _meshId)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("meshId from cache (") + meshId + ") not equal to meshId from server (" + _meshId).c_str()));
		}

		m_meshId = meshId;

		buffer.reserve(streamBufferSize);
		BYTE* streamBuffer = buffer.ptr();

		// reposition to the beginning of file
		int ret = fseek(inFile, 0, SEEK_SET);
		if (ret != 0)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("fseek to beginning of file error!").c_str()));
		}

		// bypass buffer size
		if (fread(shortBuffer, size_t_size, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 2!").c_str()));
		}

		size_t s = fread(streamBuffer, streamBufferSize, 1, inFile);

		fclose(inFile);

		buffer.adjustSize(streamBufferSize);
	}

	void MeshCacheBuffer::readBufferFromCache(std::string _meshId, std::string& _buffer)
	{
		TheWorld_Utils::MemoryBuffer buffer;

		readBufferFromCache(_meshId, buffer);
		
		size_t size = buffer.size();

		_buffer.clear();
		_buffer.reserve(size);
		_buffer.append((char*)buffer.ptr(), size);
	}
		
	void MeshCacheBuffer::writeBufferToCache(std::string& buffer)
	{
		writeBufferToCache((BYTE*)buffer.c_str(), buffer.size());
	}

	void MeshCacheBuffer::writeBufferToCache(TheWorld_Utils::MemoryBuffer& buffer)
	{
		writeBufferToCache(buffer.ptr(), buffer.size());
	}

	void MeshCacheBuffer::writeBufferToCache(const BYTE* buffer, const size_t bufferSize)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer writeBufferToCache 1 ") + __FUNCTION__, "ALL");

		std::string tempPath = m_meshFilePath + ".tmp";
		
		BYTE shortBuffer[256 + 1];
		size_t bufferSize_size = 0;	// get size of a size_t

		TheWorld_Utils::serializeToByteStream<size_t>(bufferSize, shortBuffer, bufferSize_size);

		FILE* outFile = nullptr;
		errno_t err = fopen_s(&outFile, tempPath.c_str(), "wb");
		if (err != 0)
		{
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Open " + m_meshFilePath + " in errore - Err=" + std::to_string(err)).c_str()));
		}

		if (fwrite(shortBuffer, bufferSize_size, 1, outFile) != 1)
		{
			fclose(outFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Write error 3!").c_str()));
		}

		if (fwrite(buffer, bufferSize, 1, outFile) != 1)
		{
			fclose(outFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Write error 3!").c_str()));
		}

		fclose(outFile);

		if (fs::exists(m_meshFilePath.c_str()))
			if (remove(m_meshFilePath.c_str()) != 0)
				throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Remove error!").c_str()));

		if (rename(tempPath.c_str(), m_meshFilePath.c_str()) != 0)
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Rename error!").c_str()));
	}
		
	void MeshCacheBuffer::generateNormals(size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, BYTE* normalsBuffer, const size_t normalsBufferSize, size_t& usedBufferSize)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals1 1 ") + __FUNCTION__, "ALL");

		usedBufferSize = 0;

		size_t requiredBufferSize = vectGridHeights.size() == 0 ? sizeof(struct TheWorld_Utils::_RGB) : vectGridHeights.size() * sizeof(struct TheWorld_Utils::_RGB);

		my_assert(vectGridHeights.size() == 0 || vectGridHeights.size() == numVerticesPerSize * numVerticesPerSize);
		my_assert(normalsBufferSize >= requiredBufferSize);

		struct TheWorld_Utils::_RGB* _tempNormalmapBuffer = (struct TheWorld_Utils::_RGB*)normalsBuffer;


		if (vectGridHeights.size() > 0)
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals1 1.2 ") + __FUNCTION__, "generation");

			size_t idx = 0;
			for (int z = 0; z < numVerticesPerSize; z++)			// m_heightMapImage->get_height()
			{
				for (int x = 0; x < numVerticesPerSize; x++)		// m_heightMapImage->get_width()
				{
					float h = vectGridHeights[idx];

					Eigen::Vector3d v;

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals1 1.2.1 ") + __FUNCTION__, "calc vector");

						// h = height of the point for which we are computing the normal
						// hr = height of the point on the rigth
						// hl = height of the point on the left
						// hf = height of the forward point (z growing)
						// hb = height of the backward point (z lessening)
						// step = step in WUs between points
						// we compute normal normalizing the vector (h - hr, step, h - hf) or (hl - h, step, hb - h)
						// according to https://hterrain-plugin.readthedocs.io/en/latest/ section "Procedural generation" it should be (h - hr, step, hf - h)
						//Eigen::Vector3d P((float)x, h, (float)z);	// Verify
						if (x < numVerticesPerSize - 1 && z < numVerticesPerSize - 1)
						{
							float hr = vectGridHeights[z * numVerticesPerSize + x + 1];
							float hf = vectGridHeights[(z + 1) * numVerticesPerSize + x];
							v = Eigen::Vector3d(h - hr, gridStepInWU, h - hf).normalized();
							//{		// Verify
							//	Eigen::Vector3d PR((float)(x + gridStepInWU), hr, (float)z);
							//	Eigen::Vector3d PF((float)x, hf, (float)(z + gridStepInWU));
							//	Eigen::Vector3d normal1 = (PF - P).cross(PR - P).normalized();
							//	if (!equal(normal1, normal))	// DEBUGRIC
							//		m_viewer->Globals()->debugPrint("Normal=" + String(normal) + " - Normal1= " + String(normal1));
							//}
						}
						else
						{
							if (x == numVerticesPerSize - 1 && z == 0)
							{
								float hf = vectGridHeights[(z + 1) * numVerticesPerSize + x];
								float hl = vectGridHeights[z * numVerticesPerSize + x - 1];
								v = Eigen::Vector3d(hl - h, gridStepInWU, h - hf).normalized();
								//{		// Verify
								//	Eigen::Vector3d PL((float)(x - gridStepInWU), hl, (float)z);
								//	Eigen::Vector3d PF((float)x, hf, (float)(z + gridStepInWU));
								//	Eigen::Vector3d normal1 = (PL - P).cross(PF - P).normalized();
								//	if (!equal(normal1, normal))	// DEBUGRIC
								//		m_viewer->Globals()->debugPrint("Normal=" + String(normal) + " - Normal1= " + String(normal1));
								//}
							}
							else if (x == 0 && z == numVerticesPerSize - 1)
							{
								float hr = vectGridHeights[z * numVerticesPerSize + x + 1];
								float hb = vectGridHeights[(z - 1) * numVerticesPerSize + x];
								v = Eigen::Vector3d(h - hr, gridStepInWU, hb - h).normalized();
								//{		// Verify
								//	Eigen::Vector3d PR((float)(x + gridStepInWU), hr, (float)z);
								//	Eigen::Vector3d PB((float)(x), hb, (float)(z - gridStepInWU));
								//	Eigen::Vector3d normal1 = (PR - P).cross(PB - P).normalized();
								//	if (!equal(normal1, normal))	// DEBUGRIC
								//		m_viewer->Globals()->debugPrint("Normal=" + String(normal) + " - Normal1= " + String(normal1));
								//}
							}
							else
							{
								float hl = vectGridHeights[z * numVerticesPerSize + x - 1];
								float hb = vectGridHeights[(z - 1) * numVerticesPerSize + x];
								v = Eigen::Vector3d(hl - h, gridStepInWU, hb - h).normalized();
								//{		// Verify
								//	Eigen::Vector3d PB((float)x, hb, (float)(z - gridStepInWU));
								//	Eigen::Vector3d PL((float)(x - gridStepInWU), hl, (float)z);
								//	Eigen::Vector3d normal1 = (PB - P).cross(PL - P).normalized();
								//	if (!equal(normal1, normal))	// DEBUGRIC
								//		m_viewer->Globals()->debugPrint("Normal=" + String(normal) + " - Normal1= " + String(normal1));
								//}
							}
						}
					}
					
					Eigen::Vector3d normal;
					
					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals1 1.2.2 ") + __FUNCTION__, "normalize");

						normal = v.normalized();
					}

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals1 1.2.3 ") + __FUNCTION__, "RGB");

						normal = 0.5 * (normal + Eigen::Vector3d(1, 1, 1));
						struct _RGB rgb;
						rgb.r = (BYTE)(normal.x() * 255);	// normals coord are from 0 to 1 but if expressed as color in a normlamap are from 0 to 255
						rgb.g = (BYTE)(normal.z() * 255);
						rgb.b = (BYTE)(normal.y() * 255);
						*_tempNormalmapBuffer = rgb;
						_tempNormalmapBuffer++;
					}

					idx++;
				}
			}
		}
		else
		{
			struct _RGB rgb;
			rgb.r = rgb.g = rgb.b = 0;
			*_tempNormalmapBuffer = rgb;
			_tempNormalmapBuffer++;
		}

		usedBufferSize = (BYTE*)_tempNormalmapBuffer - normalsBuffer;
	}

	void MeshCacheBuffer::generateNormals(size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, TheWorld_Utils::MemoryBuffer& normalsBuffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals2 1 ") + __FUNCTION__, "serialize normals");

		my_assert(vectGridHeights.size() == 0 || vectGridHeights.size() == numVerticesPerSize * numVerticesPerSize);

		size_t requiredBufferSize = vectGridHeights.size() == 0 ? sizeof(struct TheWorld_Utils::_RGB) : vectGridHeights.size() * sizeof(struct TheWorld_Utils::_RGB);

		normalsBuffer.reserve(requiredBufferSize);

		size_t usedBufferSize = 0;
		generateNormals(numVerticesPerSize, gridStepInWU, vectGridHeights, normalsBuffer.ptr(), normalsBuffer.reserved(), usedBufferSize);

		normalsBuffer.adjustSize(usedBufferSize);
	}

	void MeshCacheBuffer::setBufferFromCacheData(size_t numVerticesPerSize, float gridStepInWU, CacheData& cacheData, std::string& buffer)
	{
		TheWorld_Utils::MemoryBuffer _buffer;
		setBufferFromCacheData(numVerticesPerSize, gridStepInWU, cacheData, _buffer);
		buffer.clear();
		buffer.reserve(_buffer.size());
		buffer.assign((char*)_buffer.ptr(), _buffer.size());
	}

	void MeshCacheBuffer::setBufferFromCacheData(size_t numVerticesPerSize, float gridStepInWU, CacheData& cacheData, TheWorld_Utils::MemoryBuffer& buffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheData 1 ") + __FUNCTION__, "ALL");

		size_t size = 0;

		size_t numHeights16 = cacheData.heights16Buffer->size() / sizeof(uint16_t);
		my_assert(cacheData.heights16Buffer->size() == numHeights16 * sizeof(uint16_t));
		size_t numHeights32 = cacheData.heights32Buffer->size() / sizeof(float);
		my_assert(cacheData.heights32Buffer->size() == numHeights32 * sizeof(float));
		size_t numNormals = cacheData.normalsBuffer->size() / sizeof(TheWorld_Utils::_RGB);
		my_assert(cacheData.normalsBuffer->size() == numNormals * sizeof(TheWorld_Utils::_RGB));

		my_assert(numHeights16 == numHeights32);
		my_assert(numHeights16 == 0 || numHeights16 == numVerticesPerSize * numVerticesPerSize);
		my_assert(numNormals == 0 || numNormals == numVerticesPerSize * numVerticesPerSize);

		
		TheWorld_Utils::MemoryBuffer flatFloat16HeightsBuffer;
		TheWorld_Utils::MemoryBuffer* _flatFloat16HeightsBuffer = cacheData.heights16Buffer;
		TheWorld_Utils::MemoryBuffer flatFloat32HeightsBuffer;
		TheWorld_Utils::MemoryBuffer* _flatFloat32HeightsBuffer = cacheData.heights32Buffer;
		if (numHeights16 == 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheData 1.1 ") + __FUNCTION__, "gen flat heigths");

			_flatFloat16HeightsBuffer = &flatFloat16HeightsBuffer;
			_flatFloat32HeightsBuffer = &flatFloat32HeightsBuffer;

			numHeights32 = numHeights16 = numVerticesPerSize * numVerticesPerSize;
			
			size_t float16HeightsBufferSize = numVerticesPerSize * numVerticesPerSize * sizeof(uint16_t);
			flatFloat16HeightsBuffer.reserve(float16HeightsBufferSize);
			uint16_t* _float16HeightsBuffer = (uint16_t*)flatFloat16HeightsBuffer.ptr();
			size_t float32HeightsBufferSize = numVerticesPerSize * numVerticesPerSize * sizeof(float);
			flatFloat32HeightsBuffer.reserve(float32HeightsBufferSize);
			float* _float32HeightsBuffer = (float*)flatFloat32HeightsBuffer.ptr();
			TheWorld_Utils::FLOAT_32 f(0.0f);
			uint16_t empyHalf = half_from_float(f.u32);
			{		// first impl.
				for (size_t z = 0; z < numVerticesPerSize; z++)
					for (size_t x = 0; x < numVerticesPerSize; x++)
					{
						*_float16HeightsBuffer = empyHalf;
						_float16HeightsBuffer++;
						*_float32HeightsBuffer = 0.0f;
						_float32HeightsBuffer++;
					}
			}
			//{		// second impl.
			//	BYTE* _float16HeightsBufferBegin = (BYTE*)_float16HeightsBuffer;
			//	BYTE* _float32HeightsBufferBegin = (BYTE*)_float32HeightsBuffer;
			//	size_t numVerticesPerSizeMinusOne = numVerticesPerSize - 1;
			//	assert(TheWorld_Utils::Utils::isPowerOfTwo(int(numVerticesPerSizeMinusOne)));
			//	size_t exponent = (size_t)log2(numVerticesPerSizeMinusOne * numVerticesPerSizeMinusOne);
			//	*_float16HeightsBuffer = empyHalf;
			//	_float16HeightsBuffer++; 
			//	*_float32HeightsBuffer = 0.0f;
			//	_float32HeightsBuffer++;
			//	for (int i = 0; i < exponent; i++)
			//	{
			//		size_t length16 = (BYTE*)_float16HeightsBuffer - _float16HeightsBufferBegin;
			//		size_t length32 = (BYTE*)_float32HeightsBuffer - _float32HeightsBufferBegin;
			//		memcpy(_float16HeightsBuffer, _float16HeightsBufferBegin, length16);
			//		_float16HeightsBuffer = (uint16_t*)((BYTE*)_float16HeightsBuffer + length16);
			//		memcpy(_float32HeightsBuffer, _float32HeightsBufferBegin, length32);
			//		_float32HeightsBuffer = (float*)((BYTE*)_float32HeightsBuffer + length32);
			//	}
			//	size_t remaining = (numVerticesPerSize * numVerticesPerSize) - (numVerticesPerSizeMinusOne * numVerticesPerSizeMinusOne);
			//	memcpy(_float16HeightsBuffer, _float16HeightsBufferBegin, remaining * sizeof(uint16_t));
			//	_float16HeightsBuffer += remaining;
			//	memcpy(_float32HeightsBuffer, _float32HeightsBufferBegin, remaining * sizeof(float));
			//	_float32HeightsBuffer += remaining;
			//}
			assert((BYTE*)_float16HeightsBuffer - flatFloat16HeightsBuffer.ptr() == float16HeightsBufferSize);
			flatFloat16HeightsBuffer.adjustSize(float16HeightsBufferSize);
			assert((BYTE*)_float32HeightsBuffer - flatFloat32HeightsBuffer.ptr() == float32HeightsBufferSize);
			flatFloat32HeightsBuffer.adjustSize(float32HeightsBufferSize);
		}

		size_t float16HeightmapSize = numHeights16 * sizeof(uint16_t);
		size_t float32HeightmapSize = numHeights32 * sizeof(float);
		size_t normalmapSize = numNormals > 0 ? numNormals * sizeof(TheWorld_Utils::_RGB) : sizeof(TheWorld_Utils::_RGB);
		size_t streamBufferSize = 1 /* "0" */
			+ sizeof(size_t) + cacheData.meshId.length()
			+ sizeof(size_t) + cacheData.terrainEditValues->size()
			+ sizeof(size_t); /* numheigths */
		if (numHeights16 > 0)
		{
			streamBufferSize = streamBufferSize
				+ sizeof(float) /*min_altitude*/ 
				+ sizeof(float) /*max_altitude*/ 
				+ float16HeightmapSize 
				+ float32HeightmapSize 
				+ normalmapSize;
		}
		
		buffer.reserve(streamBufferSize);
		BYTE* _streamBuffer = buffer.ptr();

		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheData 1.2 ") + __FUNCTION__, "copy meshId/edit values");

			memcpy(_streamBuffer, "0", 1);
			_streamBuffer++;

			TheWorld_Utils::serializeToByteStream<size_t>(cacheData.meshId.length(), _streamBuffer, size);
			_streamBuffer += size;

			memcpy(_streamBuffer, cacheData.meshId.c_str(), cacheData.meshId.length());
			_streamBuffer += cacheData.meshId.length();

			TheWorld_Utils::serializeToByteStream<size_t>(cacheData.terrainEditValues->size(), _streamBuffer, size);
			_streamBuffer += size;

			if (cacheData.terrainEditValues->size() > 0)
			{
				memcpy(_streamBuffer, cacheData.terrainEditValues->ptr(), cacheData.terrainEditValues->size());
				_streamBuffer += cacheData.terrainEditValues->size();
			}
		}

		size = 0;
		TheWorld_Utils::serializeToByteStream<size_t>(numHeights16, _streamBuffer, size);
		_streamBuffer += size;

		if (numHeights16 > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheData 1.3 ") + __FUNCTION__, "copy heigths");

			TheWorld_Utils::serializeToByteStream<float>(cacheData.minHeight, _streamBuffer, size);
			_streamBuffer += size;

			TheWorld_Utils::serializeToByteStream<float>(cacheData.maxHeight, _streamBuffer, size);
			_streamBuffer += size;

			memcpy(_streamBuffer, _flatFloat16HeightsBuffer->ptr(), float16HeightmapSize);
			_streamBuffer += float16HeightmapSize;

			memcpy(_streamBuffer, _flatFloat32HeightsBuffer->ptr(), float32HeightmapSize);
			_streamBuffer += float32HeightmapSize;
		}

		if (numNormals > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheData 1.4 ") + __FUNCTION__, "copy normals");

			memcpy(_streamBuffer, cacheData.normalsBuffer->ptr(), normalmapSize);
			_streamBuffer += normalmapSize;
		}
		else
		{
			struct TheWorld_Utils::_RGB rgb;
			rgb.r = rgb.g = rgb.b = 0;
			memcpy(_streamBuffer, (BYTE*)&rgb, sizeof(struct TheWorld_Utils::_RGB));
			_streamBuffer += sizeof(struct TheWorld_Utils::_RGB);
		}

		assert(_streamBuffer - buffer.ptr() == streamBufferSize);
		buffer.adjustSize(streamBufferSize);
	}

	void MeshCacheBuffer::setEmptyBuffer(size_t numVerticesPerSize, float gridStepInWU, std::string& meshId, TheWorld_Utils::MemoryBuffer& buffer)
	{
		meshId = generateNewMeshId();
		
		TheWorld_Utils::MeshCacheBuffer::CacheData cacheData;
		cacheData.meshId = meshId;
		//BYTE shortBuffer[256 + 1];	size_t size = 0;
		//TheWorld_Utils::serializeToByteStream<size_t>(sizeof(size_t), shortBuffer, size);
		TheWorld_Utils::TerrainEdit terrainEdit;
		TheWorld_Utils::MemoryBuffer terrainEditValuesBuffer((BYTE*) & terrainEdit, sizeof(TheWorld_Utils::TerrainEdit));
		cacheData.minHeight = 0.0f;
		cacheData.maxHeight = 0.0f;
		cacheData.terrainEditValues = &terrainEditValuesBuffer;
		TheWorld_Utils::MemoryBuffer emptyFloat16HeightsBuffer;
		TheWorld_Utils::MemoryBuffer emptyFloat32HeightsBuffer;
		TheWorld_Utils::MemoryBuffer emptyNormalBuffer;
		cacheData.heights16Buffer = &emptyFloat16HeightsBuffer;
		cacheData.heights32Buffer = &emptyFloat32HeightsBuffer;
		cacheData.normalsBuffer = &emptyNormalBuffer;

		setBufferFromCacheData(numVerticesPerSize, gridStepInWU, cacheData, buffer);
	}

	void MeshCacheBuffer::setBufferFromHeights(std::string meshId, size_t numVerticesPerSize, float gridStepInWU, TheWorld_Utils::MemoryBuffer& terrainEditValuesBuffer, std::vector<float>& vectGridHeights, std::string& buffer, float& minAltitude, float& maxAltitude, bool generateNormals)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1 ") + __FUNCTION__, "ALL");

		// get size of a size_t
		size_t size_t_size = sizeof(size_t);
		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16
		size_t float_size = sizeof(float);

		size_t size = 0;

		size_t vectSize = vectGridHeights.size();
		size_t float16HeightmapSize = vectSize * uint16_t_size;
		size_t float32HeightmapSize = vectSize * float_size;
		size_t normalmapSize = generateNormals ? vectSize * sizeof(struct _RGB) : sizeof(struct _RGB);
		size_t streamBufferSize = 1 /* "0" */ 
			+ size_t_size + meshId.length()
			+ size_t_size + terrainEditValuesBuffer.size()
			+ size_t_size /* numheigths */ + float_size /*min_altitude*/ + float_size /*max_altitude*/ + float16HeightmapSize + float32HeightmapSize + normalmapSize;

		BYTE* streamBuffer = nullptr;
		BYTE* _streamBuffer = nullptr;
		BYTE* tempFloat16HeithmapBuffer = nullptr;
		uint16_t* _tempFloat16HeithmapBuffer = nullptr;
		BYTE* tempNormalmapBuffer = nullptr;
		struct _RGB* _tempNormalmapBuffer = nullptr;

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.1 ") + __FUNCTION__, "calloc");

			_streamBuffer = streamBuffer = (BYTE*)calloc(1, streamBufferSize);
			if (streamBuffer == nullptr)
				throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));

			if (float16HeightmapSize > 0)
			{
				_tempFloat16HeithmapBuffer = (uint16_t*)(tempFloat16HeithmapBuffer = (BYTE*)calloc(1, float16HeightmapSize));
				if (tempFloat16HeithmapBuffer == nullptr)
				{
					free(streamBuffer);
					throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
				}
			}

			if (normalmapSize > 0)
			{
				_tempNormalmapBuffer = (struct _RGB*)(tempNormalmapBuffer = (BYTE*)calloc(1, normalmapSize));
				if (tempNormalmapBuffer == nullptr)
				{
					free(streamBuffer);
					free(tempFloat16HeithmapBuffer);
					throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
				}
			}
		}

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.2 ") + __FUNCTION__, "meshId/edit values");

			memcpy(_streamBuffer, "0", 1);
			_streamBuffer++;

			TheWorld_Utils::serializeToByteStream<size_t>(meshId.length(), _streamBuffer, size);
			_streamBuffer += size;

			memcpy(_streamBuffer, meshId.c_str(), meshId.length());
			_streamBuffer += meshId.length();

			TheWorld_Utils::serializeToByteStream<size_t>(terrainEditValuesBuffer.size(), _streamBuffer, size);
			_streamBuffer += size;

			if (terrainEditValuesBuffer.size() > 0)
			{
				memcpy(_streamBuffer, terrainEditValuesBuffer.ptr(), terrainEditValuesBuffer.size());
				_streamBuffer += terrainEditValuesBuffer.size();
			}
		}

		size = 0;
		TheWorld_Utils::serializeToByteStream<size_t>(vectSize, _streamBuffer, size);
		_streamBuffer += size;

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.3 ") + __FUNCTION__, "loop heigths");
		
			minAltitude = 0, maxAltitude = 0;
			bool first = true;
			size_t idx = 0;
			if (vectSize > 0)
			{
				{
					//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.3.1 ") + __FUNCTION__, "serialize heigths");

					for (int z = 0; z < numVerticesPerSize; z++)			// m_heightMapImage->get_height()
					{
						for (int x = 0; x < numVerticesPerSize; x++)		// m_heightMapImage->get_width()
						{
							TheWorld_Utils::FLOAT_32 f;
							{

								// supposing heigths are ordered line by line
								f.f32 = vectGridHeights[idx];

								*_tempFloat16HeithmapBuffer = half_from_float(f.u32);
								_tempFloat16HeithmapBuffer++;
							}

							if (first)
							{
								first = false;
								minAltitude = maxAltitude = f.f32;
							}
							else
							{
								if (f.f32 > maxAltitude)
									maxAltitude = f.f32;
								if (f.f32 < minAltitude)
									minAltitude = f.f32;
							}

							idx++;
						}
					}
				}

				std::vector<float> heigthsEmptyVector;
				std::vector<float>* heigthsVectorPointer = &heigthsEmptyVector;
				if (generateNormals)
					heigthsVectorPointer = &vectGridHeights;

				size_t usedBufferSize = 0;
				this->generateNormals(numVerticesPerSize, gridStepInWU, *heigthsVectorPointer, (BYTE*)_tempNormalmapBuffer, normalmapSize, usedBufferSize);
				_tempNormalmapBuffer = (struct TheWorld_Utils::_RGB*)((BYTE*)_tempNormalmapBuffer + usedBufferSize);
				my_assert(normalmapSize == usedBufferSize);

				{
					//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.3.3 ") + __FUNCTION__, "memcpy");

					TheWorld_Utils::serializeToByteStream<float>(minAltitude, _streamBuffer, size);
					_streamBuffer += size;

					TheWorld_Utils::serializeToByteStream<float>(maxAltitude, _streamBuffer, size);
					_streamBuffer += size;

					if (float16HeightmapSize > 0)
					{
						size_t tempFloat16HeithmapBufferSize = (BYTE*)_tempFloat16HeithmapBuffer - tempFloat16HeithmapBuffer;
						assert(float16HeightmapSize == tempFloat16HeithmapBufferSize);
						memcpy(_streamBuffer, tempFloat16HeithmapBuffer, float16HeightmapSize);	// append normal map
						_streamBuffer += float16HeightmapSize;
					}

					if (float32HeightmapSize > 0)
					{
						memcpy(_streamBuffer, &vectGridHeights[0], float32HeightmapSize);	// append normal map
						_streamBuffer += float32HeightmapSize;
					}

					if (normalmapSize > 0)
					{
						size_t tempNormalmapBufferSize = (BYTE*)_tempNormalmapBuffer - tempNormalmapBuffer;
						assert(normalmapSize == tempNormalmapBufferSize);
						memcpy(_streamBuffer, tempNormalmapBuffer, normalmapSize);	// append normal map
						_streamBuffer += normalmapSize;
					}

					assert(streamBufferSize == _streamBuffer - streamBuffer);
				}
			}
		}

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.4 ") + __FUNCTION__, "copy out buffer");
			buffer = std::string((char*)streamBuffer, streamBufferSize);
		}
		
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.5 ") + __FUNCTION__, "free");
			if (float16HeightmapSize > 0)
				free(tempFloat16HeithmapBuffer);
			if (normalmapSize > 0)
				free(tempNormalmapBuffer);
			free(streamBuffer);
		}

		m_meshId = meshId;
	}

	void ThreadPool::Start(std::string label, size_t num_threads, /*const std::function<void()>* threadInitFunction, const std::function<void()>* threadDeinitFunction,*/ ThreadInitDeinit* threadInitDeinit)
	{
		m_label = label;
		m_threadInitDeinit = threadInitDeinit;
		m_lastDiagnosticTime = std::chrono::time_point_cast<TheWorld_Utils::MsTimePoint::duration>(std::chrono::system_clock::now());
		m_lastAllWorkingStatus = false;
		//m_threadInitFunction = threadInitFunction;
		//m_threadDeinitFunction = threadDeinitFunction;
		uint32_t _num_threads = (uint32_t)num_threads;
		if (_num_threads <= 0)
			_num_threads  = std::thread::hardware_concurrency(); // Max # of threads the system supports
		//const uint32_t num_threads = 2;
		m_threads.resize(_num_threads);
		for (uint32_t i = 0; i < _num_threads; i++)
		{
			m_threads.at(i) = std::thread(&ThreadPool::ThreadLoop, this);
		}
	}

	size_t ThreadPool::getNumWorkingThreads(size_t& m_maxThreads)
	{
		m_maxThreads = m_threads.size();
		return m_workingThreads;
	}
		
	bool ThreadPool::allThreadsWorking(void)
	{
		return m_workingThreads >= m_threads.size();
	}

	void ThreadPool::ThreadLoop()
	{
		if (m_threadInitDeinit != nullptr)
			m_threadInitDeinit->threadInit();
		//if (m_threadInitFunction != nullptr)
		//{
		//	(*m_threadInitFunction)();
		//}

		while (true)
		{
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(m_queue_mutex);
				m_mutex_condition.wait(lock, [this] { return !m_jobs.empty() || m_should_terminate; });
				if (m_should_terminate)
				{
					break;
				}
				job = m_jobs.front();
				m_jobs.pop();
				m_workingThreads++;

				TheWorld_Utils::MsTimePoint now = std::chrono::time_point_cast<TheWorld_Utils::MsTimePoint::duration>(std::chrono::system_clock::now());
				size_t numThreads = m_threads.size();
				bool allThreadsWorking = m_workingThreads >= numThreads;
				if ((now - m_lastDiagnosticTime).count() >= 1000)
				{
					if (allThreadsWorking)
						PLOG_INFO << "ThreadPool " << m_label << " has all threads working (" << std::to_string(m_workingThreads) << ":" << std::to_string(numThreads) << ") - Queue size: " << m_jobs.size();
					//else
					//	if (allThreadsWorking != m_lastAllWorkingStatus)
					//		PLOG_INFO << "ThreadPool " << m_label << " has free threads (" << std::to_string(m_workingThreads + 1) << ":" << std::to_string(numThreads) << ")";

					m_lastDiagnosticTime = now;
					m_lastAllWorkingStatus = allThreadsWorking;
				}
			}
			job();
			m_workingThreads--;
		}

		if (m_threadInitDeinit != nullptr)
			m_threadInitDeinit->threadDeinit();
		//if (m_threadDeinitFunction != nullptr)
		//{
		//	(*m_threadDeinitFunction)();
		//}
	}

	void ThreadPool::QueueJob(const std::function<void()>& job)
	{
		{
			std::unique_lock<std::mutex> lock(m_queue_mutex);
			m_jobs.push(job);
		}
		m_mutex_condition.notify_one();
	}
	
	bool ThreadPool::busy()
	{
		bool poolbusy;
		{
			std::unique_lock<std::mutex> lock(m_queue_mutex);
			poolbusy = !m_jobs.empty();
		}
		return poolbusy;
	}

	void ThreadPool::Stop()
	{
		{
			std::unique_lock<std::mutex> lock(m_queue_mutex);
			m_should_terminate = true;
		}
		m_mutex_condition.notify_all();
		for (std::thread& active_thread : m_threads) 
		{
			active_thread.join();
		}
		m_threads.clear();
	}
	
	std::string Utils::ReplaceString(std::string subject, const std::string& search, const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return subject;
	}

	void Utils::ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	std::string ToString(GUID* guid)
	{
		char guid_string[37]; // 32 hex chars + 4 hyphens + null terminator
		snprintf(
			guid_string, sizeof(guid_string),
			"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			guid->Data1, guid->Data2, guid->Data3,
			guid->Data4[0], guid->Data4[1], guid->Data4[2],
			guid->Data4[3], guid->Data4[4], guid->Data4[5],
			guid->Data4[6], guid->Data4[7]);
		return guid_string;
	}
}

