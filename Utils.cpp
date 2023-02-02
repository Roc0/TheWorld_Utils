//#include "pch.h"

//#define _THEWORLD_CLIENT

//#ifdef _THEWORLD_CLIENT
//	#include <Godot.hpp>
//	#include <ResourceLoader.hpp>
//	#include <File.hpp>
//#endif
#include <cstdio>
#include "Utils.h"
#include "Profiler.h"
#include "half.h"
#include <filesystem>
#include <string>
#include <plog/Initializers/RollingFileInitializer.h>
#include <Eigen/Dense>

namespace fs = std::filesystem;

namespace TheWorld_Utils
{
	MemoryBuffer::MemoryBuffer(void)
	{
		m_ptr = nullptr;
		m_len = 0;
		m_bufferLen = 0;
	}

	MemoryBuffer::~MemoryBuffer(void)
	{
		clear();
	}

	void MemoryBuffer::set(BYTE* in, size_t len)
	{
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSet 1 ") + std::to_string(len) + " " + __FUNCTION__, "reserve");
			reserve(len);
		}
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSet 2 ") + std::to_string(len) + " " + __FUNCTION__, "memcpy");
			memcpy(m_ptr, in, len);
		}
		m_len = len;

		//{
		//	char* p = nullptr;
		//	{
		//		TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSetTest 1 ") + std::to_string(len) + " " + __FUNCTION__, "calloc");
		//		p = (char*)calloc(1, len);
		//	}
		//	{
		//		TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSetTest 2 ") + std::to_string(len) + " " + __FUNCTION__, "memcpy");
		//		memcpy(p, in, len);
		//	}
		//	{
		//		TheWorld_Utils::GuardProfiler profiler(std::string("MemorybufferSetTest 3 ") + std::to_string(len) + " " + __FUNCTION__, "free");
		//		::free(p);
		//	}
		//}
	}

	void MemoryBuffer::append(BYTE* in, size_t len)
	{
		size_t newLen = m_len + len;
		reserve(newLen);
		memcpy(m_ptr + m_len, in, len);
		m_len += len;
	}

	void MemoryBuffer::reserve(size_t len)
	{
		size_t newLen = len;
		if (newLen > m_bufferLen)
		{
			BYTE* ptr = (BYTE*)calloc(1, newLen);
			if (ptr == nullptr)
				throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
			m_bufferLen = newLen;

			memcpy(ptr, m_ptr, m_len);
			::free(m_ptr);
			m_ptr = ptr;
		}
	}

	void MemoryBuffer::reset(void)
	{
		m_len = 0;
	}

	void MemoryBuffer::clear(void)
	{
		if (m_ptr != nullptr)
		{
			::free(m_ptr);
			m_ptr = nullptr;
			m_len = 0;
			m_bufferLen = 0;
		}
	}
	
	BYTE* MemoryBuffer::ptr()
	{
		return m_ptr;
	}
	size_t MemoryBuffer::len(void)
	{
		return m_len;
	}
	bool MemoryBuffer::empty(void)
	{
		return m_len == 0;
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

//#ifdef _THEWORLD_CLIENT
//		std::string heightmapFileName = "X-" + std::to_string(lowerXGridVertex) + "_Z-" + std::to_string(lowerZGridVertex) + "_heightmap.res";
//		m_heightmapFilePath = m_cacheDir + "\\" + heightmapFileName;
//		std::string normalmapFileName = "X-" + std::to_string(lowerXGridVertex) + "_Z-" + std::to_string(lowerZGridVertex) + "_normalmap.res";
//		m_normalmapFilePath = m_cacheDir + "\\" + normalmapFileName;
//#endif
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
		m_buffer = c.m_buffer;
		m_gridStepInWU = c.m_gridStepInWU;
		m_numVerticesPerSize = c.m_numVerticesPerSize;
		m_level = c.m_level;
		m_lowerXGridVertex = c.m_lowerXGridVertex;
		m_lowerZGridVertex = c.m_lowerZGridVertex;

//#ifdef _THEWORLD_CLIENT
//		m_heightmapFilePath = c.m_heightmapFilePath;
//		m_normalmapFilePath = c.m_normalmapFilePath;
//#endif
	}

	std::string MeshCacheBuffer::getMeshIdFromMeshCache(void)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer ") + __FUNCTION__, "ALL");

		if (!fs::exists(m_meshFilePath))
			return "";
		
		BYTE shortBuffer[256 + 1];
		size_t size;

		FILE* inFile = nullptr;
		errno_t err = fopen_s(&inFile, m_meshFilePath.c_str(), "rb");
		if (err != 0)
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Open " + m_meshFilePath + " in errore - Err=" + std::to_string(err)).c_str()));

		if (fread(shortBuffer, 1, 1, inFile) != 1)	// "0"
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 1!").c_str()));
		}

		// get size of a size_t
		size_t size_t_size = sizeof(size_t);
		//TheWorld_Utils::serializeToByteStream<size_t>(0, shortBuffer, size_t_size);

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

	void MeshCacheBuffer::refreshMapsFromBuffer(std::string buffer, std::string& meshIdFromBuffer, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool updateCache)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer ") + __FUNCTION__, "ALL");

		//BYTE shortBuffer[256 + 1];
		size_t size = 0;

		const char* movingStreamBuffer = buffer.c_str();
		const char* endOfBuffer = movingStreamBuffer + buffer.size();

		movingStreamBuffer++;	// bypass "0"

		size_t meshIdSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		meshIdFromBuffer = std::string(movingStreamBuffer, meshIdSize);
		movingStreamBuffer += meshIdSize;

		m_meshId = meshIdFromBuffer;
		m_buffer = buffer;

		size_t vectSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16
		//TheWorld_Utils::serializeToByteStream<uint16_t>(0, shortBuffer, uint16_t_size);

		size_t float_size = sizeof(float);

		if (vectSize > 0)
		{
			minAltitude = TheWorld_Utils::deserializeFromByteStream<float>((BYTE*)movingStreamBuffer, size);
			movingStreamBuffer += size;

			maxAltitude = TheWorld_Utils::deserializeFromByteStream<float>((BYTE*)movingStreamBuffer, size);
			movingStreamBuffer += size;

			size_t float16HeightMapSize = vectSize * uint16_t_size;
			//float16HeigthsBuffer = std::string((char*)movingStreamBuffer, float16HeightMapSize);
			float16HeigthsBuffer.set((BYTE*)movingStreamBuffer, float16HeightMapSize);
			movingStreamBuffer += float16HeightMapSize;

			size_t float32HeightMapSize = vectSize * float_size;
			//float32HeigthsBuffer = std::string((char*)movingStreamBuffer, float32HeightMapSize);
			float32HeigthsBuffer.set((BYTE*)movingStreamBuffer, float32HeightMapSize);
			movingStreamBuffer += float32HeightMapSize;

			size_t normalMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGB);
			//normalsBuffer = std::string((char*)movingStreamBuffer, normalMapSize);
			normalsBuffer.set((BYTE*)movingStreamBuffer, normalMapSize);
			movingStreamBuffer += normalMapSize;

			if (updateCache)
				writeBufferToMeshCache(buffer);
		}
		else
		{
			minAltitude = maxAltitude = 0;
			float16HeigthsBuffer.clear();
			float32HeigthsBuffer.clear();
			normalsBuffer.clear();
		}
	}

	void MeshCacheBuffer::readBufferFromMeshCache(std::string _meshId, std::string& buffer, size_t& vectSizeFromCache)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer ") + __FUNCTION__, "ALL");

		BYTE shortBuffer[256 + 1];
		size_t size;

		// read vertices from local cache
		if (!fs::exists(m_meshFilePath))
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("not found current quadrant in cache").c_str()));

		FILE* inFile = nullptr;
		errno_t err = fopen_s(&inFile, m_meshFilePath.c_str(), "rb");
		if (err != 0)
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Open " + m_meshFilePath + " in errore - Err=" + std::to_string(err)).c_str()));

		if (fread(shortBuffer, 1, 1, inFile) != 1)	// "0"
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 1!").c_str()));
		}

		size_t size_t_size = sizeof(size_t);	// get size of a size_t
		//TheWorld_Utils::serializeToByteStream<size_t>(0, shortBuffer, size_t_size);
		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16
		//TheWorld_Utils::serializeToByteStream<uint16_t>(0, shortBuffer, uint16_t_size);
		size_t float_size = sizeof(float);

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

		// read the serialized size of the vector of heigths
		if (fread(shortBuffer, size_t_size, 1, inFile) != 1)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Read error 4!").c_str()));
		}
		// and deserialize it
		vectSizeFromCache = TheWorld_Utils::deserializeFromByteStream<size_t>(shortBuffer, size);

		// alloc buffer to contain the serialized entire vector of GridVertex
		size_t float16HeightmapSize = vectSizeFromCache * uint16_t_size;
		size_t float32HeightmapSize = vectSizeFromCache * float_size;
		size_t normalmapSize = vectSizeFromCache * sizeof(struct _RGB);
		size_t streamBufferSize = 1 /* "0" */ + size_t_size + meshId.length() + size_t_size /* numheigths */ + float_size /*min_altitude*/ + float_size /*max_altitude*/ + float16HeightmapSize + float32HeightmapSize + normalmapSize;
		BYTE* streamBuffer = (BYTE*)calloc(1, streamBufferSize);
		if (streamBuffer == nullptr)
		{
			fclose(inFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Allocation error!").c_str()));
		}
		
		// reposition to the beginning of file
		int ret = fseek(inFile, 0, SEEK_SET);
		if (ret != 0)
		{
			fclose(inFile);
			::free(streamBuffer);
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("fseek to beginning of file error!").c_str()));
		}

		size_t s = fread(streamBuffer, streamBufferSize, 1, inFile);

		fclose(inFile);

		buffer = std::string((char*)streamBuffer, streamBufferSize);
		buffer.clear();
		buffer.reserve(streamBufferSize);
		buffer.append((char*)streamBuffer, streamBufferSize);

		m_buffer = buffer;

		::free(streamBuffer);
	}
		
	void MeshCacheBuffer::readMapsFromMeshCache(std::string _meshId, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("ReadFromCache 1 ") + __FUNCTION__, "ALL");

		//BYTE shortBuffer[256 + 1];
		std::string buffer;
		size_t vectSizeFromCache;
		size_t size;
		
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("ReadFromCache 1.1  ") + __FUNCTION__, "readBufferFromMeshCache");
			readBufferFromMeshCache(_meshId, buffer, vectSizeFromCache);
		}

		BYTE* movingStreamBuffer = (BYTE *)buffer.c_str();
		BYTE* endOfBuffer = (BYTE*)movingStreamBuffer + buffer.size();

		movingStreamBuffer++;	// bypass "0"

		size_t meshIdSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		std::string meshId = std::string((char*)movingStreamBuffer, meshIdSize);
		movingStreamBuffer += meshIdSize;

		if (meshId != _meshId)
		{
			throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("meshId from cache (") + meshId + ") not equal to meshId in input (" + _meshId).c_str()));
		}

		m_meshId = meshId;
		m_buffer = buffer;

		size_t vectSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
		//size_t heightsArraySize = (m_numVerticesPerSize * m_gridStepInWU) * (m_numVerticesPerSize * m_gridStepInWU);
		movingStreamBuffer += size;
		
		if (vectSize == 0)
			throw(GDN_TheWorld_Exception(__FUNCTION__, "Cache does not contain heigthmap/normlmap"));

		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16
		//TheWorld_Utils::serializeToByteStream<uint16_t>(0, shortBuffer, uint16_t_size);
		size_t float_size = sizeof(float);

		minAltitude = TheWorld_Utils::deserializeFromByteStream<float>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		maxAltitude = TheWorld_Utils::deserializeFromByteStream<float>((BYTE*)movingStreamBuffer, size);
		movingStreamBuffer += size;

		{
			TheWorld_Utils::GuardProfiler profiler(std::string("ReadFromCache 1.2  ") + __FUNCTION__, "copy output maps");

			size_t float16HeightMapSize = vectSize * uint16_t_size;
			float16HeigthsBuffer.set(movingStreamBuffer, float16HeightMapSize);
			movingStreamBuffer += float16HeightMapSize;

			size_t float32HeightMapSize = vectSize * float_size;
			float32HeigthsBuffer.set(movingStreamBuffer, float32HeightMapSize);
			movingStreamBuffer += float32HeightMapSize;

			size_t normalMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGB);
			normalsBuffer.set(movingStreamBuffer, normalMapSize);
			movingStreamBuffer += normalMapSize;
		}
	}

	void MeshCacheBuffer::writeBufferToMeshCache(std::string buffer)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer ") + __FUNCTION__, "ALL");

		std::string tempPath = m_meshFilePath + ".tmp";
		
		FILE* outFile = nullptr;
		errno_t err = fopen_s(&outFile, tempPath.c_str(), "wb");
		if (err != 0)
		{
			throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("Open " + m_meshFilePath + " in errore - Err=" + std::to_string(err)).c_str()));
		}

		if (fwrite(buffer.c_str(), buffer.size(), 1, outFile) != 1)
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
		
	void MeshCacheBuffer::setBufferForMeshCache(std::string meshId, size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, std::string& buffer)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer ") + __FUNCTION__, "ALL");

		//BYTE shortBuffer[256 + 1];

		// get size of a size_t
		size_t size_t_size = sizeof(size_t);
		//TheWorld_Utils::serializeToByteStream<size_t>(0, shortBuffer, size_t_size);
		size_t uint16_t_size = sizeof(uint16_t);	// the size of an half ==> float_16
		//TheWorld_Utils::serializeToByteStream<uint16_t>(0, shortBuffer, uint16_t_size);
		size_t float_size = sizeof(float);

		size_t vectSize = vectGridHeights.size();
		size_t float16HeightmapSize = vectSize * uint16_t_size;
		size_t float32HeightmapSize = vectSize * float_size;
		size_t normalmapSize = vectSize * sizeof(struct _RGB);
		size_t streamBufferSize = 1 /* "0" */ + size_t_size + meshId.length() + size_t_size /* numheigths */ + float_size /*min_altitude*/ + float_size /*max_altitude*/ + float16HeightmapSize + float32HeightmapSize + normalmapSize;

		BYTE* streamBuffer = (BYTE*)calloc(1, streamBufferSize);
		if (streamBuffer == nullptr)
			throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
		size_t streamBufferIterator = 0;

		BYTE* tempFloat16HeithmapBuffer = (BYTE*)calloc(1, float16HeightmapSize);
		if (tempFloat16HeithmapBuffer == nullptr)
		{
			free(streamBuffer);
			throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
		}
		size_t tempFloat16HeithmapBufferIterator = 0;

		BYTE* tempFloat32HeithmapBuffer = (BYTE*)calloc(1, float32HeightmapSize);
		if (tempFloat32HeithmapBuffer == nullptr)
		{
			free(streamBuffer);
			free(tempFloat16HeithmapBuffer);
			throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
		}
		size_t tempFloat32HeithmapBufferIterator = 0;

		BYTE* tempNormalmapBuffer = (BYTE*)calloc(1, normalmapSize);
		if (tempNormalmapBuffer == nullptr)
		{
			free(streamBuffer);
			free(tempFloat16HeithmapBuffer);
			free(tempFloat32HeithmapBuffer);
			throw(std::exception((std::string(__FUNCTION__) + std::string("Allocation error")).c_str()));
		}
		size_t tempNormalmapBufferIterator = 0;

		memcpy(streamBuffer + streamBufferIterator, "0", 1);
		streamBufferIterator++;

		size_t size = 0;
		TheWorld_Utils::serializeToByteStream<size_t>(meshId.length(), streamBuffer + streamBufferIterator, size);
		streamBufferIterator += size;

		memcpy(streamBuffer + streamBufferIterator, meshId.c_str(), meshId.length());
		streamBufferIterator += meshId.length();

		size = 0;
		TheWorld_Utils::serializeToByteStream<size_t>(vectSize, streamBuffer + streamBufferIterator, size);
		streamBufferIterator += size;

		float minAltitude = 0, maxAltitude = 0;
		bool first = true;
		size_t idx = 0;
		if (vectSize != 0)
		{
			for (int z = 0; z < numVerticesPerSize; z++)			// m_heightMapImage->get_height()
			{
				for (int x = 0; x < numVerticesPerSize; x++)		// m_heightMapImage->get_width()
				{
					// supposing heigths are ordered line by line
					TheWorld_Utils::FLOAT_32 f;
					f.f32 = vectGridHeights[idx];
					
					TheWorld_Utils::serializeToByteStream<uint16_t>(half_from_float(f.u32), tempFloat16HeithmapBuffer + tempFloat16HeithmapBufferIterator, size);
					tempFloat16HeithmapBufferIterator += size;

					TheWorld_Utils::serializeToByteStream<float>(f.f32, tempFloat32HeithmapBuffer + tempFloat32HeithmapBufferIterator, size);
					tempFloat32HeithmapBufferIterator += size;

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

					{
						// h = height of the point for which we are computing the normal
						// hr = height of the point on the rigth
						// hl = height of the point on the left
						// hf = height of the forward point (z growing)
						// hb = height of the backward point (z lessening)
						// step = step in WUs between points
						// we compute normal normalizing the vector (h - hr, step, h - hf) or (hl - h, step, hb - h)
						// according to https://hterrain-plugin.readthedocs.io/en/latest/ section "Procedural generation" it should be (h - hr, step, hf - h)
						Eigen::Vector3d normal;
						//Eigen::Vector3d P((float)x, h, (float)z);	// Verify
						if (x < numVerticesPerSize - 1 && z < numVerticesPerSize - 1)
						{
							float hr = vectGridHeights[z * numVerticesPerSize + x + 1];
							float hf = vectGridHeights[(z + 1) * numVerticesPerSize + x];
							normal = Eigen::Vector3d(f.f32 - hr, gridStepInWU, f.f32 - hf).normalized();
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
								normal = Eigen::Vector3d(hl - f.f32, gridStepInWU, f.f32 - hf).normalized();
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
								normal = Eigen::Vector3d(f.f32 - hr, gridStepInWU, hb - f.f32).normalized();
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
								normal = Eigen::Vector3d(hl - f.f32, gridStepInWU, hb - f.f32).normalized();
								//{		// Verify
								//	Eigen::Vector3d PB((float)x, hb, (float)(z - gridStepInWU));
								//	Eigen::Vector3d PL((float)(x - gridStepInWU), hl, (float)z);
								//	Eigen::Vector3d normal1 = (PB - P).cross(PL - P).normalized();
								//	if (!equal(normal1, normal))	// DEBUGRIC
								//		m_viewer->Globals()->debugPrint("Normal=" + String(normal) + " - Normal1= " + String(normal1));
								//}
							}
						}
						//Color ShaderTerrainData::encodeNormal(Vector3 normal)
						//{
						//	normal = 0.5 * (normal + Vector3::ONE);
						//	return Color(normal.x, normal.z, normal.y);
						//}
						normal = 0.5 * (normal + Eigen::Vector3d(1, 1, 1));
						struct _RGB rgb;
						rgb.r = (BYTE)(normal.x() * 255);	// normals coord are from 0 to 1 but if expressed as color in a normlamap are from 0 to 255
						rgb.g = (BYTE)(normal.z() * 255);
						rgb.b = (BYTE)(normal.y() * 255);
						TheWorld_Utils::serializeToByteStream<struct _RGB>(rgb, tempNormalmapBuffer + tempNormalmapBufferIterator, size);
						tempNormalmapBufferIterator += size;
					}

					idx++;
				}
			}

			TheWorld_Utils::serializeToByteStream<float>(minAltitude, streamBuffer + streamBufferIterator, size);
			streamBufferIterator += size;

			TheWorld_Utils::serializeToByteStream<float>(maxAltitude, streamBuffer + streamBufferIterator, size);
			streamBufferIterator += size;

			assert(float16HeightmapSize == tempFloat16HeithmapBufferIterator);
			memcpy(streamBuffer + streamBufferIterator, tempFloat16HeithmapBuffer, float16HeightmapSize);	// append normal map
			streamBufferIterator += float16HeightmapSize;

			assert(float32HeightmapSize == tempFloat32HeithmapBufferIterator);
			memcpy(streamBuffer + streamBufferIterator, tempFloat32HeithmapBuffer, float32HeightmapSize);	// append normal map
			streamBufferIterator += float32HeightmapSize;

			assert(normalmapSize == tempNormalmapBufferIterator);
			memcpy(streamBuffer + streamBufferIterator, tempNormalmapBuffer, normalmapSize);	// append normal map
			streamBufferIterator += normalmapSize;

			assert(streamBufferSize == streamBufferIterator);
		}

		buffer = std::string((char*)streamBuffer, streamBufferIterator);
		
		m_buffer = buffer;

		free(tempFloat16HeithmapBuffer);
		free(tempFloat32HeithmapBuffer);
		free(tempNormalmapBuffer);
		free(streamBuffer);

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

