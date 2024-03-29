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
	void to_lower_implace(std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	}
	std::string to_lower(std::string s)
	{
		std::string ret = s;
		std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
		return ret;
	}

	void TerrainEdit::generateGroundImage(MemoryBuffer& albedoBumpImage, MemoryBuffer& normalRoughnessImage, std::string groundTypeName, size_t imageSize, bool flipY, MemoryBuffer& colorImage, MemoryBuffer& bumpImage, MemoryBuffer& normalImage, MemoryBuffer& roughImage)
	{
		size_t width, heigth;
		double d;

		// Color image formal controls
		d = sqrt(colorImage.size() / sizeof(struct TheWorld_Utils::_RGB));
		if (colorImage.size() == 0)
			throw(std::exception((std::string(__FUNCTION__) + std::string("Color image empty")).c_str()));
		if (d * d * sizeof(struct TheWorld_Utils::_RGB) != colorImage.size())
			throw(std::exception((std::string(__FUNCTION__) + std::string("Color image length not valid")).c_str()));
		width = heigth = (size_t)d;

		// Bump/Displ/Depth image formal controls
		if (bumpImage.size() > 0)
		{
			d = sqrt(bumpImage.size());
			if (d * d != bumpImage.size())
				throw(std::exception((std::string(__FUNCTION__) + std::string("Normal image length not valid")).c_str()));
			if ((size_t)d != width)
				throw(std::exception((std::string(__FUNCTION__) + std::string("Normal image differs from color image size")).c_str()));
		}

		// Normal image formal controls
		if (normalImage.size() > 0)
		{
			d = sqrt(normalImage.size() / sizeof(struct TheWorld_Utils::_RGB));
			if (d * d * sizeof(struct TheWorld_Utils::_RGB) != normalImage.size())
				throw(std::exception((std::string(__FUNCTION__) + std::string("Normal image length not valid")).c_str()));
			if ((size_t)d != width)
				throw(std::exception((std::string(__FUNCTION__) + std::string("Normal image differs from color image size")).c_str()));
		}

		// Rough image formal controls
		if (roughImage.size() > 0)
		{
			d = sqrt(roughImage.size());
			if (d * d != roughImage.size())
				throw(std::exception((std::string(__FUNCTION__) + std::string("Rough image length not valid")).c_str()));
			if ((size_t)d != width)
				throw(std::exception((std::string(__FUNCTION__) + std::string("Rough image differs from color image size")).c_str()));
		}

		size_t bufferSize = width * heigth * sizeof(struct TheWorld_Utils::_RGBA);

		albedoBumpImage.clear();
		albedoBumpImage.reserve(bufferSize);
		albedoBumpImage.adjustSize(bufferSize);
		struct TheWorld_Utils::_RGBA* rgba = (struct TheWorld_Utils::_RGBA*)albedoBumpImage.ptr();
		struct TheWorld_Utils::_RGB* rgb = (struct TheWorld_Utils::_RGB*)colorImage.ptr();
		BYTE* a = bumpImage.size() > 0 ? bumpImage.ptr() : nullptr;
		for (size_t row = 0; row < heigth; row++)
			for (size_t col = 0; col < width; col++)
			{
				rgba->r = rgb->r;
				rgba->g = rgb->g;
				rgba->b = rgb->b;

				if (a != nullptr)
				{
					rgba->a = *a;
					a++;
				}
				else
					rgba->a = 255;

				rgba++;
				rgb++;
			}

		normalRoughnessImage.clear();
		if (normalImage.size() > 0 || roughImage.size() > 0)
		{
			normalRoughnessImage.reserve(bufferSize);
			normalRoughnessImage.adjustSize(bufferSize);
			struct TheWorld_Utils::_RGBA* rgba = (struct TheWorld_Utils::_RGBA*)normalRoughnessImage.ptr();
			struct TheWorld_Utils::_RGB* rgb = normalImage.size() > 0 ? (struct TheWorld_Utils::_RGB*)normalImage.ptr() : nullptr;
			BYTE* a = roughImage.size() > 0 ? roughImage.ptr() : nullptr;
			for (size_t row = 0; row < heigth; row++)
				for (size_t col = 0; col < width; col++)
				{
					if (rgb != nullptr)
					{
						rgba->r = rgb->r;
						if (flipY)
							rgba->g = 255 - rgb->g;
						else
							rgba->g = rgb->g;
						rgba->b = rgb->b;
						rgb++;
					}
					else
					{
						rgba->r = 127;
						rgba->g = 127;
						rgba->b = 255;
					}

					if (a != nullptr)
					{
						rgba->a = *a;
						a++;
					}
					else
						rgba->a = 255;

					rgba++;
				}
		}
	}

	void TerrainEdit::generateGroundImage(std::string outdir, std::string groundTypeName, size_t imageSize, bool flipY, MemoryBuffer& colorImage, MemoryBuffer& bumpImage, MemoryBuffer& normalImage, MemoryBuffer& roughImage)
	{
		MemoryBuffer albedoBumpImage;
		MemoryBuffer normalRoughnessImage;

		generateGroundImage(albedoBumpImage, normalRoughnessImage, groundTypeName, imageSize, flipY, colorImage, bumpImage, normalImage, roughImage);

		if (outdir.substr(outdir.size() - 1, 1) != "\\")
			outdir += "\\";
		FILE* outFile = nullptr;

		std::string albedoBumpImagePath = outdir + groundTypeName + "_albedo_bump.ground";
		errno_t err = fopen_s(&outFile, albedoBumpImagePath.c_str(), "wb");
		if (err != 0)
		{
			throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("Open ") + albedoBumpImagePath.c_str() + " in errore - Err=" + std::to_string(err)).c_str()));
		}

		if (fwrite(albedoBumpImage.ptr(), albedoBumpImage.size(), 1, outFile) != 1)
		{
			fclose(outFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("Error writing ") + albedoBumpImagePath.c_str()).c_str()));
		}

		fclose(outFile);

		std::string normalRoughnessImagePath = outdir + groundTypeName + "_normal_roughness.ground";
		err = fopen_s(&outFile, normalRoughnessImagePath.c_str(), "wb");
		if (err != 0)
		{
			throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("Open ") + normalRoughnessImagePath.c_str() + " in errore - Err=" + std::to_string(err)).c_str()));
		}

		if (fwrite(normalRoughnessImage.ptr(), normalRoughnessImage.size(), 1, outFile) != 1)
		{
			fclose(outFile);
			throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("Error writing ") + normalRoughnessImagePath.c_str()).c_str()));
		}

		fclose(outFile);
	}

	TerrainEdit::TerrainEdit(enum class TerrainEdit::TerrainType terrainType)
	{
		size = sizeof(TerrainEdit);
		needUploadToServer = false;
		normalsNeedRegen = true;

		TerrainEdit::terrainType = terrainType;
		init(terrainType);

		minHeight = 0;
		maxHeight = 0;

		eastSideXPlus.needBlend = true;
		eastSideXPlus.minHeight = 0.0f;
		eastSideXPlus.maxHeight = 0.0f;

		westSideXMinus.needBlend = true;
		westSideXMinus.minHeight = 0.0f;
		westSideXMinus.maxHeight = 0.0f;

		southSideZPlus.needBlend = true;
		southSideZPlus.minHeight = 0.0f;
		southSideZPlus.maxHeight = 0.0f;

		northSideZMinus.needBlend = true;
		northSideZMinus.minHeight = 0.0f;
		northSideZMinus.maxHeight = 0.0f;

		//memset(extraValues.lowElevationTexName_r, 0, sizeof(extraValues.lowElevationTexName_r));
		//memset(extraValues.highElevationTexName_g, 0, sizeof(extraValues.highElevationTexName_g));
		//memset(extraValues.dirtTexName_b, 0, sizeof(extraValues.dirtTexName_b));
		//memset(extraValues.rocksTexName_a, 0, sizeof(extraValues.rocksTexName_a));
		extraValues.splatmapNeedRegen = true;
		extraValues.emptyColormap = true;
		extraValues.emptyGlobalmap = true;
	}

	bool TerrainEdit::operator==(const TerrainEdit& o) const
	{
		if (size != o.size)
			return false;
		if (needUploadToServer != o.needUploadToServer)
			return false;
		if (normalsNeedRegen != o.normalsNeedRegen)
			return false;
		if (terrainType != o.terrainType)
			return false;
		if (minHeight != o.minHeight)
			return false;
		if (maxHeight != o.maxHeight)
			return false;
		if (!(eastSideXPlus == o.eastSideXPlus) )
			return false;
		if (!(westSideXMinus == o.westSideXMinus) )
			return false;
		if ( !(southSideZPlus == o.southSideZPlus) )
			return false;
		if ( !(northSideZMinus == o.northSideZMinus) )
			return false;
		if ( !(noise == o.noise) )
			return false;
		if (strlen(extraValues.lowElevationTexName_r) != strlen(o.extraValues.lowElevationTexName_r))
			return false;
		if (strcmp(extraValues.lowElevationTexName_r, o.extraValues.lowElevationTexName_r) != 0)
			return false;
		if (strlen(extraValues.highElevationTexName_g) != strlen(o.extraValues.highElevationTexName_g))
			return false;
		if (strcmp(extraValues.highElevationTexName_g, o.extraValues.highElevationTexName_g) != 0)
			return false;
		if (strlen(extraValues.dirtTexName_b) != strlen(o.extraValues.dirtTexName_b))
			return false;
		if (strcmp(extraValues.dirtTexName_b, o.extraValues.dirtTexName_b) != 0)
			return false;
		if (strlen(extraValues.rocksTexName_a) != strlen(o.extraValues.rocksTexName_a))
			return false;
		if (strcmp(extraValues.rocksTexName_a, o.extraValues.rocksTexName_a) != 0)
			return false;
		if (extraValues.splatmapNeedRegen != o.extraValues.splatmapNeedRegen)
			return false;
		if (extraValues.emptyColormap != o.extraValues.emptyColormap)
			return false;
		if (extraValues.emptyGlobalmap != o.extraValues.emptyGlobalmap)
			return false;
		return true;
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
		case TerrainEdit::TerrainType::plateau_1:
		{
			return "plateau_1";
		}
		break;
		case TerrainEdit::TerrainType::low_hills:
		{
			return "low_hills";
		}
		break;
		case TerrainEdit::TerrainType::high_hills:
		{
			return "high_hills";
		}
		break;
		case TerrainEdit::TerrainType::low_mountains:
		{
			return "low_mountains";
		}
		break;
		case TerrainEdit::TerrainType::low_mountains_grow:
		{
			return "low_mountains_grow";
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_1:
		{
			return "high_mountains_1";
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_1_grow:
		{
			return "high_mountains_1_grow";
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_2:
		{
			return "high_mountains_2";
		}
		break;
		case TerrainEdit::TerrainType::high_mountains_2_grow:
		{
			return "high_mountains_2_grow";
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
		else if (terrainType == "plateau_1")
			return TerrainEdit::TerrainType::plateau_1;
		else if (terrainType == "low_hills")
			return TerrainEdit::TerrainType::low_hills;
		else if (terrainType == "high_hills")
			return TerrainEdit::TerrainType::high_hills;
		else if (terrainType == "low_mountains")
			return TerrainEdit::TerrainType::low_mountains;
		else if (terrainType == "low_mountains_grow")
			return TerrainEdit::TerrainType::low_mountains_grow;
		else if (terrainType == "high_mountains_1")
			return TerrainEdit::TerrainType::high_mountains_1;
		else if (terrainType == "high_mountains_1_grow")
			return TerrainEdit::TerrainType::high_mountains_1_grow;
		else if (terrainType == "high_mountains_2")
			return TerrainEdit::TerrainType::high_mountains_2;
		else if (terrainType == "high_mountains_2_grow")
			return TerrainEdit::TerrainType::high_mountains_2_grow;
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

	void TerrainEdit::adjustValues(TerrainEdit* northSideZMinus, TerrainEdit* southSideZPlus, TerrainEdit* westSideXMinus, TerrainEdit* eastSideXPlus)
	{
		if (!noise.desideredMinHeigthMandatory)
		{
			int num = 0;
			float averageHeight = 0;

			if (northSideZMinus != nullptr)
			{
				averageHeight += (northSideZMinus->southSideZPlus.maxHeight + northSideZMinus->southSideZPlus.minHeight) / 2;
				num++;
			}
			if (southSideZPlus != nullptr)
			{
				averageHeight += (southSideZPlus->northSideZMinus.maxHeight + southSideZPlus->northSideZMinus.minHeight) / 2;
				num++;
			}
			if (westSideXMinus != nullptr)
			{
				averageHeight += (westSideXMinus->eastSideXPlus.maxHeight + westSideXMinus->eastSideXPlus.minHeight) / 2;
				num++;
			}
			if (eastSideXPlus != nullptr)
			{
				averageHeight += (eastSideXPlus->westSideXMinus.maxHeight + eastSideXPlus->westSideXMinus.minHeight) / 2;
				num++;
			}

			if (num != 0)
				noise.desideredMinHeight = averageHeight / num;
		}

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

	void MemoryBuffer::toString(std::string& buffer)
	{
		size_t sz = size();
		buffer.clear();
		buffer.reserve(sz);
		buffer.append((char*)ptr(), sz);
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

			if (m_ptr != nullptr)
			{
				memcpy(ptr, m_ptr, m_size);
				::free(m_ptr);
			}
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

	void MemoryBuffer::copyFrom(MemoryBuffer& inBuffer)
	{
		clear();
		reserve(inBuffer.reserved());
		memcpy(m_ptr, inBuffer.ptr(), inBuffer.size());
		m_size = inBuffer.size();
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
		
	MeshCacheBuffer::MeshCacheBuffer(std::string _cacheDir, std::string mapName, float gridStepInWU, size_t numVerticesPerSize, int level, float lowerXGridVertex, float lowerZGridVertex)
	{
		m_mapName = mapName;
		m_gridStepInWU = gridStepInWU;
		m_numVerticesPerSize = numVerticesPerSize;
		m_level = level;
		m_lowerXGridVertex = lowerXGridVertex;
		m_lowerZGridVertex = lowerZGridVertex;

		m_cacheDir = MeshCacheBuffer::cacheDir(_cacheDir, mapName, gridStepInWU, numVerticesPerSize, level);
		if (m_cacheDir.length() < 10)
		{
			std::string msg = std::string("Beccato " + _cacheDir + " " + mapName + " " + std::to_string(gridStepInWU) + " " + std::to_string(numVerticesPerSize) + " " + std::to_string(level) + " " + std::to_string(lowerXGridVertex) + " " + std::to_string(lowerZGridVertex));
			PLOG_ERROR << msg;
			throw(GDN_TheWorld_Exception(__FUNCTION__, msg.c_str()));
		}
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

	std::string MeshCacheBuffer::cacheDir(std::string cacheDir, std::string mapName, float gridStepInWU, size_t numVerticesPerSize, int level)
	{
		return std::string(cacheDir) + "\\" + "Cache\\maps\\" + mapName + "\\" + "ST-" + std::to_string(gridStepInWU) + "_SZ-" + std::to_string(numVerticesPerSize) + "\\L-" + std::to_string(level);
	}
		
	void MeshCacheBuffer::operator=(const MeshCacheBuffer& c)
	{
		m_meshFilePath = c.m_meshFilePath;
		m_cacheDir = c.m_cacheDir;
		m_meshId = c.m_meshId;
		//m_buffer = c.m_buffer;
		m_mapName = c.m_mapName;
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

	bool MeshCacheBuffer::isEmptyBuffer(std::string meshId)
	{
		my_assert(meshId.size() > 0);
		if (meshId.size() == 0)
			return true;

		std::vector<std::string> v = TheWorld_Utils::Utils::split(meshId, ':');
		my_assert(v.size() >= 3);
		if (v.size() < 3)
			return true;

		std::string temp = v[1].substr(0, 4);
		if (v[1].substr(0, 4) == "1970")
			return true;

		return false;
	}

	std::string MeshCacheBuffer::generateNewMeshIdForEmptyBuffer(void)
	{
		GUID newId;
		RPC_STATUS ret_val = ::UuidCreate(&newId);
		if (ret_val != RPC_S_OK)
		{
			std::string msg = "UuidCreate in error with rc " + std::to_string(ret_val);
			PLOG_ERROR << msg;
			throw(GDN_TheWorld_Exception(__FUNCTION__, msg.c_str()));
		}

		std::chrono::system_clock::time_point oldestTime;
		std::time_t oldest_time_t = std::chrono::system_clock::to_time_t(oldestTime);
		tm local_tm;
		localtime_s(&local_tm, &oldest_time_t);
		BYTE shortBuffer[256 + 1];
		size_t size = 0;
		TheWorld_Utils::serializeToByteStream<std::chrono::system_clock::time_point>(oldestTime, shortBuffer, size);
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
			+ ":" + year + "-" + month + "-" + day + "_" + hours + "-" + minutes + "-" + seconds
			+ ":" + std::string((char*)shortBuffer, size);

		return meshId;
	}

	bool MeshCacheBuffer::firstMeshIdMoreRecent(std::string firstMeshId, std::string secondMeshId)
	{
		size_t size = 0;
		//BYTE shortBuffer[256 + 1];

		std::chrono::system_clock::time_point firstTime;
		std::vector<std::string> v = TheWorld_Utils::Utils::split(firstMeshId, ':');
		my_assert(v.size() >= 1);
		if (v.size() > 1)
		{
			if (v.size() >= 4)
				assert(v.size() >= 4);	// DEBUGBREAKPOINT
			my_assert(v.size() > 2);
			size_t headerToDiscardSize = v[0].size() + v[1].size() + 2;
			std::string s = firstMeshId.substr(headerToDiscardSize);
			size_t sz = s.size();
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
		my_assert(v.size() >= 1);
		if (v.size() > 1)
		{
			if (v.size() >= 4)
				assert(v.size() >= 4);	// DEBUGBREAKPOINT
			my_assert(v.size() > 2);
			size_t headerToDiscardSize = v[0].size() + v[1].size() + 2;
			std::string s = secondMeshId.substr(headerToDiscardSize);
			size_t sz = s.size();
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

	void MeshCacheBuffer::getAllDiskCache(std::string _cacheDir, std::string mapName, float gridStepInWU, size_t numVerticesPerSize, int level, std::vector<MeshCacheBuffer>& vectDiskCache)
	{
		vectDiskCache.clear();

		std::string dirPath  = MeshCacheBuffer::cacheDir(_cacheDir, mapName, gridStepInWU, numVerticesPerSize, level);
		if (!fs::exists(dirPath))
			return;

		for (const auto& entry : fs::directory_iterator(dirPath))
			if (entry.is_regular_file())
			{
				std::string filename = entry.path().filename().generic_string();
				size_t pos = filename.find_last_of('.');
				if (pos != std::string::npos)
				{
					std::string filenameNoExt = filename.substr(0, pos);
					std::vector<std::string> v1 = TheWorld_Utils::Utils::split(filenameNoExt, '_');
					if (v1.size() == 2)
					{
						float lowerXGridVertex = FLT_MAX, lowerZGridVertex = FLT_MAX;

						pos = v1[0].find_first_of('-');
						if (pos != std::string::npos)
						{
							std::string coord = v1[0].substr(0, pos);
							std::string value = v1[0].substr(pos + 1);
							if (coord == "X")
								lowerXGridVertex = std::stof(value);
						}

						pos = v1[1].find_first_of('-');
						if (pos != std::string::npos)
						{
							std::string coord = v1[1].substr(0, pos);
							std::string value = v1[1].substr(pos + 1);
							if (coord == "Z")
								lowerZGridVertex = std::stof(value);
						}

						if (lowerXGridVertex != FLT_MAX && lowerZGridVertex != FLT_MAX)
						{
							vectDiskCache.push_back(MeshCacheBuffer(_cacheDir, mapName, gridStepInWU, numVerticesPerSize, level, lowerXGridVertex, lowerZGridVertex));
						}
					}
				}
			}
	}
	
	std::string MeshCacheBuffer::getMeshIdFromDisk(void)
	{
		//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer getMeshIdFromCache 1") + __FUNCTION__, "ALL");

		if (m_meshId.size() > 0)
			return m_meshId;
		
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

	std::string MeshCacheBuffer::getCacheIdStr(void)
	{
		return "MAP-" + m_mapName + "_ST-" + std::to_string(m_gridStepInWU) + "_SZ-" + std::to_string(m_numVerticesPerSize) + "_L-" + std::to_string(m_level) + "_X-" + std::to_string(m_lowerXGridVertex) + "_Z-" + std::to_string(m_lowerZGridVertex);
	}

	bool MeshCacheBuffer::refreshMapsFromDisk(size_t numVerticesPerSize, float gridStepInWU, std::string _meshId, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1 ") + __FUNCTION__, "ALL");

		TheWorld_Utils::MemoryBuffer buffer;

		bool empty = false;
		
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1.1  ") + __FUNCTION__, "readBufferFromCache");
			readBufferFromDisk(_meshId, buffer);
		}

		if (buffer.size() == 0)
		{
			TheWorld_Utils::MemoryBuffer tempBuffer;

			{
				TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1.2 ") + __FUNCTION__, "setEmptyBuffer");
				setEmptyBuffer(numVerticesPerSize, _meshId, tempBuffer);
			}

			{
				TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1.3 ") + __FUNCTION__, "readMapsFromMeshCache");

				refreshMapsFromBuffer(tempBuffer, _meshId, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, splatmapBuffer, colormapBuffer, globalmapBuffer, false);
			}

			empty = true;
		}
		else
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer refreshMapsFromCache 1.4  ") + __FUNCTION__, "refreshMapsFromBuffer");
			std::string meshIdFromBuffer;
			refreshMapsFromBuffer(buffer, meshIdFromBuffer, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, splatmapBuffer, colormapBuffer, globalmapBuffer, false);
			assert(meshIdFromBuffer == _meshId);
			if (meshIdFromBuffer != _meshId)
			{
				throw(GDN_TheWorld_Exception(__FUNCTION__, (std::string("meshId from cache buffer (") + meshIdFromBuffer + ") not equal to meshId in input (" + _meshId).c_str()));
			}
		}

		return empty;
	}

	void MeshCacheBuffer::refreshMapsFromBuffer(const BYTE* buffer, const size_t bufferSize, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer, bool updateCache)
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
				movingStreamBuffer += sizeof(struct TheWorld_Utils::_RGB);
			}
			else
			{
				size_t normalMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGB);
				normalsBuffer.set((BYTE*)movingStreamBuffer, normalMapSize);
				movingStreamBuffer += normalMapSize;
			}

			if (movingStreamBuffer + sizeof(struct TheWorld_Utils::_RGBA) <= endOfBuffer)
			{
				struct TheWorld_Utils::_RGBA* rgba = (struct TheWorld_Utils::_RGBA*)movingStreamBuffer;
				if (rgba->r == 0 && rgba->g == 0 && rgba->b == 0 && rgba->a == 0)
				{
					splatmapBuffer.clear();
					movingStreamBuffer += sizeof(struct TheWorld_Utils::_RGBA);
				}
				else
				{
					size_t splatmapMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGBA);
					splatmapBuffer.set((BYTE*)movingStreamBuffer, splatmapMapSize);
					movingStreamBuffer += splatmapMapSize;
				}
			}
			else
			{
				splatmapBuffer.clear();
			}
			
			if (movingStreamBuffer + sizeof(struct TheWorld_Utils::_RGBA) <= endOfBuffer)
			{
				struct TheWorld_Utils::_RGBA* rgba = (struct TheWorld_Utils::_RGBA*)movingStreamBuffer;
				if ((rgba->r == 0 && rgba->g == 0 && rgba->b == 0 && rgba->a == 0) /* || (rgba->r == 255 && rgba->g == 255 && rgba->b == 255 && rgba->a == 255)*/)
				{
					colormapBuffer.clear();
					movingStreamBuffer += sizeof(struct TheWorld_Utils::_RGBA);
				}
				else
				{
					size_t colormapMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGBA);
					colormapBuffer.set((BYTE*)movingStreamBuffer, colormapMapSize);
					movingStreamBuffer += colormapMapSize;
				}
			}
			else
			{
				colormapBuffer.clear();
			}

			if (movingStreamBuffer + sizeof(struct TheWorld_Utils::_RGB) <= endOfBuffer)
			{
				struct TheWorld_Utils::_RGB* rgb = (struct TheWorld_Utils::_RGB*)movingStreamBuffer;
				if ((rgb->r == 0 && rgb->g == 0 && rgb->b == 0) /* || (rgb->r == 255 && rgb->g == 255 && rgb->b == 255)*/)
				{
					globalmapBuffer.clear();
					movingStreamBuffer += sizeof(struct TheWorld_Utils::_RGB);
				}
				else
				{
					size_t globalmapMapSize = vectSize * sizeof(struct TheWorld_Utils::_RGB);
					globalmapBuffer.set((BYTE*)movingStreamBuffer, globalmapMapSize);
					movingStreamBuffer += globalmapMapSize;
				}
			}
			else
			{
				globalmapBuffer.clear();
			}

			if (updateCache)
				writeBufferToDiskCache(buffer, bufferSize);
		}
		else
		{
			minAltitude = maxAltitude = 0;
			float16HeigthsBuffer.clear();
			float32HeigthsBuffer.clear();
			normalsBuffer.clear();
			splatmapBuffer.clear();
			colormapBuffer.clear();
			globalmapBuffer.clear();
		}
	}

	void MeshCacheBuffer::refreshMapsFromBuffer(TheWorld_Utils::MemoryBuffer& buffer, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer, bool updateCache)
	{
		refreshMapsFromBuffer(buffer.ptr(), buffer.size(), meshIdFromBuffer, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, splatmapBuffer, colormapBuffer, globalmapBuffer, updateCache);
	}

	void MeshCacheBuffer::refreshMapsFromBuffer(std::string& buffer, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitude, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer, bool updateCache)
	{
		refreshMapsFromBuffer((BYTE*)buffer.c_str(), buffer.size(), meshIdFromBuffer, terrainEditValues, minAltitude, maxAltitude, float16HeigthsBuffer, float32HeigthsBuffer, normalsBuffer, splatmapBuffer, colormapBuffer, globalmapBuffer, updateCache);
	}

	void MeshCacheBuffer::refreshCacheQuadrantDataFromBuffer(const BYTE* buffer, const size_t bufferSize, CacheQuadrantData& cacheQuadrantData, bool updateCache)
	{
		refreshMapsFromBuffer(buffer, bufferSize, cacheQuadrantData.meshId, *cacheQuadrantData.terrainEditValues, cacheQuadrantData.minHeight, cacheQuadrantData.maxHeight, *cacheQuadrantData.heights16Buffer, *cacheQuadrantData.heights32Buffer, *cacheQuadrantData.normalsBuffer, *cacheQuadrantData.splatmapBuffer, *cacheQuadrantData.colormapBuffer, *cacheQuadrantData.globalmapBuffer, updateCache);
	}

	void MeshCacheBuffer::refreshCacheQuadrantDataFromBuffer(TheWorld_Utils::MemoryBuffer& buffer, CacheQuadrantData& cacheQuadrantData, bool updateCache)
	{
		refreshCacheQuadrantDataFromBuffer(buffer.ptr(), buffer.size(), cacheQuadrantData, updateCache);
	}

	void MeshCacheBuffer::refreshCacheQuadrantDataFromBuffer(std::string& buffer, CacheQuadrantData& cacheQuadrantData, bool updateCache)
	{
		refreshCacheQuadrantDataFromBuffer((BYTE*)buffer.c_str(), buffer.size(), cacheQuadrantData, updateCache);
	}

	void MeshCacheBuffer::readBufferFromDisk(std::string _meshId, TheWorld_Utils::MemoryBuffer& buffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer readBufferFromCache 1 ") + __FUNCTION__, "ALL");

		BYTE shortBuffer[256 + 1];
		size_t size;

		// read vertices from local cache
		if (!fs::exists(m_meshFilePath))
		{
			buffer.clear();
			return;
			//throw(GDN_TheWorld_Exception(__FUNCTION__, std::string("not found current quadrant in cache").c_str()));
		}

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

		if (_meshId.length() > 0 && meshId != _meshId)
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

	void MeshCacheBuffer::readBufferFromDisk(std::string _meshId, std::string& _buffer)
	{
		TheWorld_Utils::MemoryBuffer buffer;

		readBufferFromDisk(_meshId, buffer);
		
		size_t size = buffer.size();

		_buffer.clear();
		_buffer.reserve(size);
		_buffer.append((char*)buffer.ptr(), size);
	}
		
	void MeshCacheBuffer::writeBufferToDiskCache(std::string& buffer, bool renewMeshId)
	{
		writeBufferToDiskCache((BYTE*)buffer.c_str(), buffer.size(), renewMeshId);
	}

	void MeshCacheBuffer::writeBufferToDiskCache(TheWorld_Utils::MemoryBuffer& buffer, bool renewMeshId)
	{
		writeBufferToDiskCache(buffer.ptr(), buffer.size(), renewMeshId);
	}

	void MeshCacheBuffer::writeBufferToDiskCache(const BYTE* buffer, const size_t bufferSize, bool renewMeshId)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer writeBufferToCache 1 ") + __FUNCTION__, "ALL");

		std::string tempPath = m_meshFilePath + ".tmp";
		
		BYTE shortBuffer[256 + 1];
		size_t bufferSize_size = 0;	// get size of a size_t

		if (renewMeshId)
		{
			std::string newMeshId = generateNewMeshId();

			char* movingStreamBuffer = (char*)buffer;
			char* endOfBuffer = movingStreamBuffer + bufferSize;

			movingStreamBuffer++;	// bypass "0"

			size_t size = 0;
			size_t meshIdSize = TheWorld_Utils::deserializeFromByteStream<size_t>((BYTE*)movingStreamBuffer, size);
			movingStreamBuffer += size;

			my_assert(meshIdSize == newMeshId.size())
			memcpy(movingStreamBuffer, newMeshId.c_str(), meshIdSize);
		}
				
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

	void MeshCacheBuffer::generateHeightsWithNoise(size_t numVerticesPerSize, float gridStepInWU, float lowerXGridVertex, float lowerZGridVertex, TerrainEdit* terrainEdit, std::vector<float>& vectGridHeights)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateHeights 1 ") + __FUNCTION__, "ALL");

		vectGridHeights.clear();
		vectGridHeights.resize(numVerticesPerSize * numVerticesPerSize);

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateHeights 1.1 ") + __FUNCTION__, "Generate jag");

			TheWorld_Utils::TerrainEdit terrainEdit(TheWorld_Utils::TerrainEdit::TerrainType::jagged_1);
			
			FastNoiseLite noiseLite(terrainEdit.noise.noiseSeed);
			noiseLite.SetNoiseType((enum FastNoiseLite::NoiseType)terrainEdit.noise.noiseType);
			noiseLite.SetFrequency(terrainEdit.noise.frequency);
			noiseLite.SetFractalType((enum FastNoiseLite::FractalType)terrainEdit.noise.fractalType);
			noiseLite.SetFractalOctaves(terrainEdit.noise.fractalOctaves);
			noiseLite.SetFractalLacunarity(terrainEdit.noise.fractalLacunarity);
			noiseLite.SetFractalGain(terrainEdit.noise.fractalGain);
			noiseLite.SetFractalWeightedStrength(terrainEdit.noise.fractalWeightedStrength);
			noiseLite.SetFractalPingPongStrength(terrainEdit.noise.fractalPingPongStrength);

			size_t idx = 0;
			for (int z = 0; z < numVerticesPerSize; z++)
				for (int x = 0; x < numVerticesPerSize; x++)
				{
					float altitude = 0.0f;

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("EditGenerate 1.1.2 ") + __FUNCTION__, "GetNoise");

						float xf = (lowerXGridVertex + (x * gridStepInWU));
						float zf = (lowerZGridVertex + (z * gridStepInWU));
						altitude = noiseLite.GetNoise(xf, zf);
						// noises are value in range -1 to 1 we need to interpolate with amplitude
						altitude *= (terrainEdit.noise.amplitude / 2);
					}

					vectGridHeights[idx] = altitude;

					idx++;
				}		
		}

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateHeights 1.2 ") + __FUNCTION__, "Generate Heights");

			FastNoiseLite noiseLite(terrainEdit->noise.noiseSeed);
			noiseLite.SetNoiseType((enum FastNoiseLite::NoiseType)terrainEdit->noise.noiseType);
			//noiseLite.SetRotationType3D();
			noiseLite.SetFrequency(terrainEdit->noise.frequency);
			noiseLite.SetFractalType((enum FastNoiseLite::FractalType)terrainEdit->noise.fractalType);
			noiseLite.SetFractalOctaves(terrainEdit->noise.fractalOctaves);
			noiseLite.SetFractalLacunarity(terrainEdit->noise.fractalLacunarity);
			noiseLite.SetFractalGain(terrainEdit->noise.fractalGain);
			noiseLite.SetFractalWeightedStrength(terrainEdit->noise.fractalWeightedStrength);
			noiseLite.SetFractalPingPongStrength(terrainEdit->noise.fractalPingPongStrength);
			//noiseLite.SetCellularDistanceFunction();
			//noiseLite.SetCellularReturnType();
			//noiseLite.SetCellularJitter()

			terrainEdit->minHeight = FLT_MAX;
			terrainEdit->maxHeight = -FLT_MAX;
			terrainEdit->eastSideXPlus.minHeight = FLT_MAX;
			terrainEdit->eastSideXPlus.maxHeight = -FLT_MAX;
			terrainEdit->westSideXMinus.minHeight = FLT_MAX;
			terrainEdit->westSideXMinus.maxHeight = -FLT_MAX;
			terrainEdit->southSideZPlus.minHeight = FLT_MAX;
			terrainEdit->southSideZPlus.maxHeight = -FLT_MAX;
			terrainEdit->northSideZMinus.minHeight = FLT_MAX;
			terrainEdit->northSideZMinus.maxHeight = -FLT_MAX;

			size_t maxIndex = numVerticesPerSize - 1;
			
			size_t idx = 0;
			for (int z = 0; z < numVerticesPerSize; z++)
				for (int x = 0; x < numVerticesPerSize; x++)
				{
					float altitude = 0.0f;

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("EditGenerate 1.2.2 ") + __FUNCTION__, "GetNoise main");

						float xf = (lowerXGridVertex + (x * gridStepInWU)) * terrainEdit->noise.scaleFactor;
						float zf = (lowerZGridVertex + (z * gridStepInWU)) * terrainEdit->noise.scaleFactor;
						altitude = noiseLite.GetNoise(xf, zf);
						// noises are value in range -1 to 1 we need to interpolate with amplitude
						altitude *= (terrainEdit->noise.amplitude / 2);

						if (altitude < terrainEdit->minHeight)
							terrainEdit->minHeight = altitude;
						if (altitude > terrainEdit->maxHeight)
							terrainEdit->maxHeight = altitude;

						if (x == 0)
						{
							if (altitude < terrainEdit->westSideXMinus.minHeight)
								terrainEdit->westSideXMinus.minHeight = altitude;
							if (altitude > terrainEdit->westSideXMinus.maxHeight)
								terrainEdit->westSideXMinus.maxHeight = altitude;
						}

						if (z == 0)
						{
							if (altitude < terrainEdit->northSideZMinus.minHeight)
								terrainEdit->northSideZMinus.minHeight = altitude;
							if (altitude > terrainEdit->northSideZMinus.maxHeight)
								terrainEdit->northSideZMinus.maxHeight = altitude;
						}

						if (x == maxIndex)
						{
							if (altitude < terrainEdit->eastSideXPlus.minHeight)
								terrainEdit->eastSideXPlus.minHeight = altitude;
							if (altitude > terrainEdit->eastSideXPlus.maxHeight)
								terrainEdit->eastSideXPlus.maxHeight = altitude;
						}

						if (z == maxIndex)
						{
							if (altitude < terrainEdit->southSideZPlus.minHeight)
								terrainEdit->southSideZPlus.minHeight = altitude;
							if (altitude > terrainEdit->southSideZPlus.maxHeight)
								terrainEdit->southSideZPlus.maxHeight = altitude;
						}
					}

					vectGridHeights[idx] += altitude;

					idx++;
				}
		}

		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("EditGenerate 1.3 ") + __FUNCTION__, "Adjust desidered min height");

			if (terrainEdit->noise.desideredMinHeight != FLT_MIN)
			{
				float corrector = terrainEdit->noise.desideredMinHeight - terrainEdit->minHeight;
				size_t idx = 0;
				for (int z = 0; z < numVerticesPerSize; z++)
					for (int x = 0; x < numVerticesPerSize; x++)
					{
						vectGridHeights[idx] += corrector;
						idx++;
					}

				terrainEdit->minHeight += corrector;
				terrainEdit->maxHeight += corrector;

				terrainEdit->eastSideXPlus.minHeight += corrector;
				terrainEdit->eastSideXPlus.maxHeight += corrector;

				terrainEdit->westSideXMinus.minHeight += corrector;
				terrainEdit->westSideXMinus.maxHeight += corrector;

				terrainEdit->southSideZPlus.minHeight += corrector;
				terrainEdit->southSideZPlus.maxHeight += corrector;

				terrainEdit->northSideZMinus.minHeight += corrector;
				terrainEdit->northSideZMinus.maxHeight += corrector;
			}
		}
	}
		
	void MeshCacheBuffer::applyWorldModifier(int level, size_t numVerticesPerSize, float gridStepInWU, float lowerXGridVertex, float lowerZGridVertex, TerrainEdit* terrainEdit, std::vector<float>& vectGridHeights, WorldModifier& wm)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer applyWorldModifier 1 ") + __FUNCTION__, "ALL");

		if (level != wm.getPos().getLevel())
			return;

		float sizeInWU = (numVerticesPerSize - 1) * gridStepInWU;

		float AOE = wm.getAOE();
		float wmX = wm.getPos().getPosX();
		float wmZ = wm.getPos().getPosZ();
		float wmMin = wm.getMin();
		float wmMax = wm.getMax();
		enum class TheWorld_Utils::WMType wmType = wm.getPos().getType();
		enum class TheWorld_Utils::WMFunctionType wmFunctionType = wm.getFunctionType();
		enum class TheWorld_Utils::WMOrder wmOrder = wm.getOrder();
		float wmMaxExtension = wmMax - wmMin;
		float wmStrength = wmMax - wmMin;

		if (wmType == TheWorld_Utils::WMType::none)
			return;

		if (AOE > 0)
		{
			float d1 = TheWorld_Utils::Utils::getDistance(wmX, wmZ, lowerXGridVertex, lowerZGridVertex);
			float d2 = TheWorld_Utils::Utils::getDistance(wmX, wmZ, lowerXGridVertex + sizeInWU, lowerZGridVertex);
			float d3 = TheWorld_Utils::Utils::getDistance(wmX, wmZ, lowerXGridVertex, lowerZGridVertex + sizeInWU);
			float d4 = TheWorld_Utils::Utils::getDistance(wmX, wmZ, lowerXGridVertex + sizeInWU, lowerZGridVertex + sizeInWU);

			if (d1 > AOE && d2 > AOE && d3 > AOE && d4 > AOE)
				return;
		}

		terrainEdit->eastSideXPlus.minHeight = FLT_MAX;
		terrainEdit->eastSideXPlus.maxHeight = -FLT_MAX;

		terrainEdit->westSideXMinus.minHeight = FLT_MAX;
		terrainEdit->westSideXMinus.maxHeight = -FLT_MAX;

		terrainEdit->southSideZPlus.minHeight = FLT_MAX;
		terrainEdit->southSideZPlus.maxHeight = -FLT_MAX;

		terrainEdit->northSideZMinus.minHeight = FLT_MAX;
		terrainEdit->northSideZMinus.maxHeight = -FLT_MAX;

		size_t maxIndex = numVerticesPerSize - 1;

		size_t idx = -1;
		for (int z = 0; z < numVerticesPerSize; z++)
			for (int x = 0; x < numVerticesPerSize; x++)
			{
				idx++;

				if (x == maxIndex && z == 0)
				{
					assert(x == maxIndex && z == 0);
				}
				if (x == 0 && z == maxIndex)
				{
					assert(x == 0 && z == maxIndex);
				}
				if (x == maxIndex && z == maxIndex)
				{
					assert(x == maxIndex && z == maxIndex);
				}

				float dist = TheWorld_Utils::Utils::getDistance(wmX, wmZ, (float)x * gridStepInWU + lowerXGridVertex, (float)z * gridStepInWU + lowerZGridVertex);

				if (dist > AOE)
				{
					if (idx == 0)
					{
						terrainEdit->minHeight = terrainEdit->maxHeight = vectGridHeights[idx];
					}
					else
					{
						if (vectGridHeights[idx] < terrainEdit->minHeight)
							terrainEdit->minHeight = vectGridHeights[idx];
						if (vectGridHeights[idx] > terrainEdit->maxHeight)
							terrainEdit->maxHeight = vectGridHeights[idx];
					}
					
					if (x == 0)
					{
						if (vectGridHeights[idx] < terrainEdit->westSideXMinus.minHeight)
							terrainEdit->westSideXMinus.minHeight = vectGridHeights[idx];
						if (vectGridHeights[idx] > terrainEdit->westSideXMinus.maxHeight)
							terrainEdit->westSideXMinus.maxHeight = vectGridHeights[idx];
					}

					if (z == 0)
					{
						if (vectGridHeights[idx] < terrainEdit->northSideZMinus.minHeight)
							terrainEdit->northSideZMinus.minHeight = vectGridHeights[idx];
						if (vectGridHeights[idx] > terrainEdit->northSideZMinus.maxHeight)
							terrainEdit->northSideZMinus.maxHeight = vectGridHeights[idx];
					}

					if (x == maxIndex)
					{
						if (vectGridHeights[idx] < terrainEdit->eastSideXPlus.minHeight)
							terrainEdit->eastSideXPlus.minHeight = vectGridHeights[idx];
						if (vectGridHeights[idx] > terrainEdit->eastSideXPlus.maxHeight)
							terrainEdit->eastSideXPlus.maxHeight = vectGridHeights[idx];
					}

					if (z == maxIndex)
					{
						if (vectGridHeights[idx] < terrainEdit->southSideZPlus.minHeight)
							terrainEdit->southSideZPlus.minHeight = vectGridHeights[idx];
						if (vectGridHeights[idx] > terrainEdit->southSideZPlus.maxHeight)
							terrainEdit->southSideZPlus.maxHeight = vectGridHeights[idx];
					}

					continue;
				}

				switch (wmType)
				{
				case TheWorld_Utils::WMType::elevator:
				{
					// TheWorld_Utils::WMType::elevator
					switch (wmFunctionType)
					{
						// TheWorld_Utils::WMType::elevator - TheWorld_Utils::WMFunctionType::ConsiderMinMax
					case TheWorld_Utils::WMFunctionType::ConsiderMinMax:
					{
						switch (wmOrder)
						{
							// TheWorld_Utils::WMType::elevator - TheWorld_Utils::WMFunctionType::ConsiderMinMax - TheWorld_Utils::WMOrder::MaxEffectOnWM
						case TheWorld_Utils::WMOrder::MaxEffectOnWM:
						{
							float coeff = (AOE - dist) / AOE;	// 0 on perimeter, 1 on WM
							float addition = wmMaxExtension * coeff + wmMin;
							vectGridHeights[idx] += addition;
						}
						break;
						// TheWorld_Utils::WMType::elevator - TheWorld_Utils::WMFunctionType::ConsiderMinMax - TheWorld_Utils::WMOrder::MinEffectOnWM
						case TheWorld_Utils::WMOrder::MinEffectOnWM:
						{
							float coeff = dist / AOE;			// 1 on perimeter, 0 on WM
							float addition = wmMaxExtension * coeff + wmMin;
							vectGridHeights[idx] += addition;
						}
						break;
						}
					}
					break;

					// TheWorld_Utils::WMType::elevator - TheWorld_Utils::WMFunctionType::ConsiderStrength
					case TheWorld_Utils::WMFunctionType::ConsiderStrength:
					{
						switch (wmOrder)
						{
							// TheWorld_Utils::WMType::elevator - TheWorld_Utils::WMFunctionType::ConsiderStrength - TheWorld_Utils::WMOrder::MaxEffectOnWM
						case TheWorld_Utils::WMOrder::MaxEffectOnWM:
						{
							float coeff = (AOE - dist) / AOE;	// 0 on perimeter, 1 on WM
							float addition = wmStrength * coeff;
							vectGridHeights[idx] += addition;
						}
						break;

						// TheWorld_Utils::WMType::elevator - TheWorld_Utils::WMFunctionType::ConsiderStrength - TheWorld_Utils::WMOrder::MinEffectOnWM
						case TheWorld_Utils::WMOrder::MinEffectOnWM:
						{
							float coeff = dist / AOE;			// 1 on perimeter, 0 on WM
							float addition = wmMaxExtension * coeff;
							vectGridHeights[idx] += addition;
						}
						break;
						}
					}
					break;
					}
				}
				break;
				}

				if (idx == 0)
				{
					terrainEdit->minHeight = terrainEdit->maxHeight = vectGridHeights[idx];
				}
				else
				{
					if (vectGridHeights[idx] < terrainEdit->minHeight)
						terrainEdit->minHeight = vectGridHeights[idx];
					if (vectGridHeights[idx] > terrainEdit->maxHeight)
						terrainEdit->maxHeight = vectGridHeights[idx];
				}

				if (x == 0)
				{
					if (vectGridHeights[idx] < terrainEdit->westSideXMinus.minHeight)
						terrainEdit->westSideXMinus.minHeight = vectGridHeights[idx];
					if (vectGridHeights[idx] > terrainEdit->westSideXMinus.maxHeight)
						terrainEdit->westSideXMinus.maxHeight = vectGridHeights[idx];
				}

				if (z == 0)
				{
					if (vectGridHeights[idx] < terrainEdit->northSideZMinus.minHeight)
						terrainEdit->northSideZMinus.minHeight = vectGridHeights[idx];
					if (vectGridHeights[idx] > terrainEdit->northSideZMinus.maxHeight)
						terrainEdit->northSideZMinus.maxHeight = vectGridHeights[idx];
				}

				if (x == maxIndex)
				{
					if (vectGridHeights[idx] < terrainEdit->eastSideXPlus.minHeight)
						terrainEdit->eastSideXPlus.minHeight = vectGridHeights[idx];
					if (vectGridHeights[idx] > terrainEdit->eastSideXPlus.maxHeight)
						terrainEdit->eastSideXPlus.maxHeight = vectGridHeights[idx];
				}

				if (z == maxIndex)
				{
					if (vectGridHeights[idx] < terrainEdit->southSideZPlus.minHeight)
						terrainEdit->southSideZPlus.minHeight = vectGridHeights[idx];
					if (vectGridHeights[idx] > terrainEdit->southSideZPlus.maxHeight)
						terrainEdit->southSideZPlus.maxHeight = vectGridHeights[idx];
				}
			}
	}
		
	void MeshCacheBuffer::generateNormalsForBlendedQuadrants(size_t numVerticesPerSize, float gridStepInWU, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& east_float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& south_float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool& requestedExit)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals 1 ") + __FUNCTION__, "ALL");

		size_t numVertices = float32HeigthsBuffer.size() / sizeof(float);
		my_assert(float32HeigthsBuffer.size() == numVertices * sizeof(float));
		my_assert(numVertices == 0 || numVertices == numVerticesPerSize * numVerticesPerSize);

		size_t requiredBufferSize = numVertices == 0 ? sizeof(struct TheWorld_Utils::_RGB) : numVertices * sizeof(struct TheWorld_Utils::_RGB);
		normalsBuffer.clear();
		normalsBuffer.reserve(requiredBufferSize);
		normalsBuffer.adjustSize(requiredBufferSize);

		struct TheWorld_Utils::_RGB* _tempNormalmapBuffer = (struct TheWorld_Utils::_RGB*)normalsBuffer.ptr();

		size_t maxIndex = numVerticesPerSize - 1;

		if (numVertices > 0)
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals 1.2 ") + __FUNCTION__, "generation");

			for (int z = 0; z < numVerticesPerSize; z++)			// m_heightMapImage->get_height()
			{
				for (int x = 0; x < numVerticesPerSize; x++)		// m_heightMapImage->get_width()
				{
					if (requestedExit)
						return;
					
					float h = float32HeigthsBuffer.at<float>(x, z, numVerticesPerSize);

					Eigen::Vector3d normal;

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals 1.2.1 ") + __FUNCTION__, "calc vector");

						// h = height of the point for which we are computing the normal
						// hr = height of the point on the rigth (x growing)
						// hl = height of the point on the left (x lessening)
						// hf = height of the forward point (z growing)
						// hb = height of the backward point (z lessening)
						// step = step in WUs between points
						// we compute normal normalizing the vector (h - hr, step, h - hf) or (hl - h, step, hb - h)
						// according to https://hterrain-plugin.readthedocs.io/en/latest/ section "Procedural generation" it should be (h - hr, step, hf - h)
						
						//Eigen::Vector3d P(float(x) * gridStepInWU, h, float(z) * gridStepInWU);	// Verify
						
						if (x < maxIndex && z < maxIndex)
						{
							float hr = float32HeigthsBuffer.at<float>(x + 1, z, numVerticesPerSize);
							float hf = float32HeigthsBuffer.at<float>(x, z + 1, numVerticesPerSize);
							normal = Eigen::Vector3d(h - hr, gridStepInWU, h - hf).normalized();
																																							//normal = Eigen::Vector3d(h - hr, gridStepInWU, hf - h).normalized();
							//{	// Verify
							//	float nx = float(normal.x());
							//	float ny = float(normal.y());
							//	float nz = float(normal.z());
							//	Eigen::Vector3d PR(float(x) * gridStepInWU + gridStepInWU, hr, float(z) * gridStepInWU);
							//	Eigen::Vector3d PF(float(x) * gridStepInWU, hf, float(z) * gridStepInWU + gridStepInWU);
							//	Eigen::Vector3d normal1 = (PF - P).cross(PR - P).normalized();
							//	float n1x = float(normal1.x());
							//	float n1y = float(normal1.y());
							//	float n1z = float(normal1.z());
							//	if (!TheWorld_Utils::Utils::isEqualVectorWithLimitedPrecision(normal1, normal, 7))	// DEBUGRIC
							//		normal1 = normal;
							//}	// Verify
						}
						else if (x == maxIndex && z == maxIndex)
						{
							if (east_float32HeigthsBuffer.size() > 0 && south_float32HeigthsBuffer.size() > 0)
							{
								float hr = east_float32HeigthsBuffer.at<float>(1, z, numVerticesPerSize);
								float hf = south_float32HeigthsBuffer.at<float>(x, 1, numVerticesPerSize);
								normal = Eigen::Vector3d(h - hr, gridStepInWU, h - hf).normalized();
							}
							else
							{
								float hl = float32HeigthsBuffer.at<float>(x - 1, z, numVerticesPerSize);
								float hb = float32HeigthsBuffer.at<float>(x, z - 1, numVerticesPerSize);
								normal = Eigen::Vector3d(hl - h, gridStepInWU, hb - h).normalized();
							}
						}
						else if (x == maxIndex)	// && z < maxIndex
						{
							if (east_float32HeigthsBuffer.size() > 0)
							{
								float hr = east_float32HeigthsBuffer.at<float>(1, z, numVerticesPerSize);
								float hf = float32HeigthsBuffer.at<float>(x, z + 1, numVerticesPerSize);
								normal = Eigen::Vector3d(h - hr, gridStepInWU, h - hf).normalized();
							}
							else
							{
								float hl = float32HeigthsBuffer.at<float>(x - 1, z, numVerticesPerSize);
								float hf = float32HeigthsBuffer.at<float>(x, z + 1, numVerticesPerSize);
								normal = Eigen::Vector3d(hl - h, gridStepInWU, h - hf).normalized();
							}
						}
						else	// x < maxIndex && z == maxIndex
						{
							if (south_float32HeigthsBuffer.size() > 0)
							{
								float hr = float32HeigthsBuffer.at<float>(x + 1, z, numVerticesPerSize);
								float hf = south_float32HeigthsBuffer.at<float>(x, 1, numVerticesPerSize);
								normal = Eigen::Vector3d(h - hr, gridStepInWU, h - hf).normalized();
							}
							else
							{
								float hr = float32HeigthsBuffer.at<float>(x + 1, z, numVerticesPerSize);
								float hb = float32HeigthsBuffer.at<float>(x, z - 1, numVerticesPerSize);
								normal = Eigen::Vector3d(h - hr, gridStepInWU, hb - h).normalized();
							}
						}
					}

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer generateNormals 1.2.3 ") + __FUNCTION__, "RGB");

								//float nx2 = float(normal.x());		// DEBUG ONLY
								//float ny2 = float(normal.y());		// DEBUG ONLY
								//float nz2 = float(normal.z());		// DEBUG ONLY
						Eigen::Vector3d packedNormal = packNormal(normal);
								//float nx = float(packedNormal.x());	// DEBUG ONLY
								//float ny = float(packedNormal.y());	// DEBUG ONLY
								//float nz = float(packedNormal.z());	// DEBUG ONLY
						//Eigen::Vector3d unpackedNormal = unpackNormal(packedNormal);	// DEBUG ONLY
						//		float nx1 = float(unpackedNormal.x());	// DEBUG ONLY
						//		float ny1 = float(unpackedNormal.y());	// DEBUG ONLY
						//		float nz1 = float(unpackedNormal.z());	// DEBUG ONLY
						struct _RGB rgb;
						rgb.r = (BYTE)(packedNormal.x() * 255);	// normals coords range from 0 to 1 but if expressed as color in a normalmap range from 0 to 255
						rgb.g = (BYTE)(packedNormal.y() * 255);
						rgb.b = (BYTE)(packedNormal.z() * 255);
						*_tempNormalmapBuffer = rgb;
						_tempNormalmapBuffer++;
					}
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

		my_assert((BYTE*)_tempNormalmapBuffer - normalsBuffer.ptr() == normalsBuffer.size());
	}

	void MeshCacheBuffer::generateSplatmap(size_t numVerticesPerSize, float gridStepInWU, TheWorld_Utils::TerrainEdit* terrainEdit, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer,
											float slopeVerticalFactor,	// raising it slope will have higher maximum and so rocks will be more diffuse on slopes
											float slopeFlatFactor,		// raising it slope will have lesser mininum and maximum so rocks will be less diffuse on slopes
											float dirtOnRocksFactor,	// raising it dirt will be more diffuse on rocks
											float highElevationFactor,	// raising it high elevation amount will be greater at lower altitude
											float lowElevationFactor,	// raising it low elevation amount will be greater at higher altitude
											size_t splatMapMode)
	{
		assert(float32HeigthsBuffer.size() == numVerticesPerSize * numVerticesPerSize * sizeof(float));
		assert(normalsBuffer.size() == numVerticesPerSize * numVerticesPerSize * sizeof(struct TheWorld_Utils::_RGB));
		
		splatmapBuffer.clear();
		size_t splatmapSize = numVerticesPerSize * numVerticesPerSize * sizeof(struct TheWorld_Utils::_RGBA);
		splatmapBuffer.reserve(splatmapSize);
		splatmapBuffer.adjustSize(splatmapSize);

		float diffAltitude = terrainEdit->maxHeight - terrainEdit->minHeight;
		
		for (size_t z = 0; z < numVerticesPerSize; z++)
			for (size_t x = 0; x < numVerticesPerSize; x++)
			{
				float h = float32HeigthsBuffer.at<float>(x, z, numVerticesPerSize);
				struct TheWorld_Utils::_RGB rgbNormal = normalsBuffer.at<TheWorld_Utils::_RGB>(x, z, numVerticesPerSize);
				// normals coords range from 0 to 255 in normals buffer we need they range from 0.0 to 1.0
				Eigen::Vector3d packedNormal((const double)(double(rgbNormal.r) / 255), (const double)(double(rgbNormal.g) / 255), (const double)(double(rgbNormal.b) / 255));
						//float nx = (float)packedNormal.x();	// DEBUG ONLY
						//float ny = (float)packedNormal.y();	// DEBUG ONLY
						//float nz = (float)packedNormal.z();	// DEBUG ONLY
				Eigen::Vector3d normal = unpackNormal(packedNormal);
						//nx = (float)normal.x();				// DEBUG ONLY
						//ny = (float)normal.y();				// DEBUG ONLY
						//nz = (float)normal.z();				// DEBUG ONLY
				//normal = normal.normalized();					// DEBUG ONLY
						//nx = (float)normal.x();				// DEBUG ONLY
						//ny = (float)normal.y();				// DEBUG ONLY
						//nz = (float)normal.z();				// DEBUG ONLY
				Eigen::Vector3d up(0.0f, 1.0f, 0.0f);
				
				// component on up direction of the normal (it is the y component of the normal if up is classic up == 0, 1, 0) - It ranges from 0=vertical terrain to 1=flat terrain
				float upComponent = float(normal.dot(up));
				
				// (1.0f - upComponent) ranges from 0=flat terrain to 1=vertical terrain so slope ranges from 0-slopeFlatFactor=flat terrain to slopeVerticalFactor-slopeFlatFactor=vertical terrain
				float slope = (slopeVerticalFactor * (1.0f - upComponent)) - slopeFlatFactor;

				// amounts range from 0.0f to 1.0f
				float rocksAmount = TheWorld_Utils::Utils::clamp<float>(slope, 0.0f, 1.0f);								// rocksAmount is max when slope is greater than 1 (vertical enough) and is null when slope is less than 0 (flat enough), otherwise it grows with the slope (if slope between 0 and 1)
				float dirtAmount = TheWorld_Utils::Utils::clamp<float>(rocksAmount * dirtOnRocksFactor, 0.0f, 1.0f);	// dirt grows on rocks
				float highElevationAmount = highElevationFactor * ((h - terrainEdit->minHeight) / diffAltitude) - lowElevationFactor;		// ranges from -lowElevationFactor to highElevationFactor-lowElevationFactor
				highElevationAmount = TheWorld_Utils::Utils::clamp<float>(highElevationAmount, 0.0f, 1.0f);				// highElevationAmount is capped from 0 and 1
				float lowElevationAmount = 1.0f - highElevationAmount;													// lowElevationAmount is the complement of highElevationAmount in range 0/1

				//if (rocksAmount == 0.0f)
				//	rocksAmount = 0.0f;

				struct TheWorld_Utils::_RGBA* rgba = splatmapBuffer.at_ptr<TheWorld_Utils::_RGBA>(x, z, numVerticesPerSize);

				float r, g, b, a;
				if (splatMapMode == 1)
				{
					r = lowElevationAmount;
					r = std::lerp(r, 0.0f, highElevationAmount);	// with higher priority: if highElevationAmount=0 r is unchanged, if highElevationAmount==>1 r==>0 (highElevationAmount can lower lowElevationAmount)
					r = std::lerp(r, 0.0f, dirtAmount);				// with higher priority: if dirtAmount=0 r is unchanged, if dirtAmount==>1 r==>0 (dirtAmount can lower lowElevationAmount)
					r = std::lerp(r, 0.0f, rocksAmount);			// with higher priority: if rocksAmount=0 r is unchanged, if rocksAmount==>1 r==>0 (rocksAmount can lower lowElevationAmount)

					g = highElevationAmount;
					g = std::lerp(g, 0.0f, dirtAmount);				// with higher priority: if dirtAmount=0 g is unchanged, if dirtAmount==>1 g==>0 (dirtAmount can lower highElevationAmount)
					g = std::lerp(g, 0.0f, rocksAmount);			// with higher priority: if rocksAmount=0 g is unchanged, if rocksAmount==>1 g==>0 (rocksAmount can lower highElevationAmount)

					b = dirtAmount;
					b = std::lerp(b, 0.0f, rocksAmount);			// with higher priority: if rocksAmount=0 b is unchanged, if rocksAmount==>1 b==>0 (rocksAmount can lower dirtAmount)

					a = rocksAmount;
				}
				else
				{
					a = rocksAmount;
					a = std::lerp(a, 0.0f, dirtAmount);
					a = std::lerp(a, 0.0f, highElevationAmount);
					a = std::lerp(a, 0.0f, lowElevationAmount);

					b = dirtAmount;
					b = std::lerp(b, 0.0f, highElevationAmount);
					b = std::lerp(b, 0.0f, lowElevationAmount);

					g = highElevationAmount;
					g = std::lerp(g, 0.0f, lowElevationAmount);

					r = lowElevationAmount;
				}

				rgba->r = BYTE(r * 255);
				rgba->g = BYTE(g * 255);
				rgba->b = BYTE(b * 255);
				rgba->a = BYTE(a * 255);
			}
	}

	void MeshCacheBuffer::deprecated_generateNormals(size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, BYTE* normalsBuffer, const size_t normalsBufferSize, size_t& usedBufferSize)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer deprecated_generateNormals1 1 ") + __FUNCTION__, "ALL");

		usedBufferSize = 0;

		size_t requiredBufferSize = vectGridHeights.size() == 0 ? sizeof(struct TheWorld_Utils::_RGB) : vectGridHeights.size() * sizeof(struct TheWorld_Utils::_RGB);

		my_assert(vectGridHeights.size() == 0 || vectGridHeights.size() == numVerticesPerSize * numVerticesPerSize);
		my_assert(normalsBufferSize >= requiredBufferSize);

		struct TheWorld_Utils::_RGB* _tempNormalmapBuffer = (struct TheWorld_Utils::_RGB*)normalsBuffer;

		size_t maxIndex = numVerticesPerSize - 1;

		if (vectGridHeights.size() > 0)
		{
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer deprecated_generateNormals1 1.2 ") + __FUNCTION__, "generation");

			size_t idx = 0;
			for (int z = 0; z < numVerticesPerSize; z++)			// m_heightMapImage->get_height()
			{
				for (int x = 0; x < numVerticesPerSize; x++)		// m_heightMapImage->get_width()
				{
					float h = vectGridHeights[idx];

					Eigen::Vector3d v;

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer deprecated_generateNormals1 1.2.1 ") + __FUNCTION__, "calc vector");

						// h = height of the point for which we are computing the normal
						// hr = height of the point on the rigth
						// hl = height of the point on the left
						// hf = height of the forward point (z growing)
						// hb = height of the backward point (z lessening)
						// step = step in WUs between points
						// we compute normal normalizing the vector (h - hr, step, h - hf) or (hl - h, step, hb - h)
						// according to https://hterrain-plugin.readthedocs.io/en/latest/ section "Procedural generation" it should be (h - hr, step, hf - h)
						//Eigen::Vector3d P((float)x, h, (float)z);	// Verify
						if (x < maxIndex && z < maxIndex)
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
							if (x == maxIndex && z == 0)
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
							else if (x == 0 && z == maxIndex)
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
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer deprecated_generateNormals1 1.2.2 ") + __FUNCTION__, "normalize");

						normal = v.normalized();
					}

					{
						//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer deprecated_generateNormals1 1.2.3 ") + __FUNCTION__, "RGB");

						Eigen::Vector3d packNormal(Eigen::Vector3d normal);
						Eigen::Vector3d packedNormal = packNormal(normal);
						struct _RGB rgb;
						rgb.r = (BYTE)(packedNormal.x() * 255);	// normals coord are from 0 to 1 but if expressed as color in a normlamap are from 0 to 255
						rgb.g = (BYTE)(packedNormal.y() * 255);
						rgb.b = (BYTE)(packedNormal.z() * 255);
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

	void MeshCacheBuffer::deprecated_generateNormals(size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, TheWorld_Utils::MemoryBuffer& normalsBuffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer deprecated_generateNormals2 1 ") + __FUNCTION__, "serialize normals");

		my_assert(vectGridHeights.size() == 0 || vectGridHeights.size() == numVerticesPerSize * numVerticesPerSize);

		size_t requiredBufferSize = vectGridHeights.size() == 0 ? sizeof(struct TheWorld_Utils::_RGB) : vectGridHeights.size() * sizeof(struct TheWorld_Utils::_RGB);

		normalsBuffer.reserve(requiredBufferSize);

		size_t usedBufferSize = 0;
		deprecated_generateNormals(numVerticesPerSize, gridStepInWU, vectGridHeights, normalsBuffer.ptr(), normalsBuffer.reserved(), usedBufferSize);

		normalsBuffer.adjustSize(usedBufferSize);
	}

	void MeshCacheBuffer::setBufferFromCacheQuadrantData(size_t numVerticesPerSize, CacheQuadrantData& cacheQuadrantData, std::string& buffer)
	{
		TheWorld_Utils::MemoryBuffer _buffer;
		setBufferFromCacheQuadrantData(numVerticesPerSize, cacheQuadrantData, _buffer);
		buffer.clear();
		buffer.reserve(_buffer.size());
		buffer.assign((char*)_buffer.ptr(), _buffer.size());
	}

	void MeshCacheBuffer::setBufferFromCacheQuadrantData(size_t numVerticesPerSize, CacheQuadrantData& cacheQuadrantData, TheWorld_Utils::MemoryBuffer& buffer)
	{
		TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1 ") + __FUNCTION__, "ALL");

		size_t size = 0;

		size_t numHeights16 = cacheQuadrantData.heights16Buffer != nullptr ? cacheQuadrantData.heights16Buffer->size() / sizeof(uint16_t) : 0;
		my_assert(cacheQuadrantData.heights16Buffer == nullptr || cacheQuadrantData.heights16Buffer->size() == numHeights16 * sizeof(uint16_t));
		size_t numHeights32 = cacheQuadrantData.heights32Buffer != nullptr ? cacheQuadrantData.heights32Buffer->size() / sizeof(float) : 0;
		my_assert(cacheQuadrantData.heights32Buffer == nullptr || cacheQuadrantData.heights32Buffer->size() == numHeights32 * sizeof(float));
		size_t numNormals = cacheQuadrantData.normalsBuffer != nullptr ? cacheQuadrantData.normalsBuffer->size() / sizeof(TheWorld_Utils::_RGB) : 0;
		my_assert(cacheQuadrantData.normalsBuffer == nullptr || cacheQuadrantData.normalsBuffer->size() == numNormals * sizeof(TheWorld_Utils::_RGB));
		size_t numSplatmapVertices = cacheQuadrantData.splatmapBuffer != nullptr ? cacheQuadrantData.splatmapBuffer->size() / sizeof(TheWorld_Utils::_RGBA) : 0;
		my_assert(cacheQuadrantData.splatmapBuffer == nullptr || cacheQuadrantData.splatmapBuffer->size() == numSplatmapVertices * sizeof(TheWorld_Utils::_RGBA));
		size_t numColormapVertices = cacheQuadrantData.colormapBuffer != nullptr ? cacheQuadrantData.colormapBuffer->size() / sizeof(TheWorld_Utils::_RGBA) : 0;
		my_assert(cacheQuadrantData.colormapBuffer == nullptr || cacheQuadrantData.colormapBuffer->size() == numColormapVertices * sizeof(TheWorld_Utils::_RGBA));
		size_t numGlobalmapVertices = cacheQuadrantData.globalmapBuffer != nullptr ? cacheQuadrantData.globalmapBuffer->size() / sizeof(TheWorld_Utils::_RGB) : 0;
		my_assert(cacheQuadrantData.globalmapBuffer == nullptr || cacheQuadrantData.globalmapBuffer->size() == numGlobalmapVertices * sizeof(TheWorld_Utils::_RGB));

		my_assert(numHeights16 == numHeights32);
		my_assert(numHeights16 == 0 || numHeights16 == numVerticesPerSize * numVerticesPerSize);
		my_assert(numNormals == 0 || numNormals == numVerticesPerSize * numVerticesPerSize);
		my_assert(numSplatmapVertices == 0 || numSplatmapVertices == numVerticesPerSize * numVerticesPerSize);
		my_assert(numColormapVertices == 0 || numColormapVertices == numVerticesPerSize * numVerticesPerSize);
		my_assert(numGlobalmapVertices == 0 || numGlobalmapVertices == numVerticesPerSize * numVerticesPerSize);

		static std::recursive_mutex s_mtxEmptyBuffer;
		static TheWorld_Utils::MemoryBuffer s_flatFloat16HeightsBuffer;
		TheWorld_Utils::MemoryBuffer* _flatFloat16HeightsBuffer = cacheQuadrantData.heights16Buffer;
		static TheWorld_Utils::MemoryBuffer s_flatFloat32HeightsBuffer;
		TheWorld_Utils::MemoryBuffer* _flatFloat32HeightsBuffer = cacheQuadrantData.heights32Buffer;

		if (numHeights16 == 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.1 ") + __FUNCTION__, "gen flat heights");

			_flatFloat16HeightsBuffer = &s_flatFloat16HeightsBuffer;
			_flatFloat32HeightsBuffer = &s_flatFloat32HeightsBuffer;

			numHeights32 = numHeights16 = numVerticesPerSize * numVerticesPerSize;
			
			{
				std::lock_guard<std::recursive_mutex> lock(s_mtxEmptyBuffer);
				
				size_t float16HeightsBufferSize = numVerticesPerSize * numVerticesPerSize * sizeof(uint16_t);
				size_t float32HeightsBufferSize = numVerticesPerSize * numVerticesPerSize * sizeof(float);

				if (s_flatFloat16HeightsBuffer.size() == 0 || s_flatFloat32HeightsBuffer.size() == 0)
				{
					s_flatFloat16HeightsBuffer.reserve(float16HeightsBufferSize);
					uint16_t* _float16HeightsBuffer = (uint16_t*)s_flatFloat16HeightsBuffer.ptr();
					s_flatFloat32HeightsBuffer.reserve(float32HeightsBufferSize);
					float* _float32HeightsBuffer = (float*)s_flatFloat32HeightsBuffer.ptr();

					TheWorld_Utils::FLOAT_32 f(0.0f);
					uint16_t emptyHalf = half_from_float(f.u32);
					{		// first impl.
						for (size_t z = 0; z < numVerticesPerSize; z++)
							for (size_t x = 0; x < numVerticesPerSize; x++)
							{
								*_float16HeightsBuffer = emptyHalf;
								_float16HeightsBuffer++;
								*_float32HeightsBuffer = 0.0f;
								_float32HeightsBuffer++;
							}
					}

					//{		// second impl.
					//	BYTE* _float16HeightsBufferBegin = (BYTE*)_float16HeightsBuffer;
					//	BYTE* _float32HeightsBufferBegin = (BYTE*)_float32HeightsBuffer;
					//	size_t numVerticesPerSizeMinusOne = maxIndex;
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

					assert((BYTE*)_float16HeightsBuffer - s_flatFloat16HeightsBuffer.ptr() == float16HeightsBufferSize);
					s_flatFloat16HeightsBuffer.adjustSize(float16HeightsBufferSize);
					assert((BYTE*)_float32HeightsBuffer - s_flatFloat32HeightsBuffer.ptr() == float32HeightsBufferSize);
					s_flatFloat32HeightsBuffer.adjustSize(float32HeightsBufferSize);
				}
				else
				{
					assert(float16HeightsBufferSize == _flatFloat16HeightsBuffer->size());
					assert(float32HeightsBufferSize == _flatFloat32HeightsBuffer->size());
				}
			}
		}

		size_t float16HeightmapSize = numHeights16 * sizeof(uint16_t);
		size_t float32HeightmapSize = numHeights32 * sizeof(float);
		size_t normalmapSize = numNormals > 0 ? numNormals * sizeof(TheWorld_Utils::_RGB) : sizeof(TheWorld_Utils::_RGB);
		size_t splatmapSize = numSplatmapVertices > 0 ? numSplatmapVertices * sizeof(TheWorld_Utils::_RGBA) : sizeof(TheWorld_Utils::_RGBA);
		size_t colormapSize = numColormapVertices > 0 ? numColormapVertices * sizeof(TheWorld_Utils::_RGBA) : sizeof(TheWorld_Utils::_RGBA);
		size_t globalmapSize = numGlobalmapVertices > 0 ? numGlobalmapVertices * sizeof(TheWorld_Utils::_RGB) : sizeof(TheWorld_Utils::_RGB);

		size_t terrainEditValueSize = cacheQuadrantData.terrainEditValues == nullptr ? 0 : cacheQuadrantData.terrainEditValues->size();
		size_t streamBufferSize = 1 /* "0" */
			+ sizeof(size_t) + cacheQuadrantData.meshId.length()
			+ sizeof(size_t) + terrainEditValueSize
			+ sizeof(size_t); /* numheights */
		if (numHeights16 > 0)
		{
			streamBufferSize = streamBufferSize
				+ sizeof(float) /*min_altitude*/ 
				+ sizeof(float) /*max_altitude*/ 
				+ float16HeightmapSize 
				+ float32HeightmapSize 
				+ normalmapSize
				+ splatmapSize
				+ colormapSize
				+ globalmapSize;
		}
		
		buffer.reserve(streamBufferSize);
		BYTE* _streamBuffer = buffer.ptr();

		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.2 ") + __FUNCTION__, "copy meshId/edit values");

			memcpy(_streamBuffer, "0", 1);
			_streamBuffer++;

			TheWorld_Utils::serializeToByteStream<size_t>(cacheQuadrantData.meshId.length(), _streamBuffer, size);
			_streamBuffer += size;

			memcpy(_streamBuffer, cacheQuadrantData.meshId.c_str(), cacheQuadrantData.meshId.length());
			_streamBuffer += cacheQuadrantData.meshId.length();

			TheWorld_Utils::serializeToByteStream<size_t>(terrainEditValueSize, _streamBuffer, size);
			_streamBuffer += size;

			if (terrainEditValueSize > 0)
			{
				memcpy(_streamBuffer, cacheQuadrantData.terrainEditValues->ptr(), terrainEditValueSize);
				_streamBuffer += terrainEditValueSize;
			}
		}

		size = 0;
		TheWorld_Utils::serializeToByteStream<size_t>(numHeights16, _streamBuffer, size);
		_streamBuffer += size;

		if (numHeights16 > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.3 ") + __FUNCTION__, "copy heights");

			TheWorld_Utils::serializeToByteStream<float>(cacheQuadrantData.minHeight, _streamBuffer, size);
			_streamBuffer += size;

			TheWorld_Utils::serializeToByteStream<float>(cacheQuadrantData.maxHeight, _streamBuffer, size);
			_streamBuffer += size;

			memcpy(_streamBuffer, _flatFloat16HeightsBuffer->ptr(), float16HeightmapSize);
			_streamBuffer += float16HeightmapSize;

			memcpy(_streamBuffer, _flatFloat32HeightsBuffer->ptr(), float32HeightmapSize);
			_streamBuffer += float32HeightmapSize;
		}

		if (numNormals > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.4 ") + __FUNCTION__, "copy normals");

			memcpy(_streamBuffer, cacheQuadrantData.normalsBuffer->ptr(), normalmapSize);
			_streamBuffer += normalmapSize;
		}
		else
		{
			struct TheWorld_Utils::_RGB rgb;
			rgb.r = rgb.g = rgb.b = 0;
			memcpy(_streamBuffer, (BYTE*)&rgb, sizeof(struct TheWorld_Utils::_RGB));
			_streamBuffer += sizeof(struct TheWorld_Utils::_RGB);
		}

		if (numSplatmapVertices > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.4 ") + __FUNCTION__, "copy splatmap");

			memcpy(_streamBuffer, cacheQuadrantData.splatmapBuffer->ptr(), splatmapSize);
			_streamBuffer += splatmapSize;
		}
		else
		{
			struct TheWorld_Utils::_RGBA rgba;
			rgba.r = rgba.g = rgba.b = rgba.a = 0;
			memcpy(_streamBuffer, (BYTE*)&rgba, sizeof(struct TheWorld_Utils::_RGBA));
			_streamBuffer += sizeof(struct TheWorld_Utils::_RGBA);
		}

		if (numColormapVertices > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.4 ") + __FUNCTION__, "copy colormap");

			memcpy(_streamBuffer, cacheQuadrantData.colormapBuffer->ptr(), colormapSize);
			_streamBuffer += colormapSize;
		}
		else
		{
			struct TheWorld_Utils::_RGBA rgba;
			rgba.r = rgba.g = rgba.b = rgba.a = 0;
			memcpy(_streamBuffer, (BYTE*)&rgba, sizeof(struct TheWorld_Utils::_RGBA));
			_streamBuffer += sizeof(struct TheWorld_Utils::_RGBA);
		}

		if (numGlobalmapVertices > 0)
		{
			TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromCacheQuadrantData 1.4 ") + __FUNCTION__, "copy globalmap");

			memcpy(_streamBuffer, cacheQuadrantData.globalmapBuffer->ptr(), globalmapSize);
			_streamBuffer += globalmapSize;
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

	void MeshCacheBuffer::setEmptyBuffer(size_t numVerticesPerSize, std::string& meshId, TheWorld_Utils::MemoryBuffer& buffer)
	{
		meshId = generateNewMeshIdForEmptyBuffer();
		
		TheWorld_Utils::MeshCacheBuffer::CacheQuadrantData cacheQuadrantData;
		cacheQuadrantData.meshId = meshId;
		//BYTE shortBuffer[256 + 1];	size_t size = 0;
		//TheWorld_Utils::serializeToByteStream<size_t>(sizeof(size_t), shortBuffer, size);
		TheWorld_Utils::TerrainEdit terrainEdit;
		TheWorld_Utils::MemoryBuffer terrainEditValuesBuffer((BYTE*)&terrainEdit, terrainEdit.size);
		cacheQuadrantData.minHeight = 0.0f;
		cacheQuadrantData.maxHeight = 0.0f;
		cacheQuadrantData.terrainEditValues = &terrainEditValuesBuffer;
		TheWorld_Utils::MemoryBuffer emptyFloat16HeightsBuffer;
		TheWorld_Utils::MemoryBuffer emptyFloat32HeightsBuffer;
		TheWorld_Utils::MemoryBuffer emptyNormalBuffer;
		TheWorld_Utils::MemoryBuffer emptySplatmapBuffer;
		TheWorld_Utils::MemoryBuffer emptyColormapBuffer;
		TheWorld_Utils::MemoryBuffer emptyGlobalmapBuffer;
		cacheQuadrantData.heights16Buffer = &emptyFloat16HeightsBuffer;
		cacheQuadrantData.heights32Buffer = &emptyFloat32HeightsBuffer;
		cacheQuadrantData.normalsBuffer = &emptyNormalBuffer;
		cacheQuadrantData.splatmapBuffer = &emptySplatmapBuffer;
		cacheQuadrantData.colormapBuffer = &emptyColormapBuffer;
		cacheQuadrantData.globalmapBuffer = &emptyGlobalmapBuffer;

		setBufferFromCacheQuadrantData(numVerticesPerSize, cacheQuadrantData, buffer);
	}

	uint16_t MeshCacheBuffer::halfFromFloat(uint32_t f)
	{
		return half_from_float(f);
	}
	uint32_t MeshCacheBuffer::halfToFloat(uint16_t f)
	{
		return half_to_float(f);
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
			+ size_t_size /* numheights */ + float_size /*min_altitude*/ + float_size /*max_altitude*/ + float16HeightmapSize + float32HeightmapSize + normalmapSize;

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
			//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.3 ") + __FUNCTION__, "loop heights");
		
			minAltitude = 0, maxAltitude = 0;
			bool first = true;
			size_t idx = 0;
			if (vectSize > 0 && _tempFloat16HeithmapBuffer != nullptr)
			{
				{
					//TheWorld_Utils::GuardProfiler profiler(std::string("MeshCacheBuffer setBufferFromHeights 1.3.1 ") + __FUNCTION__, "serialize heights");

					for (int z = 0; z < numVerticesPerSize; z++)			// m_heightMapImage->get_height()
					{
						for (int x = 0; x < numVerticesPerSize; x++)		// m_heightMapImage->get_width()
						{
							TheWorld_Utils::FLOAT_32 f;
							{

								// supposing heights are ordered line by line
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

				std::vector<float> heightsEmptyVector;
				std::vector<float>* heightsVectorPointer = &heightsEmptyVector;
				if (generateNormals)
					heightsVectorPointer = &vectGridHeights;

				size_t usedBufferSize = 0;
				this->deprecated_generateNormals(numVerticesPerSize, gridStepInWU, *heightsVectorPointer, (BYTE*)_tempNormalmapBuffer, normalmapSize, usedBufferSize);
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

	bool MeshCacheBuffer::blendQuadrant(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
		CacheQuadrantData& data,
		CacheQuadrantData& northData,
		CacheQuadrantData& southData,
		CacheQuadrantData& westData,
		CacheQuadrantData& eastData,
		CacheQuadrantData& northwestData,
		CacheQuadrantData& northeastData,
		CacheQuadrantData& southwestData,
		CacheQuadrantData& southeastData)
	{
		bool updated = false;

		TerrainEdit* terrainEdit = (TerrainEdit*)data.terrainEditValues->ptr();
		//if (!terrainEdit->northSideZMinus.needBlend && !terrainEdit->southSideZPlus.needBlend && !terrainEdit->eastSideXPlus.needBlend && !terrainEdit->westSideXMinus.needBlend)
		//	return updated;
		
		if (blendQuadrantOnNorthSide(numVerticesPerSize, gridStepInWU, lastPhase, data, northData, southData, westData, eastData, northwestData, northeastData, southwestData, southeastData))
			updated = true;

		if (blendQuadrantOnSouthSide(numVerticesPerSize, gridStepInWU, lastPhase, data, northData, southData, westData, eastData, northwestData, northeastData, southwestData, southeastData))
			updated = true;

		if (blendQuadrantOnWestSide(numVerticesPerSize, gridStepInWU, lastPhase, data, northData, southData, westData, eastData, northwestData, northeastData, southwestData, southeastData))
			updated = true;

		if (blendQuadrantOnEastSide(numVerticesPerSize, gridStepInWU, lastPhase, data, northData, southData, westData, eastData, northwestData, northeastData, southwestData, southeastData))
			updated = true;

		CacheQuadrantData emptyData;

		if (blendQuadrantOnWestSide(numVerticesPerSize, gridStepInWU, lastPhase, northData, 
			emptyData /*N*/, data /*S*/, northwestData /*W*/, northeastData /*E*/, emptyData /*NW*/, emptyData /*NE*/, westData /*SW*/, eastData /*SE*/))
			updated = true;
		if (blendQuadrantOnEastSide(numVerticesPerSize, gridStepInWU, lastPhase, northData,
			emptyData /*N*/, data /*S*/, northwestData /*W*/, northeastData /*E*/, emptyData /*NW*/, emptyData /*NE*/, westData /*SW*/, eastData /*SE*/))
			updated = true;

		if (blendQuadrantOnWestSide(numVerticesPerSize, gridStepInWU, lastPhase, southData,
			data /*N*/, emptyData /*S*/, southwestData /*W*/, southeastData /*E*/, westData /*NW*/, eastData /*NE*/, emptyData /*SW*/, emptyData /*SE*/))
			updated = true;
		if (blendQuadrantOnEastSide(numVerticesPerSize, gridStepInWU, lastPhase, southData,
			data /*N*/, emptyData /*S*/, southwestData /*W*/, southeastData /*E*/, westData /*NW*/, eastData /*NE*/, emptyData /*SW*/, emptyData /*SE*/))
			updated = true;

		if (blendQuadrantOnNorthSide(numVerticesPerSize, gridStepInWU, lastPhase, westData,
			northwestData /*N*/, southwestData /*S*/, emptyData /*W*/, data /*E*/, emptyData /*NW*/, northData /*NE*/, emptyData /*SW*/, southData /*SE*/))
			updated = true;
		if (blendQuadrantOnSouthSide(numVerticesPerSize, gridStepInWU, lastPhase, westData,
			northwestData /*N*/, southwestData /*S*/, emptyData /*W*/, data /*E*/, emptyData /*NW*/, northData /*NE*/, emptyData /*SW*/, southData /*SE*/))
			updated = true;

		if (blendQuadrantOnNorthSide(numVerticesPerSize, gridStepInWU, lastPhase, eastData,
			northeastData /*N*/, southeastData /*S*/, data /*W*/, emptyData /*E*/, northData /*NW*/, emptyData /*NE*/, southData /*SW*/, emptyData /*SE*/))
			updated = true;
		if (blendQuadrantOnSouthSide(numVerticesPerSize, gridStepInWU, lastPhase, eastData,
			northeastData /*N*/, southeastData /*S*/, data /*W*/, emptyData /*E*/, northData /*NW*/, emptyData /*NE*/, southData /*SW*/, emptyData /*SE*/))
			updated = true;

		if (lastPhase)
		{
			if(blendBorders(numVerticesPerSize, gridStepInWU, lastPhase, data, northData, southData, westData, eastData))
				updated = true;
		}

		if (data.heightsUpdated)
		{
			calcMinMaxHeight(numVerticesPerSize, terrainEdit, *data.heights32Buffer);
		}

		if (northData.heightsUpdated && northData.terrainEditValues != nullptr)
		{
			TerrainEdit* northTerrainEdit = (TerrainEdit*)northData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, northTerrainEdit, *northData.heights32Buffer);
		}

		if (southData.heightsUpdated && southData.terrainEditValues != nullptr)
		{
			TerrainEdit* southTerrainEdit = (TerrainEdit*)southData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, southTerrainEdit, *southData.heights32Buffer);
		}

		if (westData.heightsUpdated && westData.terrainEditValues != nullptr)
		{
			TerrainEdit* westTerrainEdit = (TerrainEdit*)westData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, westTerrainEdit, *westData.heights32Buffer);
		}

		if (eastData.heightsUpdated && eastData.terrainEditValues != nullptr)
		{
			TerrainEdit* eastTerrainEdit = (TerrainEdit*)eastData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, eastTerrainEdit, *eastData.heights32Buffer);
		}

		if (northwestData.heightsUpdated && northwestData.terrainEditValues != nullptr)
		{
			TerrainEdit* northwestTerrainEdit = (TerrainEdit*)northwestData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, northwestTerrainEdit, *northwestData.heights32Buffer);
		}

		if (northeastData.heightsUpdated && northeastData.terrainEditValues != nullptr)
		{
			TerrainEdit* northeastTerrainEdit = (TerrainEdit*)northeastData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, northeastTerrainEdit, *northeastData.heights32Buffer);
		}

		if (southwestData.heightsUpdated && southwestData.terrainEditValues != nullptr)
		{
			TerrainEdit* southwesthTerrainEdit = (TerrainEdit*)southwestData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, southwesthTerrainEdit, *southwestData.heights32Buffer);
		}

		if (southeastData.heightsUpdated && southeastData.terrainEditValues != nullptr)
		{
			TerrainEdit* southeastTerrainEdit = (TerrainEdit*)southeastData.terrainEditValues->ptr();
			calcMinMaxHeight(numVerticesPerSize, southeastTerrainEdit, *southeastData.heights32Buffer);
		}

		return updated;
	}

	bool MeshCacheBuffer::blendBorders(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
		CacheQuadrantData& data,
		CacheQuadrantData& northData,
		CacheQuadrantData& southData,
		CacheQuadrantData& westData,
		CacheQuadrantData& eastData)
	{
		bool updated = false;

		if (data.heights32Buffer == nullptr || data.heights16Buffer == nullptr)
			return updated;

		size_t maxIndex = numVerticesPerSize - 1;

		// North side
		if (northData.heights32Buffer != nullptr && northData.heights16Buffer != nullptr)
		{
			for (size_t x = 0; x < numVerticesPerSize; x++)
			{
				float dataUpperBorderHeight = data.heights32Buffer->at<float>(x, 0, numVerticesPerSize);
				float northDataLowerBorderHeight = northData.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize);
				if (dataUpperBorderHeight != northDataLowerBorderHeight)
				{
					updated = true;
					data.heightsUpdated = true;
					northData.heightsUpdated = true;

					if (dataUpperBorderHeight > northDataLowerBorderHeight)
					{
						data.heights32Buffer->at<float>(x, 0, numVerticesPerSize) = northDataLowerBorderHeight;
						data.heights16Buffer->at<uint16_t>(x, 0, numVerticesPerSize) = northData.heights16Buffer->at<uint16_t>(x, maxIndex, numVerticesPerSize);
					}
					else
					{
						northData.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize) = dataUpperBorderHeight;
						northData.heights16Buffer->at<uint16_t>(x, maxIndex, numVerticesPerSize) = data.heights16Buffer->at<uint16_t>(x, 0, numVerticesPerSize);
					}
				}
			}
		}

		// South side
		if (southData.heights32Buffer != nullptr && southData.heights16Buffer != nullptr)
		{
			for (size_t x = 0; x < numVerticesPerSize; x++)
			{
				float dataLowerBorderHeight = data.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize);
				float southDataUpperBorderHeight = southData.heights32Buffer->at<float>(x, 0, numVerticesPerSize);
				if (dataLowerBorderHeight != southDataUpperBorderHeight)
				{
					updated = true;
					data.heightsUpdated = true;
					southData.heightsUpdated = true;

					if (dataLowerBorderHeight > southDataUpperBorderHeight)
					{
						data.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize) = southDataUpperBorderHeight;
						data.heights16Buffer->at<uint16_t>(x, maxIndex, numVerticesPerSize) = southData.heights16Buffer->at<uint16_t>(x, 0, numVerticesPerSize);
					}
					else
					{
						southData.heights32Buffer->at<float>(x, 0, numVerticesPerSize) = dataLowerBorderHeight;
						southData.heights16Buffer->at<uint16_t>(x, 0, numVerticesPerSize) = data.heights16Buffer->at<uint16_t>(x, maxIndex, numVerticesPerSize);
					}
				}
			}
		}

		// West side
		if (westData.heights32Buffer != nullptr && westData.heights16Buffer != nullptr)
		{
			for (size_t z = 0; z < numVerticesPerSize; z++)
			{
				float dataLeftBorderHeight = data.heights32Buffer->at<float>(0, z, numVerticesPerSize);
				float westDataRightBorderHeight = westData.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize);
				if (dataLeftBorderHeight != westDataRightBorderHeight)
				{
					updated = true;
					data.heightsUpdated = true;
					westData.heightsUpdated = true;

					if (dataLeftBorderHeight > westDataRightBorderHeight)
					{
						data.heights32Buffer->at<float>(0, z, numVerticesPerSize) = westDataRightBorderHeight;
						data.heights16Buffer->at<uint16_t>(0, z, numVerticesPerSize) = westData.heights16Buffer->at<uint16_t>(maxIndex, z, numVerticesPerSize);
					}
					else
					{
						westData.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize) = dataLeftBorderHeight;
						westData.heights16Buffer->at<uint16_t>(maxIndex, z, numVerticesPerSize) = data.heights16Buffer->at<uint16_t>(0, z, numVerticesPerSize);
					}
				}
			}
		}

		// East side
		if (eastData.heights32Buffer != nullptr && eastData.heights16Buffer != nullptr)
		{
			for (size_t z = 0; z < numVerticesPerSize; z++)
			{
				float dataRightBorderHeight = data.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize);
				float eastDataLeftBorderHeight = eastData.heights32Buffer->at<float>(0, z, numVerticesPerSize);
				if (dataRightBorderHeight != eastDataLeftBorderHeight)
				{
					updated = true;
					data.heightsUpdated = true;
					eastData.heightsUpdated = true;

					if (dataRightBorderHeight > eastDataLeftBorderHeight)
					{
						data.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize) = eastDataLeftBorderHeight;
						data.heights16Buffer->at<uint16_t>(maxIndex, z, numVerticesPerSize) = eastData.heights16Buffer->at<uint16_t>(0, z, numVerticesPerSize);
					}
					else
					{
						eastData.heights32Buffer->at<float>(0, z, numVerticesPerSize) = dataRightBorderHeight;
						eastData.heights16Buffer->at<uint16_t>(0, z, numVerticesPerSize) = data.heights16Buffer->at<uint16_t>(maxIndex, z, numVerticesPerSize);
					}
				}
			}
		}

		return updated;
	}
		
	void MeshCacheBuffer::calcMinMaxHeight(size_t numVerticesPerSize, TheWorld_Utils::TerrainEdit* terrainEdit, TheWorld_Utils::MemoryBuffer& heights32Buffer)
	{
		terrainEdit->minHeight = FLT_MAX;
		terrainEdit->maxHeight = -FLT_MAX;
		terrainEdit->eastSideXPlus.minHeight = FLT_MAX;
		terrainEdit->eastSideXPlus.maxHeight = -FLT_MAX;
		terrainEdit->westSideXMinus.minHeight = FLT_MAX;
		terrainEdit->westSideXMinus.maxHeight = -FLT_MAX;
		terrainEdit->southSideZPlus.minHeight = FLT_MAX;
		terrainEdit->southSideZPlus.maxHeight = -FLT_MAX;
		terrainEdit->northSideZMinus.minHeight = FLT_MAX;
		terrainEdit->northSideZMinus.maxHeight = -FLT_MAX;

		size_t maxIndex = numVerticesPerSize - 1;

		for (size_t z = 0; z < numVerticesPerSize; z++)
			for (size_t x = 0; x < numVerticesPerSize; x++)
			{
				float h = heights32Buffer.at<float>(x, z, numVerticesPerSize);
				if (h < terrainEdit->minHeight)
					terrainEdit->minHeight = h;
				if (h > terrainEdit->maxHeight)
					terrainEdit->maxHeight = h;

				if (x == 0)
				{
					if (h < terrainEdit->westSideXMinus.minHeight)
						terrainEdit->westSideXMinus.minHeight = h;
					if (h > terrainEdit->westSideXMinus.maxHeight)
						terrainEdit->westSideXMinus.maxHeight = h;
				}

				if (z == 0)
				{
					if (h < terrainEdit->northSideZMinus.minHeight)
						terrainEdit->northSideZMinus.minHeight = h;
					if (h > terrainEdit->northSideZMinus.maxHeight)
						terrainEdit->northSideZMinus.maxHeight = h;
				}

				if (x == maxIndex)
				{
					if (h < terrainEdit->eastSideXPlus.minHeight)
						terrainEdit->eastSideXPlus.minHeight = h;
					if (h > terrainEdit->eastSideXPlus.maxHeight)
						terrainEdit->eastSideXPlus.maxHeight = h;
				}

				if (z == maxIndex)
				{
					if (h < terrainEdit->southSideZPlus.minHeight)
						terrainEdit->southSideZPlus.minHeight = h;
					if (h > terrainEdit->southSideZPlus.maxHeight)
						terrainEdit->southSideZPlus.maxHeight = h;
				}
			}
	}

	bool MeshCacheBuffer::blendQuadrantOnNorthSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
		CacheQuadrantData& data,
		CacheQuadrantData& northData,
		CacheQuadrantData& southData,
		CacheQuadrantData& westData,
		CacheQuadrantData& eastData,
		CacheQuadrantData& northwestData,
		CacheQuadrantData& northeastData,
		CacheQuadrantData& southwestData,
		CacheQuadrantData& southeastData)
	{
		bool updated = false;

		if (data.terrainEditValues == nullptr)
			return updated;

		TerrainEdit* terrainEdit = (TerrainEdit*)data.terrainEditValues->ptr();
		TerrainEdit* northTerrainEdit = nullptr;
		if (northData.terrainEditValues != nullptr)
			northTerrainEdit = (TerrainEdit*)northData.terrainEditValues->ptr();
		//TerrainEdit* southTerrainEdit = nullptr;
		//if (southData.terrainEditValues != nullptr)
		//	southTerrainEdit = (TerrainEdit*)southData.terrainEditValues->ptr();
		TerrainEdit* westTerrainEdit = nullptr;
		if (westData.terrainEditValues != nullptr)
			westTerrainEdit = (TerrainEdit*)westData.terrainEditValues->ptr();
		TerrainEdit* eastTerrainEdit = nullptr;
		if (eastData.terrainEditValues != nullptr)
			eastTerrainEdit = (TerrainEdit*)eastData.terrainEditValues->ptr();
		TerrainEdit* northwestTerrainEdit = nullptr;
		if (northwestData.terrainEditValues != nullptr)
			northwestTerrainEdit = (TerrainEdit*)northwestData.terrainEditValues->ptr();
		TerrainEdit* northeastTerrainEdit = nullptr;
		if (northeastData.terrainEditValues != nullptr)
			northeastTerrainEdit = (TerrainEdit*)northeastData.terrainEditValues->ptr();
		//TerrainEdit* southwestTerrainEdit = nullptr;
		//if (southwestData.terrainEditValues != nullptr)
		//	southwestTerrainEdit = (TerrainEdit*)southwestData.terrainEditValues->ptr();
		//TerrainEdit* southeastTerrainEdit = nullptr;
		//if (southeastData.terrainEditValues != nullptr)
		//	southeastTerrainEdit = (TerrainEdit*)southeastData.terrainEditValues->ptr();

		size_t maxIndex = numVerticesPerSize - 1;

		if (northData.heights32Buffer != nullptr && northTerrainEdit != nullptr && (terrainEdit->northSideZMinus.needBlend || northTerrainEdit->southSideZPlus.needBlend))
		{
			// moving on north side: every x with z = 0 
			for (size_t x = 0; x < numVerticesPerSize; x++)
			{
				float gapForEachQuandrantOnBorder = abs(data.heights32Buffer->at<float>(x, 0, numVerticesPerSize) - northData.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize)) / 2;
				size_t numVerticesToAdjustInEachQuadrant = (size_t)ceilf(gapForEachQuandrantOnBorder / (gridStepInWU / 2));
				if (numVerticesToAdjustInEachQuadrant > numVerticesPerSize / 2)
					numVerticesToAdjustInEachQuadrant = numVerticesPerSize / 2;

				if (numVerticesToAdjustInEachQuadrant > 0)
				{
					float increment = 0.0f;
					float averageStep = (abs(data.heights32Buffer->at<float>(x, numVerticesToAdjustInEachQuadrant - 1, numVerticesPerSize)
								- northData.heights32Buffer->at<float>(x, maxIndex - (numVerticesToAdjustInEachQuadrant - 1), numVerticesPerSize)) / 2) / numVerticesToAdjustInEachQuadrant;

					data.heightsUpdated = true;
					northData.heightsUpdated = true;
					updated = true;

					for (int z = (int)numVerticesToAdjustInEachQuadrant - 1; z >= 0; z--)
					{
						float currentGapInEachQuadrant = abs(data.heights32Buffer->at<float>(x, z, numVerticesPerSize) - northData.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize)) / 2;
						float desideredGapInEachQuadrant = abs(averageStep) * z;
						if (currentGapInEachQuadrant > desideredGapInEachQuadrant)
						{
							increment = currentGapInEachQuadrant - desideredGapInEachQuadrant;
							if (data.heights32Buffer->at<float>(x, z, numVerticesPerSize) > northData.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize))
								increment = -increment;
						}
						else
							increment = 0.0f;

						//float h = data.heights32Buffer->at<float>(x, z, numVerticesPerSize);
						data.heights32Buffer->at<float>(x, z, numVerticesPerSize) += increment;
						TheWorld_Utils::FLOAT_32 f(data.heights32Buffer->at<float>(x, z, numVerticesPerSize));
						uint16_t half = half_from_float(f.u32);
						data.heights16Buffer->at<uint16_t>(x, z, numVerticesPerSize) = half;

						//float hNorth = northData.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize);
						northData.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize) -= increment;
						TheWorld_Utils::FLOAT_32 f1(northData.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize));
						uint16_t half1 = half_from_float(f1.u32);
						northData.heights16Buffer->at<uint16_t>(x, maxIndex - z, numVerticesPerSize) = half1;

						//if (x == 0)
						//	PLOG_DEBUG << "x=" << std::to_string(x) << " z=" << std::to_string(z) << "/" << std::to_string(maxIndex - z) << " increment= " << std::to_string(increment) << " " << std::to_string(h) << " ==> " << std::to_string(data.heights32Buffer->at<float>(x, z, numVerticesPerSize)) << " " << std::to_string(hNorth) << " ==> " << std::to_string(northData.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize));

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (x == 0)
						{
							if (westData.heights32Buffer != nullptr && westTerrainEdit != nullptr && !westTerrainEdit->eastSideXPlus.needBlend && !terrainEdit->westSideXMinus.needBlend)
							{
								if (lastPhase)
								{
									westData.heightsUpdated = true;
									updated = true;
									westData.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize) = f.f32;
									westData.heights16Buffer->at<uint16_t>(maxIndex, z, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->westSideXMinus.needBlend = true;
									westTerrainEdit->eastSideXPlus.needBlend = true;
								}
							}
							if (northwestData.heights32Buffer != nullptr && northwestTerrainEdit  != nullptr && !northwestTerrainEdit->eastSideXPlus.needBlend && northTerrainEdit!= nullptr && !northTerrainEdit->westSideXMinus.needBlend)
							{
								if (lastPhase)
								{
									northwestData.heightsUpdated = true;
									updated = true;
									northwestData.heights32Buffer->at<float>(maxIndex, maxIndex - z, numVerticesPerSize) = f1.f32;
									northwestData.heights16Buffer->at<uint16_t>(maxIndex, maxIndex - z, numVerticesPerSize) = half1;
								}
								else
								{
									northwestTerrainEdit->eastSideXPlus.needBlend = true;
									northTerrainEdit->westSideXMinus.needBlend = true;
								}
							}
						}

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (x == maxIndex)
						{
							if (eastData.heights32Buffer != nullptr && eastTerrainEdit != nullptr && !eastTerrainEdit->westSideXMinus.needBlend && !terrainEdit->eastSideXPlus.needBlend)
							{
								if (lastPhase)
								{
									eastData.heightsUpdated = true;
									updated = true;
									eastData.heights32Buffer->at<float>(0, z, numVerticesPerSize) = f.f32;
									eastData.heights16Buffer->at<uint16_t>(0, z, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->eastSideXPlus.needBlend = true;
									eastTerrainEdit->westSideXMinus.needBlend = true;
								}
							}
							if (northeastData.heights32Buffer != nullptr && northeastTerrainEdit != nullptr && !northeastTerrainEdit->westSideXMinus.needBlend && northTerrainEdit != nullptr && !northTerrainEdit->eastSideXPlus.needBlend)
							{
								if (lastPhase)
								{
									northeastData.heightsUpdated = true;
									updated = true;
									northeastData.heights32Buffer->at<float>(0, maxIndex - z, numVerticesPerSize) = f1.f32;
									northeastData.heights16Buffer->at<uint16_t>(0, maxIndex - z, numVerticesPerSize) = half1;
								}
								else
								{
									northeastTerrainEdit->westSideXMinus.needBlend = true;
									northTerrainEdit->eastSideXPlus.needBlend = true;
								}
							}
						}
					}
				}
			}

			terrainEdit->northSideZMinus.needBlend = false;
			northTerrainEdit->southSideZPlus.needBlend = false;
		}

		return updated;
	}

	bool MeshCacheBuffer::blendQuadrantOnSouthSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
		CacheQuadrantData& data,
		CacheQuadrantData& northData,
		CacheQuadrantData& southData,
		CacheQuadrantData& westData,
		CacheQuadrantData& eastData,
		CacheQuadrantData& northwestData,
		CacheQuadrantData& northeastData,
		CacheQuadrantData& southwestData,
		CacheQuadrantData& southeastData)
	{
		bool updated = false;

		if (data.terrainEditValues == nullptr)
			return updated;

		TerrainEdit* terrainEdit = (TerrainEdit*)data.terrainEditValues->ptr();
		//TerrainEdit* northTerrainEdit = nullptr;
		//if (northData.terrainEditValues != nullptr)
		//	northTerrainEdit = (TerrainEdit*)northData.terrainEditValues->ptr();
		TerrainEdit* southTerrainEdit = nullptr;
		if (southData.terrainEditValues != nullptr)
			southTerrainEdit = (TerrainEdit*)southData.terrainEditValues->ptr();
		TerrainEdit* westTerrainEdit = nullptr;
		if (westData.terrainEditValues != nullptr)
			westTerrainEdit = (TerrainEdit*)westData.terrainEditValues->ptr();
		TerrainEdit* eastTerrainEdit = nullptr;
		if (eastData.terrainEditValues != nullptr)
			eastTerrainEdit = (TerrainEdit*)eastData.terrainEditValues->ptr();
		//TerrainEdit* northwestTerrainEdit = nullptr;
		//if (northwestData.terrainEditValues != nullptr)
		//	northwestTerrainEdit = (TerrainEdit*)northwestData.terrainEditValues->ptr();
		//TerrainEdit* northeastTerrainEdit = nullptr;
		//if (northeastData.terrainEditValues != nullptr)
		//	northeastTerrainEdit = (TerrainEdit*)northeastData.terrainEditValues->ptr();
		TerrainEdit* southwestTerrainEdit = nullptr;
		if (southwestData.terrainEditValues != nullptr)
			southwestTerrainEdit = (TerrainEdit*)southwestData.terrainEditValues->ptr();
		TerrainEdit* southeastTerrainEdit = nullptr;
		if (southeastData.terrainEditValues != nullptr)
			southeastTerrainEdit = (TerrainEdit*)southeastData.terrainEditValues->ptr();

		size_t maxIndex = numVerticesPerSize - 1;

		if (southData.heights32Buffer != nullptr && southTerrainEdit != nullptr && (terrainEdit->southSideZPlus.needBlend || southTerrainEdit->northSideZMinus.needBlend))
		{
			// moving on south side: every x with z = 0 
			for (size_t x = 0; x < numVerticesPerSize; x++)
			{
				float gapForEachQuandrantOnBorder = abs(data.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize) - southData.heights32Buffer->at<float>(x, 0, numVerticesPerSize)) / 2;
				size_t numVerticesToAdjustInEachQuadrant = (size_t)ceilf(gapForEachQuandrantOnBorder / (gridStepInWU / 2));
				if (numVerticesToAdjustInEachQuadrant > numVerticesPerSize / 2)
					numVerticesToAdjustInEachQuadrant = numVerticesPerSize / 2;

				if (numVerticesToAdjustInEachQuadrant > 0)
				{
					float increment = 0.0f;
					float averageStep = (abs(data.heights32Buffer->at<float>(x, maxIndex - (numVerticesToAdjustInEachQuadrant - 1), numVerticesPerSize)
						- southData.heights32Buffer->at<float>(x, numVerticesToAdjustInEachQuadrant - 1, numVerticesPerSize)) / 2) / numVerticesToAdjustInEachQuadrant;

					data.heightsUpdated = true;
					southData.heightsUpdated = true;
					updated = true;

					for (int z = (int)numVerticesToAdjustInEachQuadrant - 1; z >= 0; z--)
					{
						float currentGapInEachQuadrant = abs(data.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize) - southData.heights32Buffer->at<float>(x, z, numVerticesPerSize)) / 2;
						float desideredGapInEachQuadrant = abs(averageStep) * z;
						if (currentGapInEachQuadrant > desideredGapInEachQuadrant)
						{
							increment = currentGapInEachQuadrant - desideredGapInEachQuadrant;
							if (data.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize) > southData.heights32Buffer->at<float>(x, z, numVerticesPerSize))
								increment = -increment;
						}
						else
							increment = 0.0f;

						data.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize) += increment;
						TheWorld_Utils::FLOAT_32 f(data.heights32Buffer->at<float>(x, maxIndex - z, numVerticesPerSize));
						uint16_t half = half_from_float(f.u32);
						data.heights16Buffer->at<uint16_t>(x, maxIndex - z, numVerticesPerSize) = half;

						southData.heights32Buffer->at<float>(x, z, numVerticesPerSize) -= increment;
						TheWorld_Utils::FLOAT_32 f1(southData.heights32Buffer->at<float>(x, z, numVerticesPerSize));
						uint16_t half1 = half_from_float(f1.u32);
						southData.heights16Buffer->at<uint16_t>(x, z, numVerticesPerSize) = half1;

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (x == 0)
						{
							if (westData.heights32Buffer != nullptr && westTerrainEdit != nullptr && !westTerrainEdit->eastSideXPlus.needBlend && !terrainEdit->westSideXMinus.needBlend)
							{
								if (lastPhase)
								{
									westData.heightsUpdated = true;
									updated = true;
									westData.heights32Buffer->at<float>(maxIndex, maxIndex - z, numVerticesPerSize) = f.f32;
									westData.heights16Buffer->at<uint16_t>(maxIndex, maxIndex - z, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->westSideXMinus.needBlend = true;
									westTerrainEdit->eastSideXPlus.needBlend = true;
								}
							}
							if (southwestData.heights32Buffer != nullptr && southwestTerrainEdit != nullptr && !southwestTerrainEdit->eastSideXPlus.needBlend && southTerrainEdit != nullptr && !southTerrainEdit->westSideXMinus.needBlend)
							{
								if (lastPhase)
								{
									southwestData.heightsUpdated = true;
									updated = true;
									southwestData.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize) = f1.f32;
									southwestData.heights16Buffer->at<uint16_t>(maxIndex, z, numVerticesPerSize) = half1;
								}
								else
								{
									southwestTerrainEdit->eastSideXPlus.needBlend = true;
									southTerrainEdit->westSideXMinus.needBlend = true;
								}
							}
						}

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (x == maxIndex)
						{
							if (eastData.heights32Buffer != nullptr && eastTerrainEdit != nullptr && !eastTerrainEdit->westSideXMinus.needBlend && !terrainEdit->eastSideXPlus.needBlend)
							{
								if (lastPhase)
								{
									eastData.heightsUpdated = true;
									updated = true;
									eastData.heights32Buffer->at<float>(0, maxIndex - z, numVerticesPerSize) = f.f32;
									eastData.heights16Buffer->at<uint16_t>(0, maxIndex - z, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->eastSideXPlus.needBlend = true;
									eastTerrainEdit->westSideXMinus.needBlend = true;
								}
							}
							if (southeastData.heights32Buffer != nullptr && southeastTerrainEdit != nullptr && !southeastTerrainEdit->westSideXMinus.needBlend && southTerrainEdit != nullptr && !southTerrainEdit->eastSideXPlus.needBlend)
							{
								if (lastPhase)
								{
									southeastData.heightsUpdated = true;
									updated = true;
									southeastData.heights32Buffer->at<float>(0, z, numVerticesPerSize) = f1.f32;
									southeastData.heights16Buffer->at<uint16_t>(0, z, numVerticesPerSize) = half1;
								}
								else
								{
									southeastTerrainEdit->westSideXMinus.needBlend = true;
									southTerrainEdit->eastSideXPlus.needBlend = true;
								}
							}
						}
					}
				}
			}

			terrainEdit->southSideZPlus.needBlend = false;
			southTerrainEdit->northSideZMinus.needBlend = false;
		}

		return updated;
	}

	bool MeshCacheBuffer::blendQuadrantOnWestSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
		CacheQuadrantData& data,
		CacheQuadrantData& northData,
		CacheQuadrantData& southData,
		CacheQuadrantData& westData,
		CacheQuadrantData& eastData,
		CacheQuadrantData& northwestData,
		CacheQuadrantData& northeastData,
		CacheQuadrantData& southwestData,
		CacheQuadrantData& southeastData)
	{
		bool updated = false;

		if (data.terrainEditValues == nullptr)
			return updated;

		TerrainEdit* terrainEdit = (TerrainEdit*)data.terrainEditValues->ptr();
		TerrainEdit* northTerrainEdit = nullptr;
		if (northData.terrainEditValues != nullptr)
			northTerrainEdit = (TerrainEdit*)northData.terrainEditValues->ptr();
		TerrainEdit* southTerrainEdit = nullptr;
		if (southData.terrainEditValues != nullptr)
			southTerrainEdit = (TerrainEdit*)southData.terrainEditValues->ptr();
		TerrainEdit* westTerrainEdit = nullptr;
		if (westData.terrainEditValues != nullptr)
			westTerrainEdit = (TerrainEdit*)westData.terrainEditValues->ptr();
		//TerrainEdit* eastTerrainEdit = nullptr;
		//if (eastData.terrainEditValues != nullptr)
		//	eastTerrainEdit = (TerrainEdit*)eastData.terrainEditValues->ptr();
		TerrainEdit* northwestTerrainEdit = nullptr;
		if (northwestData.terrainEditValues != nullptr)
			northwestTerrainEdit = (TerrainEdit*)northwestData.terrainEditValues->ptr();
		//TerrainEdit* northeastTerrainEdit = nullptr;
		//if (northeastData.terrainEditValues != nullptr)
		//	northeastTerrainEdit = (TerrainEdit*)northeastData.terrainEditValues->ptr();
		TerrainEdit* southwestTerrainEdit = nullptr;
		if (southwestData.terrainEditValues != nullptr)
			southwestTerrainEdit = (TerrainEdit*)southwestData.terrainEditValues->ptr();
		//TerrainEdit* southeastTerrainEdit = nullptr;
		//if (southeastData.terrainEditValues != nullptr)
		//	southeastTerrainEdit = (TerrainEdit*)southeastData.terrainEditValues->ptr();

		size_t maxIndex = numVerticesPerSize - 1;

		if (westData.heights32Buffer != nullptr && westTerrainEdit != nullptr && (terrainEdit->westSideXMinus.needBlend || westTerrainEdit->eastSideXPlus.needBlend))
		{
			// moving on west side: every z with x = 0 
			for (size_t z = 0; z < numVerticesPerSize; z++)
			{
				float gapForEachQuandrantOnBorder = abs(data.heights32Buffer->at<float>(0, z, numVerticesPerSize) - westData.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize)) / 2;
				size_t numVerticesToAdjustInEachQuadrant = (size_t)ceilf(gapForEachQuandrantOnBorder / (gridStepInWU / 2));
				if (numVerticesToAdjustInEachQuadrant > numVerticesPerSize / 2)
					numVerticesToAdjustInEachQuadrant = numVerticesPerSize / 2;

				if (numVerticesToAdjustInEachQuadrant > 0)
				{
					float increment = 0.0f;
					float averageStep = (abs(data.heights32Buffer->at<float>(numVerticesToAdjustInEachQuadrant - 1, z, numVerticesPerSize)
						- westData.heights32Buffer->at<float>(maxIndex - (numVerticesToAdjustInEachQuadrant - 1), z, numVerticesPerSize)) / 2) / numVerticesToAdjustInEachQuadrant;

					data.heightsUpdated = true;
					westData.heightsUpdated = true;
					updated = true;

					for (int x = (int)numVerticesToAdjustInEachQuadrant - 1; x >= 0; x--)
					{
						float currentGapInEachQuadrant = abs(data.heights32Buffer->at<float>(x, z, numVerticesPerSize) - westData.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize)) / 2;
						float desideredGapInEachQuadrant = abs(averageStep) * x;
						if (currentGapInEachQuadrant > desideredGapInEachQuadrant)
						{
							increment = currentGapInEachQuadrant - desideredGapInEachQuadrant;
							if (data.heights32Buffer->at<float>(x, z, numVerticesPerSize) > westData.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize))
								increment = -increment;
						}
						else
							increment = 0.0f;

						data.heights32Buffer->at<float>(x, z, numVerticesPerSize) += increment;
						TheWorld_Utils::FLOAT_32 f(data.heights32Buffer->at<float>(x, z, numVerticesPerSize));
						uint16_t half = half_from_float(f.u32);
						data.heights16Buffer->at<uint16_t>(x, z, numVerticesPerSize) = half;

						westData.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize) -= increment;
						TheWorld_Utils::FLOAT_32 f1(westData.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize));
						uint16_t half1 = half_from_float(f1.u32);
						westData.heights16Buffer->at<uint16_t>(maxIndex - x, z, numVerticesPerSize) = half1;

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (z == 0)
						{
							if (northData.heights32Buffer != nullptr && northTerrainEdit != nullptr && !northTerrainEdit->southSideZPlus.needBlend && !terrainEdit->northSideZMinus.needBlend)
							{
								if (lastPhase)
								{
									northData.heightsUpdated = true;
									updated = true;
									northData.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize) = f.f32;
									northData.heights16Buffer->at<uint16_t>(x, maxIndex, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->northSideZMinus.needBlend = true;
									northTerrainEdit->southSideZPlus.needBlend = true;
								}
							}
							if (northwestData.heights32Buffer != nullptr && northwestTerrainEdit != nullptr && !northwestTerrainEdit->southSideZPlus.needBlend && westTerrainEdit != nullptr && !westTerrainEdit->northSideZMinus.needBlend)
							{
								if (lastPhase)
								{
									northwestData.heightsUpdated = true;
									updated = true;
									northwestData.heights32Buffer->at<float>(maxIndex - x, maxIndex, numVerticesPerSize) = f1.f32;
									northwestData.heights16Buffer->at<uint16_t>(maxIndex - x, maxIndex, numVerticesPerSize) = half1;
								}
								else
								{
									northwestTerrainEdit->southSideZPlus.needBlend = true;
									westTerrainEdit->northSideZMinus.needBlend = true;
								}
							}
						}

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (z == maxIndex)
						{
							if (southData.heights32Buffer != nullptr && southTerrainEdit != nullptr && !southTerrainEdit->northSideZMinus.needBlend && !terrainEdit->southSideZPlus.needBlend)
							{
								if (lastPhase)
								{
									southData.heightsUpdated = true;
									updated = true;
									southData.heights32Buffer->at<float>(x, 0, numVerticesPerSize) = f.f32;
									southData.heights16Buffer->at<uint16_t>(x, 0, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->southSideZPlus.needBlend = true;
									southTerrainEdit->northSideZMinus.needBlend = true;
								}
							}
							if (southwestData.heights32Buffer != nullptr && southwestTerrainEdit != nullptr && !southwestTerrainEdit->northSideZMinus.needBlend && westTerrainEdit != nullptr && !westTerrainEdit->southSideZPlus.needBlend)
							{
								if (lastPhase)
								{
									southwestData.heightsUpdated = true;
									updated = true;
									southwestData.heights32Buffer->at<float>(maxIndex - x, 0, numVerticesPerSize) = f1.f32;
									southwestData.heights16Buffer->at<uint16_t>(maxIndex - x, 0, numVerticesPerSize) = half1;
								}
								else
								{
									southwestTerrainEdit->northSideZMinus.needBlend = true;
									westTerrainEdit->southSideZPlus.needBlend = true;
								}
							}
						}
					}
				}
			}

			terrainEdit->westSideXMinus.needBlend = false;
			westTerrainEdit->eastSideXPlus.needBlend = false;
		}

		return updated;
	}

	bool MeshCacheBuffer::blendQuadrantOnEastSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
		CacheQuadrantData& data,
		CacheQuadrantData& northData,
		CacheQuadrantData& southData,
		CacheQuadrantData& westData,
		CacheQuadrantData& eastData,
		CacheQuadrantData& northwestData,
		CacheQuadrantData& northeastData,
		CacheQuadrantData& southwestData,
		CacheQuadrantData& southeastData)
	{
		bool updated = false;

		if (data.terrainEditValues == nullptr)
			return updated;

		TerrainEdit* terrainEdit = (TerrainEdit*)data.terrainEditValues->ptr();
		TerrainEdit* northTerrainEdit = nullptr;
		if (northData.terrainEditValues != nullptr)
			northTerrainEdit = (TerrainEdit*)northData.terrainEditValues->ptr();
		TerrainEdit* southTerrainEdit = nullptr;
		if (southData.terrainEditValues != nullptr)
			southTerrainEdit = (TerrainEdit*)southData.terrainEditValues->ptr();
		//TerrainEdit* westTerrainEdit = nullptr;
		//if (westData.terrainEditValues != nullptr)
		//	westTerrainEdit = (TerrainEdit*)westData.terrainEditValues->ptr();
		TerrainEdit* eastTerrainEdit = nullptr;
		if (eastData.terrainEditValues != nullptr)
			eastTerrainEdit = (TerrainEdit*)eastData.terrainEditValues->ptr();
		//TerrainEdit* northwestTerrainEdit = nullptr;
		//if (northwestData.terrainEditValues != nullptr)
		//	northwestTerrainEdit = (TerrainEdit*)northwestData.terrainEditValues->ptr();
		TerrainEdit* northeastTerrainEdit = nullptr;
		if (northeastData.terrainEditValues != nullptr)
			northeastTerrainEdit = (TerrainEdit*)northeastData.terrainEditValues->ptr();
		//TerrainEdit* southwestTerrainEdit = nullptr;
		//if (southwestData.terrainEditValues != nullptr)
		//	southwestTerrainEdit = (TerrainEdit*)southwestData.terrainEditValues->ptr();
		TerrainEdit* southeastTerrainEdit = nullptr;
		if (southeastData.terrainEditValues != nullptr)
			southeastTerrainEdit = (TerrainEdit*)southeastData.terrainEditValues->ptr();

		size_t maxIndex = numVerticesPerSize - 1;

		if (eastData.heights32Buffer != nullptr && eastTerrainEdit != nullptr && (terrainEdit->eastSideXPlus.needBlend || eastTerrainEdit->westSideXMinus.needBlend))
		{
			// moving on south side: every x with z = 0 
			for (size_t z = 0; z < numVerticesPerSize; z++)
			{
				float gapForEachQuandrantOnBorder = abs(data.heights32Buffer->at<float>(maxIndex, z, numVerticesPerSize) - eastData.heights32Buffer->at<float>(0, z, numVerticesPerSize)) / 2;
				size_t numVerticesToAdjustInEachQuadrant = (size_t)ceilf(gapForEachQuandrantOnBorder / (gridStepInWU / 2));
				if (numVerticesToAdjustInEachQuadrant > numVerticesPerSize / 2)
					numVerticesToAdjustInEachQuadrant = numVerticesPerSize / 2;

				if (numVerticesToAdjustInEachQuadrant > 0)
				{
					float increment = 0.0f;
					float averageStep = (abs(data.heights32Buffer->at<float>(maxIndex - (numVerticesToAdjustInEachQuadrant - 1), z, numVerticesPerSize)
						- eastData.heights32Buffer->at<float>(numVerticesToAdjustInEachQuadrant - 1, z, numVerticesPerSize)) / 2) / numVerticesToAdjustInEachQuadrant;

					data.heightsUpdated = true;
					eastData.heightsUpdated = true;
					updated = true;

					for (int x = (int)numVerticesToAdjustInEachQuadrant - 1; x >= 0; x--)
					{
						float currentGapInEachQuadrant = abs(data.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize) - eastData.heights32Buffer->at<float>(x, z, numVerticesPerSize)) / 2;
						float desideredGapInEachQuadrant = abs(averageStep) * x;
						if (currentGapInEachQuadrant > desideredGapInEachQuadrant)
						{
							increment = currentGapInEachQuadrant - desideredGapInEachQuadrant;
							if (data.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize) > eastData.heights32Buffer->at<float>(x, z, numVerticesPerSize))
								increment = -increment;
						}
						else
							increment = 0.0f;

						data.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize) += increment;
						TheWorld_Utils::FLOAT_32 f(data.heights32Buffer->at<float>(maxIndex - x, z, numVerticesPerSize));
						uint16_t half = half_from_float(f.u32);
						data.heights16Buffer->at<uint16_t>(maxIndex - x, z, numVerticesPerSize) = half;

						eastData.heights32Buffer->at<float>(x, z, numVerticesPerSize) -= increment;
						TheWorld_Utils::FLOAT_32 f1(eastData.heights32Buffer->at<float>(x, z, numVerticesPerSize));
						uint16_t half1 = half_from_float(f1.u32);
						eastData.heights16Buffer->at<uint16_t>(x, z, numVerticesPerSize) = half1;

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (z == 0)
						{
							if (northData.heights32Buffer != nullptr && northTerrainEdit != nullptr && !northTerrainEdit->southSideZPlus.needBlend && !terrainEdit->northSideZMinus.needBlend)
							{
								if (lastPhase)
								{
									northData.heightsUpdated = true;
									updated = true;
									northData.heights32Buffer->at<float>(maxIndex - x, maxIndex, numVerticesPerSize) = f.f32;
									northData.heights16Buffer->at<uint16_t>(maxIndex - x, maxIndex, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->northSideZMinus.needBlend = true;
									northTerrainEdit->southSideZPlus.needBlend = true;
								}
							}
							if (northeastData.heights32Buffer != nullptr && northeastTerrainEdit != nullptr && !northeastTerrainEdit->southSideZPlus.needBlend && eastTerrainEdit != nullptr && !eastTerrainEdit->northSideZMinus.needBlend)
							{
								if (lastPhase)
								{
									northeastData.heightsUpdated = true;
									updated = true;
									northeastData.heights32Buffer->at<float>(x, maxIndex, numVerticesPerSize) = f1.f32;
									northeastData.heights16Buffer->at<uint16_t>(x, maxIndex, numVerticesPerSize) = half1;
								}
								else
								{
									northeastTerrainEdit->southSideZPlus.needBlend = true;
									eastTerrainEdit->northSideZMinus.needBlend = true;
								}
							}
						}

						// if last phase we need to reconcile vertices on the border of modified quadrants
						if (z == maxIndex)
						{
							if (southData.heights32Buffer != nullptr && southTerrainEdit != nullptr && !southTerrainEdit->northSideZMinus.needBlend && !terrainEdit->southSideZPlus.needBlend)
							{
								if (lastPhase)
								{
									southData.heightsUpdated = true;
									updated = true;
									southData.heights32Buffer->at<float>(maxIndex - x, 0, numVerticesPerSize) = f.f32;
									southData.heights16Buffer->at<uint16_t>(maxIndex - x, 0, numVerticesPerSize) = half;
								}
								else
								{
									terrainEdit->southSideZPlus.needBlend = true;
									southTerrainEdit->northSideZMinus.needBlend = true;
								}
							}
							if (southeastData.heights32Buffer != nullptr && southeastTerrainEdit != nullptr && !southeastTerrainEdit->northSideZMinus.needBlend && eastTerrainEdit != nullptr && !eastTerrainEdit->southSideZPlus.needBlend)
							{
								if (lastPhase)
								{
									southeastData.heightsUpdated = true;
									updated = true;
									southeastData.heights32Buffer->at<float>(x, 0, numVerticesPerSize) = f1.f32;
									southeastData.heights16Buffer->at<uint16_t>(x, 0, numVerticesPerSize) = half1;
								}
								else
								{
									southeastTerrainEdit->northSideZMinus.needBlend = true;
									eastTerrainEdit->southSideZPlus.needBlend = true;
								}
							}
						}
					}
				}
			}

			terrainEdit->eastSideXPlus.needBlend = false;
			eastTerrainEdit->westSideXMinus.needBlend = false;
		}

		return updated;
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

	size_t ThreadPool::getNumWorkingThreads(size_t& maxThreads)
	{
		maxThreads = m_threads.size();
		return m_numWorkingThreads;
	}
		
	bool ThreadPool::allThreadsWorking(void)
	{
		return m_numWorkingThreads >= m_threads.size();
	}

	void ThreadPool::ThreadLoop()
	{
		if (m_threadInitDeinit != nullptr)
			m_threadInitDeinit->threadInit();
		//if (m_threadInitFunction != nullptr)
		//{
		//	(*m_threadInitFunction)();
		//}

		DWORD tid = GetCurrentThreadId();
		std::thread::id this_id = std::this_thread::get_id();
		size_t _tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

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

				{
					std::unique_lock<std::mutex> lock(m_threadmap_mutex);
					m_numWorkingThreads++;
					m_workingThreads[tid] = m_numWorkingThreads;
				}

				TheWorld_Utils::MsTimePoint now = std::chrono::time_point_cast<TheWorld_Utils::MsTimePoint::duration>(std::chrono::system_clock::now());
				size_t numThreads = m_threads.size();
				bool allThreadsWorking = m_numWorkingThreads >= numThreads;
				if ((now - m_lastDiagnosticTime).count() >= 1000)
				{
					if (allThreadsWorking)
						PLOG_INFO << "ThreadPool " << m_label << " has all threads working (" << std::to_string(m_numWorkingThreads) << ":" << std::to_string(numThreads) << ") - Queue size: " << m_jobs.size();
					//else
					//	if (allThreadsWorking != m_lastAllWorkingStatus)
					//		PLOG_INFO << "ThreadPool " << m_label << " has free threads (" << std::to_string(m_numWorkingThreads + 1) << ":" << std::to_string(numThreads) << ")";

					m_lastDiagnosticTime = now;
					m_lastAllWorkingStatus = allThreadsWorking;
				}
			}
			job();
			{
				std::unique_lock<std::mutex> lock(m_threadmap_mutex);
				if (m_workingThreads.contains(tid))
					m_workingThreads.erase(tid);
				m_numWorkingThreads--;
			}
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

	// normals coord range from 0 to 1
	Eigen::Vector3d packNormal(Eigen::Vector3d normal)
	{
		Eigen::Vector3d temp = 0.5 * (normal + Eigen::Vector3d(1, 1, 1));
		return Eigen::Vector3d(temp.x(), temp.z(), temp.y());
	}

	// normals coord range from 0 to 1
	Eigen::Vector3d unpackNormal(Eigen::Vector3d packedNormal)
	{
		Eigen::Vector3d temp(packedNormal.x(), packedNormal.z(), packedNormal.y());
		return (2 * temp) - Eigen::Vector3d(1, 1, 1);
		//Eigen::Vector3d temp = (2 * packedNormal) - Eigen::Vector3d(1, 1, 1);
		//return Eigen::Vector3d(temp.x(), temp.z(), temp.y());
	}
	void packNormal(float normalX, float normalY, float normalZ, float& packedNormalX, float& packedNormalY, float& packedNormalZ)
	{
		Eigen::Vector3d packedNormal = TheWorld_Utils::packNormal(Eigen::Vector3d(normalX, normalY, normalZ));
		packedNormalX = float(packedNormal.x());
		packedNormalY = float(packedNormal.y());
		packedNormalZ = float(packedNormal.z());
	}

	void unpackNormal(float packedNormalX, float packedNormalY, float packedNormalZ, float& normalX, float& normalY, float& normalZ)
	{
		Eigen::Vector3d normal = TheWorld_Utils::unpackNormal(Eigen::Vector3d(packedNormalX, packedNormalY, packedNormalZ));
		normalX = float(normal.x());
		normalY = float(normal.y());
		normalZ = float(normal.z());
	}
}
