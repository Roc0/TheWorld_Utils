#pragma once

//#ifdef _THEWORLD_CLIENT
//	#include <Godot.hpp>
//	#include <ImageTexture.hpp>
//	#include <PoolArrays.hpp>
//#endif

#include "WorldModifier.h"

#ifndef MYAPI 
#define MYAPI   __declspec( dllimport )
#endif 

#include "framework.h"
#include <cfloat>
#include <assert.h> 
#include <guiddef.h>
#include <string> 
#include <chrono>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <exception>
#include <plog/Log.h>
#include "gsl\assert"

#pragma warning(push, 0)
#include <Eigen/Dense>
#pragma warning(pop)

#define Vector3Zero godot::Vector3(0, 0, 0)
#define Vector3UP godot::Vector3(0, 1, 0)

#define Vector3X godot::Vector3(1, 0, 0)
#define Vector3Y godot::Vector3(0, 1, 0)
#define Vector3Z godot::Vector3(0, 0, 1)

namespace TheWorld_Utils
{
	// https://cellperformance.beyond3d.com/articles/2006/07/branchfree_implementation_of_h_1.html

	__declspec(dllexport) Eigen::Vector3d packNormal(Eigen::Vector3d normal);
	__declspec(dllexport) Eigen::Vector3d unpackNormal(Eigen::Vector3d packedNormal);
	__declspec(dllexport) void packNormal(float normalX, float normalY, float normalZ, float& packedNormalX, float& packedNormalY, float& packedNormalZ);
	__declspec(dllexport) void unpackNormal(float packedNormalX, float packedNormalY, float packedNormalZ, float& normalX, float& normalY, float& normalZ);

	__declspec(dllexport) void to_lower_implace(std::string& s);
	__declspec(dllexport) std::string to_lower(std::string s);

	//std::vector<std::string> split(const std::string& s, char seperator)
	//{
	//	std::vector<std::string> output;

	//	std::string::size_type prev_pos = 0, pos = 0;

	//	while ((pos = s.find(seperator, pos)) != std::string::npos)
	//	{
	//		std::string substring(s.substr(prev_pos, pos - prev_pos));

	//		output.push_back(substring);

	//		prev_pos = ++pos;
	//	}

	//	output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

	//	return output;
	//}

	class Exception : public std::exception
	{
	public:
		_declspec(dllexport) Exception(const char* function, const char* message = NULL, const char* message2 = NULL, int rc = 0)
		{
			m_exceptionName = "MapManagerException";
			if (message == NULL || strlen(message) == 0)
				m_message = "MapManager Generic Exception - C++ Exception";
			else
			{
				if (message2 == NULL || strlen(message2) == 0)
				{
					m_message = message;
				}
				else
				{
					m_message = message;
					m_message += " - ";
					m_message += message2;
					m_message += " - rc=";
					m_message += std::to_string(rc);
				}
			}
		};
		_declspec(dllexport) ~Exception() {};

		const char* what() const throw ()
		{
			return m_message.c_str();
		}

		const char* exceptionName()
		{
			return m_exceptionName.c_str();
		}
	private:
		std::string m_message;

	protected:
		std::string m_exceptionName;
	};

#define my_assert(cond) {if (!(cond)) {PLOG_DEBUG << "assert failed!";};	assert(cond);	if (!(cond)) {throw new TheWorld_Utils::Exception(__FUNCTION__, std::string(std::string("assert failed! Line number: ") + std::to_string(__LINE__)).c_str());};}

	typedef union FLOAT_32 FLOAT_32;
	union FLOAT_32
	{
		float    f32;
		uint32_t u32;
	};

	struct _RGB
	{
		BYTE r;
		BYTE g;
		BYTE b;
	};

	struct _RGBA
	{
		BYTE r;
		BYTE g;
		BYTE b;
		BYTE a;
	};

	static constexpr float kPi = 3.14159265358979323846f;
	static constexpr float kPi2 = 6.28318530717958647692f;
	static constexpr float kEpsilon = 0.0001f;
	static constexpr float kAreaEpsilon = FLT_EPSILON;
	static constexpr float kNormalEpsilon = 0.001f;

	class MemoryBuffer
	{
	public:
		__declspec(dllexport) MemoryBuffer(void);
		__declspec(dllexport) MemoryBuffer(BYTE* in, size_t size);
		__declspec(dllexport) ~MemoryBuffer(void);
		__declspec(dllexport) void set(BYTE* in, size_t size);
		__declspec(dllexport) void append(BYTE* in, size_t size);
		__declspec(dllexport) void reserve(size_t size);
		__declspec(dllexport) size_t reserved(void);
		__declspec(dllexport) void reset(void);
		__declspec(dllexport) BYTE* ptr();
		__declspec(dllexport) size_t size(void);
		__declspec(dllexport) bool empty(void);
		__declspec(dllexport) void clear(void);
		__declspec(dllexport) void adjustSize(size_t size);
		__declspec(dllexport) void copyFrom(MemoryBuffer& inBuffer);
		__declspec(dllexport) void populateFloatVector(std::vector<float>& v);
		__declspec(dllexport) void populateUint16Vector(std::vector<uint16_t>& v);
		__declspec(dllexport) void toString(std::string& buffer);
		template <typename T> __declspec(dllexport) T& at(size_t idx)
		{
			T* ptr = (T*)m_ptr;
			ptr += idx;
			return *ptr;
		}
		template <typename T> __declspec(dllexport) T& at(size_t x, size_t y, size_t size)
		{
			T* ptr = (T*)m_ptr;
			ptr += ((y * size) + x);
			return *ptr;
		}
		template <typename T> __declspec(dllexport) T* at_ptr(size_t x, size_t y, size_t size)
		{
			T* ptr = (T*)m_ptr;
			ptr += ((y * size) + x);
			return ptr;
		}
		template <typename T> __declspec(dllexport) void populateVector(std::vector<T>& v)
		{
			size_t numElements = size() / sizeof(T);
			my_assert(size() == numElements * sizeof(T));
			v.assign((T*)m_ptr, (T*)m_ptr + numElements);
		}

	private:
		MemoryBuffer(const MemoryBuffer&);
		void operator=(const MemoryBuffer&);

		BYTE* m_ptr;
		size_t m_size;
		size_t m_bufferSize;
	};

	class NoiseValues
	{
	public:
		int noiseType;
		int rotationType3D;
		int noiseSeed;
		float frequency;
		int fractalType;
		int fractalOctaves;
		float fractalLacunarity;
		float fractalGain;
		float fractalWeightedStrength;
		float fractalPingPongStrength;

		int cellularDistanceFunction;
		int cellularReturnType;
		float cellularJitter;

		int warpNoiseDomainWarpType;
		int warpNoiseRotationType3D;
		int warpNoiseSeed;
		float warpNoiseDomainWarpAmp;
		float warpNoiseFrequency;
		int warpNoieseFractalType;
		int warpNoiseFractalOctaves;
		float warpNoiseFractalLacunarity;
		float warpNoiseFractalGain;

		unsigned int amplitude;		// range of heights in WU (noise is from -1 to 1)
		float scaleFactor;
		float desideredMinHeight;
		bool desideredMinHeigthMandatory;

		bool operator==(const NoiseValues& o) const
		{
			if (noiseType != o.noiseType)
				return false;
			if (rotationType3D != o.rotationType3D)
				return false;
			if (noiseSeed != o.noiseSeed)
				return false;
			if (frequency != o.frequency)
				return false;
			if (fractalType != o.fractalType)
				return false;
			if (fractalOctaves != o.fractalOctaves)
				return false;
			if (fractalLacunarity != o.fractalLacunarity)
				return false;
			if (fractalGain != o.fractalGain)
				return false;
			if (fractalWeightedStrength != o.fractalWeightedStrength)
				return false;
			if (fractalPingPongStrength != o.fractalPingPongStrength)
				return false;
			if (cellularDistanceFunction != o.cellularDistanceFunction)
				return false;
			if (cellularReturnType != o.cellularReturnType)
				return false;
			if (cellularJitter != o.cellularJitter)
				return false;
			if (warpNoiseDomainWarpType != o.warpNoiseDomainWarpType)
				return false;
			if (warpNoiseRotationType3D != o.warpNoiseRotationType3D)
				return false;
			if (warpNoiseSeed != o.warpNoiseSeed)
				return false;
			if (warpNoiseDomainWarpAmp != o.warpNoiseDomainWarpAmp)
				return false;
			if (warpNoiseFrequency != o.warpNoiseFrequency)
				return false;
			if (warpNoieseFractalType != o.warpNoieseFractalType)
				return false;
			if (warpNoiseFractalOctaves != o.warpNoiseFractalOctaves)
				return false;
			if (warpNoiseFractalLacunarity != o.warpNoiseFractalLacunarity)
				return false;
			if (warpNoiseFractalGain != o.warpNoiseFractalGain)
				return false;
			if (amplitude != o.amplitude)
				return false;
			if (scaleFactor != o.scaleFactor)
				return false;
			if (desideredMinHeight != o.desideredMinHeight)
				return false;
			if (desideredMinHeigthMandatory != o.desideredMinHeigthMandatory)
				return false;
			return true;
		}
	};
	
	class TerrainSideInfo
	{
	public:
		bool needBlend;
		float minHeight;
		float maxHeight;
		bool operator==(const TerrainSideInfo& o) const
		{
			if (needBlend != o.needBlend)
				return false;
			if (minHeight != o.minHeight)
				return false;
			if (maxHeight != o.maxHeight)
				return false;
			return true;
		}
	};

	class TerrainEdit
	{
		class MeshCacheBuffer;

	public:
		__declspec(dllexport) static void generateGroundImage(std::string outdir, std::string groundTypeName, size_t imageSize, bool flipY, MemoryBuffer& colorImage, MemoryBuffer& bumpImage, MemoryBuffer& normalImage, MemoryBuffer& roughImage);
		__declspec(dllexport) static void generateGroundImage(MemoryBuffer& albedoBumpImage, MemoryBuffer& normalRoughnessImage, std::string groundTypeName, size_t imageSize, bool flipY, MemoryBuffer& colorImage, MemoryBuffer& bumpImage, MemoryBuffer& normalImage, MemoryBuffer& roughImage);

		enum class TextureType
		{
			lowElevation = 0
			, highElevation = 1
			, dirt = 2
			, rocks = 3
		};
		 
		enum class TerrainType
		{
			unknown = 0
			, jagged_1 = 1
			, campaign_1 = 2
			, plateau_1 = 3
			, low_hills = 4
			, high_hills = 5
			, low_mountains = 6
			, low_mountains_grow = 7
			, high_mountains_1 = 8
			, high_mountains_1_grow = 9
			, high_mountains_2 = 10
			, high_mountains_2_grow = 11
			, noise_1 = 12
		};

		__declspec(dllexport) static std::string getDefaultTextureNameForTerrainType(enum class TerrainType terrainType, enum class TextureType textureType);

		size_t size;
		bool needUploadToServer;
		bool normalsNeedRegen;
		enum class TerrainType terrainType;

		float minHeight;
		float maxHeight;

		TerrainSideInfo eastSideXPlus;		// X growing
		TerrainSideInfo westSideXMinus;		// X lowering
		TerrainSideInfo southSideZPlus;		// Z growing
		TerrainSideInfo northSideZMinus;	// Z lowering

		NoiseValues noise;

		struct
		{
			char lowElevationTexName_r[51];
			char highElevationTexName_g[51];
			char dirtTexName_b[51];
			char rocksTexName_a[51];
			bool splatmapNeedRegen;
			bool emptyColormap;
			bool emptyGlobalmap;
		} extraValues;

		__declspec(dllexport) TerrainEdit(enum class TerrainEdit::TerrainType terrainType = TerrainEdit::TerrainType::campaign_1);
		__declspec(dllexport) void setTextureNameForTerrainType(enum class TerrainEdit::TextureType textureType);
		__declspec(dllexport) void init(enum class TerrainEdit::TerrainType terrainType);
		__declspec(dllexport) void serialize(TheWorld_Utils::MemoryBuffer& buffer);
		__declspec(dllexport) void deserialize(TheWorld_Utils::MemoryBuffer& buffer);
		__declspec(dllexport) static std::string terrainTypeString(enum class TerrainEdit::TerrainType terrainType);
		__declspec(dllexport) static enum class TerrainEdit::TerrainType terrainTypeEnum(std::string& terrainType);
		__declspec(dllexport) void adjustValues(TerrainEdit* northSideZMinus, TerrainEdit* southSideZPlus, TerrainEdit* westSideXMinus, TerrainEdit* eastSideXPlus);
		__declspec(dllexport) bool operator==(const TerrainEdit& terrainEdit) const;
	};

	template<> MYAPI void MemoryBuffer::populateVector<float>(std::vector<float>& v);

	template <class TimeT = std::chrono::milliseconds, class ClockT = std::chrono::steady_clock> class Timer
	{
		using timep_t = typename ClockT::time_point;
		timep_t _start = ClockT::now(), _end = {};

	public:
		Timer(std::string method = "", std::string headerMsg = "", bool tickMsg = false, bool tockMsg = false)
		{
			_counterStarted = false;
			_headerMsg = headerMsg;
			_save_headerMsg = headerMsg;
			_method = method;
			_tickMsg = tickMsg;
			_tockMsg = tockMsg;
		}

		~Timer()
		{
			if (_tockMsg)
				tock();
		}

		void tick()
		{
			_end = timep_t{};
			_start = ClockT::now();
			_counterStarted = true;

			if (_tickMsg)
			{
				std::string method = _method;
				if (method.length() > 0)
					method += " ";
				std::string headerMsg = _headerMsg;
				if (headerMsg.length() > 0)
					headerMsg = "- " + headerMsg;
				else
					headerMsg = "-";
				PLOG_DEBUG << std::string("START ") + method + headerMsg;
			}
		}

		void tock()
		{
			if (_counterStarted)
			{
				_end = ClockT::now();
				_counterStarted = false;

				if (_tockMsg)
				{
					std::string method = _method;
					if (method.length() > 0)
						method += " ";
					std::string headerMsg = _headerMsg;
					if (headerMsg.length() > 0)
						headerMsg = "- " + headerMsg;
					else
						headerMsg = "-";
					PLOG_DEBUG << std::string("ELAPSED ") + method + headerMsg + " ==> " + std::to_string(duration().count()).c_str() + " ms";
				}
			}
		}

		void headerMsg(std::string headerMsg)
		{
			_headerMsg = headerMsg;
		}

		void headerMsgSuffix(std::string _headerMsgSuffix)
		{
			_headerMsg = _save_headerMsg + _headerMsgSuffix;
		}

		void method(std::string method)
		{
			_method = method;
		}

		bool counterStarted(void)
		{
			return _counterStarted;
		}

		template <class TT = TimeT>	TT partialDuration() const
		{
			return std::chrono::duration_cast<TT>(ClockT::now() - _start);
		}

		template <class TT = TimeT>	TT duration() const
		{
			Expects(_end != timep_t{} && "toc before reporting");
			//assert(_end != timep_t{} && "toc before reporting");
			return std::chrono::duration_cast<TT>(_end - _start);
		}

	private:
		bool _counterStarted;
		std::string _headerMsg;
		std::string _save_headerMsg;
		std::string _method;
		bool _tickMsg;
		bool _tockMsg;

	};

#define TimerMs Timer<std::chrono::milliseconds, std::chrono::steady_clock>
#define TimerMcs Timer<std::chrono::microseconds, std::chrono::steady_clock>

	class GridVertex;

	class MeshCacheBuffer
	{
	public:
		class CacheQuadrantData
		{
		public:
			std::string meshId;
			TheWorld_Utils::MemoryBuffer* terrainEditValues = nullptr;
			float minHeight = 0.0f;
			float maxHeight = 0.0f;
			TheWorld_Utils::MemoryBuffer* heights16Buffer = nullptr;
			TheWorld_Utils::MemoryBuffer* heights32Buffer = nullptr;
			TheWorld_Utils::MemoryBuffer* normalsBuffer = nullptr;
			TheWorld_Utils::MemoryBuffer* splatmapBuffer = nullptr;
			TheWorld_Utils::MemoryBuffer* colormapBuffer = nullptr;
			TheWorld_Utils::MemoryBuffer* globalmapBuffer = nullptr;
			bool heightsUpdated = false;
			bool normalsUpdated = false;
			bool texturesUpdated = false;
		};

	public:
		// char "0"
		// size_t	mesh id size
		// char*	mesh id
		// size_t	class TerrainEdit size
		// TerrainEdit deserialized
		// size_t num vertices
		//		float	min heigth		==> num vertices > 0
		//		float	man heigth		==> num vertices > 0
		//		float16 heightmap		(with size = num vertices * sizeof(uint16_t)) ==> num vertices > 0
		//		float32 heightmap		(with size = num vertices * sizeof(float)) ==> num vertices > 0
		//		RGB normalmap			(with size = num vertices * sizeof(RGB) oppure = sizeof(RGB) se il primo colore ha r=0, g=0, b=0) ==> num vertices > 0

		__declspec(dllexport) MeshCacheBuffer(void);
		__declspec(dllexport) ~MeshCacheBuffer(void);
		__declspec(dllexport) MeshCacheBuffer(std::string cacheDir, std::string mapName, float gridStepInWU, size_t numVerticesPerSize, int level, float lowerXGridVertex, float lowerZGridVertex);
		__declspec(dllexport) MeshCacheBuffer(const MeshCacheBuffer& c);

		__declspec(dllexport) void operator=(const MeshCacheBuffer& c);

		__declspec(dllexport) static uint16_t halfFromFloat(uint32_t f);
		__declspec(dllexport) static uint32_t halfToFloat(uint16_t f);

		static std::string cacheDir(std::string cacheDir, std::string mapName, float gridStepInWU, size_t numVerticesPerSize, int level);
		__declspec(dllexport) static std::string generateNewMeshId(void);
		__declspec(dllexport) static std::string generateNewMeshIdForEmptyBuffer(void);
		__declspec(dllexport) static bool isEmptyBuffer(std::string meshId);
		__declspec(dllexport) static bool firstMeshIdMoreRecent(std::string firstMeshId, std::string secondMeshId);
		__declspec(dllexport) std::string getMeshIdFromDisk(void);
		__declspec(dllexport) static void getAllDiskCache(std::string cacheDir, std::string mapName, float gridStepInWU, size_t numVerticesPerSize, int level, std::vector<MeshCacheBuffer>& vectDiskCache);
		__declspec(dllexport) std::string getCacheIdStr(void);
		__declspec(dllexport) bool refreshMapsFromDisk(size_t numVerticesPerSize, float gridStepInWU, std::string meshId, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitde, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer);
		__declspec(dllexport) void refreshMapsFromBuffer(const BYTE* buffer, const size_t bufferSize, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitde, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer, bool updateCache);
		__declspec(dllexport) void refreshMapsFromBuffer(TheWorld_Utils::MemoryBuffer& buffer, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitde, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer, bool updateCache);
		__declspec(dllexport) void refreshMapsFromBuffer(std::string& buffer, std::string& meshIdFromBuffer, TheWorld_Utils::MemoryBuffer& terrainEditValues, float& minAltitde, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer, TheWorld_Utils::MemoryBuffer& colormapBuffer, TheWorld_Utils::MemoryBuffer& globalmapBuffer, bool updateCache);
		__declspec(dllexport) void refreshCacheQuadrantDataFromBuffer(const BYTE* buffer, const size_t bufferSize, CacheQuadrantData& cacheQuadrantData, bool updateCache);
		__declspec(dllexport) void refreshCacheQuadrantDataFromBuffer(TheWorld_Utils::MemoryBuffer& buffer, CacheQuadrantData& cacheQuadrantData, bool updateCache);
		__declspec(dllexport) void refreshCacheQuadrantDataFromBuffer(std::string& buffer, CacheQuadrantData& cacheQuadrantData, bool updateCache);
		__declspec(dllexport) void readBufferFromDisk(std::string meshId, TheWorld_Utils::MemoryBuffer& buffer);
		__declspec(dllexport) void readBufferFromDisk(std::string meshId, std::string& buffer);
		__declspec(dllexport) void writeBufferToDiskCache(std::string& buffer, bool renewMeshId = false);
		__declspec(dllexport) void writeBufferToDiskCache(TheWorld_Utils::MemoryBuffer& buffer, bool renewMeshId = false);
		__declspec(dllexport) void writeBufferToDiskCache(const BYTE* buffer, const size_t bufferSize, bool renewMeshId = false);
		__declspec(dllexport) void setBufferFromHeights(std::string meshId, size_t numVerticesPerSize, float gridStepInWU, TheWorld_Utils::MemoryBuffer& terrainEditValuesBuffer, std::vector<float>& vectGridHeights, std::string& buffer, float& minAltitude, float& maxAltitude, bool generateNormals);
		__declspec(dllexport) void setBufferFromCacheQuadrantData(size_t numVerticesPerSize, CacheQuadrantData& cacheQuadrantData, TheWorld_Utils::MemoryBuffer& buffer);
		__declspec(dllexport) void setBufferFromCacheQuadrantData(size_t numVerticesPerSize, CacheQuadrantData& cacheQuadrantData, std::string& buffer);
		__declspec(dllexport) void setEmptyBuffer(size_t numVerticesPerSize, std::string& meshId, TheWorld_Utils::MemoryBuffer& buffer);
		__declspec(dllexport) void generateHeightsWithNoise(size_t numVerticesPerSize, float gridStepInWU, float lowerXGridVertex, float lowerZGridVertex, TerrainEdit* terrainEdit, std::vector<float>& vectGridHeights);
		__declspec(dllexport) void applyWorldModifier(int level, size_t numVerticesPerSize, float gridStepInWU, float lowerXGridVertex, float lowerZGridVertex, TerrainEdit* terrainEdit, std::vector<float>& vectGridHeights, WorldModifier& wm);
		__declspec(dllexport) void generateNormalsForBlendedQuadrants(size_t numVerticesPerSize, float gridStepInWU, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& east_float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& south_float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool& requestedExit);
		__declspec(dllexport) void generateSplatmap(size_t numVerticesPerSize, float gridStepInWU, TheWorld_Utils::TerrainEdit* terrainEdit, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, TheWorld_Utils::MemoryBuffer& splatmapBuffer,
													float slopeVerticalFactor = 4.0f, float slopeFlatFactor = 1.0f, float dirtOnRocksFactor = 2.0f, float highElevationFactor = 4.0f, float lowElevationFactor = 2.0f, size_t splatMapMode = 1);
		__declspec(dllexport) void deprecated_generateNormals(size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, TheWorld_Utils::MemoryBuffer& normalsBuffer);
		__declspec(dllexport) void deprecated_generateNormals(size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, BYTE* normalsBuffer, const size_t normalsBufferSize, size_t& usedBufferSize);
		__declspec(dllexport) void calcMinMxHeight(size_t numVerticesPerSize, TheWorld_Utils::TerrainEdit* terrainEdit, TheWorld_Utils::MemoryBuffer& heights32Buffer);
		__declspec(dllexport) bool blendBorders(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
			CacheQuadrantData& data,
			CacheQuadrantData& northData,
			CacheQuadrantData& southData,
			CacheQuadrantData& westData,
			CacheQuadrantData& eastData);
		__declspec(dllexport) bool blendQuadrant(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
			CacheQuadrantData& data,
			CacheQuadrantData& northData,
			CacheQuadrantData& southData,
			CacheQuadrantData& westData,
			CacheQuadrantData& eastData,
			CacheQuadrantData& northwestData,
			CacheQuadrantData& northeastData,
			CacheQuadrantData& southwesthData,
			CacheQuadrantData& southeastData);
		__declspec(dllexport) bool blendQuadrantOnNorthSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
			CacheQuadrantData& data,
			CacheQuadrantData& northData,
			CacheQuadrantData& southData,
			CacheQuadrantData& westData,
			CacheQuadrantData& eastData,
			CacheQuadrantData& northwestData,
			CacheQuadrantData& northeastData,
			CacheQuadrantData& southwesthData,
			CacheQuadrantData& southeastData);
		__declspec(dllexport) bool blendQuadrantOnSouthSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
			CacheQuadrantData& data,
			CacheQuadrantData& northData,
			CacheQuadrantData& southData,
			CacheQuadrantData& westData,
			CacheQuadrantData& eastData,
			CacheQuadrantData& northwestData,
			CacheQuadrantData& northeastData,
			CacheQuadrantData& southwesthData,
			CacheQuadrantData& southeastData);
		__declspec(dllexport) bool blendQuadrantOnWestSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
			CacheQuadrantData& data,
			CacheQuadrantData& northData,
			CacheQuadrantData& southData,
			CacheQuadrantData& westData,
			CacheQuadrantData& eastData,
			CacheQuadrantData& northwestData,
			CacheQuadrantData& northeastData,
			CacheQuadrantData& southwesthData,
			CacheQuadrantData& southeastData);
		__declspec(dllexport) bool blendQuadrantOnEastSide(size_t numVerticesPerSize, float gridStepInWU, bool lastPhase,
			CacheQuadrantData& data,
			CacheQuadrantData& northData,
			CacheQuadrantData& southData,
			CacheQuadrantData& westData,
			CacheQuadrantData& eastData,
			CacheQuadrantData& northwestData,
			CacheQuadrantData& northeastData,
			CacheQuadrantData& southwesthData,
			CacheQuadrantData& southeastData);

		//__declspec(dllexport) std::string getCacheDir()
		//{
		//	return m_cacheDir;
		//}

		__declspec(dllexport) std::string getMeshId(void)
		{
			return m_meshId;
		}
		__declspec(dllexport) std::string getMapName(void)
		{
			return m_mapName;
		}
		__declspec(dllexport) float getGridStepInWU(void)
		{
			return m_gridStepInWU;
		}
		__declspec(dllexport) size_t getNumVerticesPerSize(void)
		{
			return m_numVerticesPerSize;
		}
		__declspec(dllexport) int getLevel(void)
		{
			return m_level;
		}
		__declspec(dllexport) float getLowerXGridVertex(void)
		{
			return m_lowerXGridVertex;
		}
		__declspec(dllexport) float getLowerZGridVertex(void)
		{
			return m_lowerZGridVertex;
		}
		__declspec(dllexport) std::string getCacheFilePath(void)
		{
			return m_meshFilePath;
		}

	private:
		std::string m_meshFilePath;
		std::string m_cacheDir;
		std::string m_meshId;
		//std::string m_buffer;
		std::string m_mapName;
		float m_gridStepInWU;
		size_t m_numVerticesPerSize;
		int m_level;
		float m_lowerXGridVertex;
		float m_lowerZGridVertex;
	};
	
	std::string ToString(GUID* guid);

	class Utils
	{
	public:
		__declspec(dllexport) static void plogInit(plog::Severity sev, plog::IAppender* appender);
		__declspec(dllexport) static void plogSetMaxSeverity(plog::Severity sev);
		__declspec(dllexport) static void plogDenit(void);

		static std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace);

		static void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);

		static bool isEqualWithLimitedPrecision(float num1, float num2, int precision)
		{
			float diff = abs(num1 - num2);

			float value = 0.0F;
			if (precision == 7)
				value = 0.0000001F;
			else if (precision == 6)
				value = 0.000001F;
			else if (precision == 5)
				value = 0.00001F;
			else if (precision == 4)
				value = 0.0001F;
			else if (precision == 3)
				value = 0.001F;
			else if (precision == 2)
				value = 0.01F;
			else if (precision == 1)
				value = 0.1F;

			if (diff < value)
				return true;

			return false;
		}

		static bool isEqualVectorWithLimitedPrecision(Eigen::Vector3d v1, Eigen::Vector3d v2, int precision)
		{
			if (isEqualWithLimitedPrecision((float)v1.x(), (float)v2.x(), precision) && isEqualWithLimitedPrecision((float)v1.y(), (float)v2.y(), precision) && isEqualWithLimitedPrecision((float)v1.z(), (float)v2.z(), precision))
				return true;
			else
				return false;
		}

		static int align(int x, int a)
		{
			return (x + a - 1) & ~(a - 1);
		}

		/*
		 * Case Insensitive String Comparision
		 */
		//static bool caseInSensStringEqual(std::string& str1, std::string& str2)
		//{
		//	return boost::iequals(str1, str2);
		//}

		/*
		 * Case Insensitive String Comparision
		 */
		//static bool caseInSensWStringEqual(std::wstring& str1, std::wstring& str2)
		//{
		//	return boost::iequals(str1, str2);
		//}

		static float square(float f) 
		{
			return f * f;
		}

		static bool isPowerOfTwo(int n)
		{
			if (n == 0)
				return false;

			return (ceil(log2(n)) == floor(log2(n)));
		}

		/** Return the next power of two.
		* @see http://graphics.stanford.edu/~seander/bithacks.html
		* @warning Behaviour for 0 is undefined.
		* @note isPowerOfTwo(x) == true -> nextPowerOfTwo(x) == x
		* @note nextPowerOfTwo(x) = 2 << log2(x-1)
		*/
		static uint32_t nextPowerOfTwo(uint32_t x) 
		{
			assert(x != 0);
			// On modern CPUs this is supposed to be as fast as using the bsr instruction.
			x--;
			x |= x >> 1;
			x |= x >> 2;
			x |= x >> 4;
			x |= x >> 8;
			x |= x >> 16;
			return x + 1;
		}

		template <typename T>
		static T max2(const T& a, const T& b) 
		{
			return a > b ? a : b;
		}

		template <typename T>
		static T min2(const T& a, const T& b) 
		{
			return a < b ? a : b;
		}

		template <typename T>
		static T max3(const T& a, const T& b, const T& c) 
		{
			return max2(a, max2(b, c));
		}

		/// Return the maximum of the three arguments.
		template <typename T>
		static T min3(const T& a, const T& b, const T& c) 
		{
			return min2(a, min2(b, c));
		}

		/// Clamp between two values.
		template <typename T>
		static T clamp(const T& x, const T& a, const T& b) 
		{
			return Utils::min2(Utils::max2(x, a), b);
		}

		template <typename T>
		static void swap(T& a, T& b) 
		{
			T temp = a;
			a = b;
			b = temp;
		}

		union FloatUint32 {
			float f;
			uint32_t u;
		};

		static bool isFinite(float f) 
		{
			FloatUint32 fu;
			fu.f = f;
			return fu.u != 0x7F800000u && fu.u != 0x7F800001u;
		}

		static bool isNan(float f) 
		{
			return f != f;
		}

		static int ftoi_ceil(float val) 
		{
			return (int)ceilf(val);
		}

		static bool isZero(const float f, const float epsilon) 
		{
			return fabs(f) <= epsilon;
		}

		
		// Robust floating point comparisons:
		// http://realtimecollisiondetection.net/blog/?p=89
		//static bool equal(const float f0, const float f1, const float epsilon = 0.00001);
		static std::vector<std::string> split(std::string text, char delim)
		{
			std::string line;
			std::vector<std::string> vec;
			std::stringstream ss(text);
			while (std::getline(ss, line, delim)) {
				vec.push_back(line);
			}
			return vec;
		}
	
		static float getDistance(float x1, float y1, float x2, float y2)
		{
			return sqrtf((powf((x2 - x1), 2.0) + powf((y2 - y1), 2.0)));
		}
	};

	class GDN_TheWorld_Exception : public std::exception
	{
	public:
		_declspec(dllexport) GDN_TheWorld_Exception(const char* function, const char* message = NULL, const char* message2 = NULL, int rc = 0)
		{
			m_exceptionName = "MapManagerException";
			if (message == NULL || strlen(message) == 0)
				m_message = "MapManager Generic Exception - C++ Exception";
			else
			{
				if (message2 == NULL || strlen(message2) == 0)
				{
					m_message = message;
				}
				else
				{
					m_message = message;
					m_message += " - ";
					m_message += message2;
					m_message += " - rc=";
					m_message += std::to_string(rc);
				}
			}
		};
		_declspec(dllexport) ~GDN_TheWorld_Exception() {};

		const char* what() const throw ()
		{
			return m_message.c_str();
		}

		const char* exceptionName()
		{
			return m_exceptionName.c_str();
		}
	private:
		std::string m_message;

	protected:
		std::string m_exceptionName;
	};


	template <typename F>
	struct FinalAction {
		FinalAction(F f) : final_{ f } {}
		~FinalAction()
		{
			if (enabled_) final_(); 
		}
		void disable() 
		{
			enabled_ = false; 
		};
	private:
		F final_;
		bool enabled_{ true };
	};

	template <typename F> FinalAction<F> finally(F f)
	{
		return FinalAction<F>(f); 
	}

	typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> MsTimePoint;

	class ThreadInitDeinit
	{
		friend class ThreadPool;
		virtual void threadInit(void) = 0;
		virtual void threadDeinit(void) = 0;
	};

	class ThreadPool
	{
	public:
		__declspec(dllexport) void Start(std::string label, size_t num_threads = 0, /*const std::function<void()>* threadInitFunction = nullptr, const std::function<void()>* threadDeinitFunction = nullptr,*/ ThreadInitDeinit* threadInitDeinit = nullptr);
		__declspec(dllexport) void QueueJob(const std::function<void()>& job);
		__declspec(dllexport) void Stop();
		__declspec(dllexport) bool busy();
		__declspec(dllexport) size_t getNumWorkingThreads(size_t& m_maxThreads);
		__declspec(dllexport) bool allThreadsWorking(void);

	private:
		void ThreadLoop();

		std::string m_label;
		bool m_should_terminate = false;           // Tells threads to stop looking for jobs
		size_t m_workingThreads = 0;
		std::mutex m_queue_mutex;                  // Prevents data races to the job queue
		std::condition_variable m_mutex_condition; // Allows threads to wait on new jobs or termination 
		std::vector<std::thread> m_threads;
		std::queue<std::function<void()>> m_jobs;
		//const std::function<void()>* m_threadInitFunction = nullptr;
		//const std::function<void()>* m_threadDeinitFunction = nullptr;
		ThreadInitDeinit* m_threadInitDeinit = nullptr;
		TheWorld_Utils::MsTimePoint m_lastDiagnosticTime = std::chrono::time_point_cast<TheWorld_Utils::MsTimePoint::duration>(std::chrono::system_clock::now());
		bool m_lastAllWorkingStatus = false;
	};

	template <typename T>
	void serializeToByteStream(T in, BYTE* stream, size_t& size)
	{
		T* pIn = &in;
		BYTE* pc = reinterpret_cast<BYTE*>(pIn);
		for (size_t i = 0; i < sizeof(T); i++)
		{
			stream[i] = *pc;
			pc++;
		}
		size = sizeof(T);
	}

	template <typename T>
	T deserializeFromByteStream(BYTE* stream, size_t& size)
	{
		T* pOut = reinterpret_cast<T*>(stream);
		size = sizeof(T);
		return *pOut;
	}

	/*static float roundToDigit(float num, int digit)
	{
		//float value = (int)(num * pow(10.0, digit) + .5);
		//float v = (float)value / (int)pow(10.0, digit);
		//return v;

		char str[40];
		sprintf(str, "%.6f", num);
		sscanf(str, "%f", &num);
		return num;
	}*/

	static bool equal(const float f0, const float f1, const float epsilon = 0.00001)
	{
		//return fabs(f0-f1) <= epsilon;
		return fabs(f0 - f1) <= epsilon * TheWorld_Utils::Utils::max3(1.0f, fabsf(f0), fabsf(f1));
	}
	
	//void X_aligned_memcpy_sse2(void* dest, const void* src, const unsigned long size)
	//{

	//	__asm
	//	{
	//		mov esi, src;    //src pointer
	//		mov edi, dest;   //dest pointer

	//		mov ebx, size;   //ebx is our counter 
	//		shr ebx, 7;      //divide by 128 (8 * 128bit registers)


	//	loop_copy:
	//		prefetchnta 128[ESI]; //SSE2 prefetch
	//		prefetchnta 160[ESI];
	//		prefetchnta 192[ESI];
	//		prefetchnta 224[ESI];

	//		movdqa xmm0, 0[ESI]; //move data from src to registers
	//		movdqa xmm1, 16[ESI];
	//		movdqa xmm2, 32[ESI];
	//		movdqa xmm3, 48[ESI];
	//		movdqa xmm4, 64[ESI];
	//		movdqa xmm5, 80[ESI];
	//		movdqa xmm6, 96[ESI];
	//		movdqa xmm7, 112[ESI];

	//		movntdq 0[EDI], xmm0; //move data from registers to dest
	//		movntdq 16[EDI], xmm1;
	//		movntdq 32[EDI], xmm2;
	//		movntdq 48[EDI], xmm3;
	//		movntdq 64[EDI], xmm4;
	//		movntdq 80[EDI], xmm5;
	//		movntdq 96[EDI], xmm6;
	//		movntdq 112[EDI], xmm7;

	//		add esi, 128;
	//		add edi, 128;
	//		dec ebx;

	//		jnz loop_copy; //loop please
	//	loop_copy_end:
	//	}
	//}
}

