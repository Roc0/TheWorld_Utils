#pragma once

//#ifdef _THEWORLD_CLIENT
//	#include <Godot.hpp>
//	#include <ImageTexture.hpp>
//	#include <PoolArrays.hpp>
//#endif

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

//#include <Godot.hpp>
//#include <Vector3.hpp>
//#include <boost/algorithm/string.hpp>

#define Vector3Zero Vector3(0, 0, 0)
#define Vector3UP Vector3(0, 1, 0)

#define Vector3X Vector3(1, 0, 1)
#define Vector3Y Vector3(0, 1, 0)
#define Vector3Z Vector3(0, 0, 1)

namespace TheWorld_Utils
{
	// https://cellperformance.beyond3d.com/articles/2006/07/branchfree_implementation_of_h_1.html

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

	static constexpr float kPi = 3.14159265358979323846f;
	static constexpr float kPi2 = 6.28318530717958647692f;
	static constexpr float kEpsilon = 0.0001f;
	static constexpr float kAreaEpsilon = FLT_EPSILON;
	static constexpr float kNormalEpsilon = 0.001f;

	class MemoryBuffer
	{
	public:
		__declspec(dllexport) MemoryBuffer(void);
		__declspec(dllexport) ~MemoryBuffer(void);
		__declspec(dllexport) void set(BYTE* in, size_t len);
		__declspec(dllexport) void append(BYTE* in, size_t len);
		__declspec(dllexport) void reserve(size_t len);
		__declspec(dllexport) void reset(void);
		__declspec(dllexport) BYTE* ptr();
		__declspec(dllexport) size_t len(void);
		__declspec(dllexport) bool empty(void);
		__declspec(dllexport) void clear(void);

	private:
		MemoryBuffer(const MemoryBuffer&);
		void operator=(const MemoryBuffer&);

		BYTE* m_ptr;
		size_t m_len;
		size_t m_bufferLen;
	};

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
		__declspec(dllexport) MeshCacheBuffer(void);
		__declspec(dllexport) ~MeshCacheBuffer(void);
		__declspec(dllexport) MeshCacheBuffer(std::string cacheDir, float gridStepInWU, size_t numVerticesPerSize, int level, float lowerXGridVertex, float lowerZGridVertex);
		__declspec(dllexport) MeshCacheBuffer(const MeshCacheBuffer& c);

		__declspec(dllexport) void operator=(const MeshCacheBuffer& c);

		__declspec(dllexport) std::string getMeshIdFromMeshCache(void);
		__declspec(dllexport) void refreshMapsFromBuffer(std::string buffer, std::string& meshIdFromBuffer, float& minAltitde, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer, bool updateCache);
		__declspec(dllexport) void readBufferFromMeshCache(std::string meshId, std::string& buffer, size_t& vectSizeFromCache);
		__declspec(dllexport) void readMapsFromMeshCache(std::string meshId, float& minAltitde, float& maxAltitude, TheWorld_Utils::MemoryBuffer& float16HeigthsBuffer, TheWorld_Utils::MemoryBuffer& float32HeigthsBuffer, TheWorld_Utils::MemoryBuffer& normalsBuffer);
		__declspec(dllexport) void writeBufferToMeshCache(std::string buffer);
		__declspec(dllexport) void setBufferForMeshCache(std::string meshId, size_t numVerticesPerSize, float gridStepInWU, std::vector<float>& vectGridHeights, std::string& buffer);
		//__declspec(dllexport) std::string getCacheDir()
		//{
		//	return m_cacheDir;
		//}

		__declspec(dllexport) std::string getMeshId()
		{
			return m_meshId;
		}

//#ifdef _THEWORLD_CLIENT
//		//void writeHeightmap(godot::Ref<godot::Image> heightMapImage);
//		//void writeNormalmap(godot::Ref<godot::Image> normalMapImage);
//		//godot::Ref<godot::Image> readHeigthmap(bool& ok);
//		//godot::Ref<godot::Image> readNormalmap(bool& ok);
//		enum class ImageType
//		{
//			heightmap = 0,
//			normalmap = 1
//		};
//
//		__declspec(dllexport) void writeImage(godot::Ref<godot::Image> image, enum class ImageType type);
//		__declspec(dllexport) godot::Ref<godot::Image> readImage(bool& ok, enum class ImageType type);
//#endif

	private:
		std::string m_meshFilePath;
		std::string m_cacheDir;
		std::string m_meshId;
		std::string m_buffer;
		float m_gridStepInWU;
		size_t m_numVerticesPerSize;
		int m_level;
		float m_lowerXGridVertex;
		float m_lowerZGridVertex;

//#ifdef _THEWORLD_CLIENT
//		std::string m_heightmapFilePath;
//		std::string m_normalmapFilePath;
//#endif
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

	/*static bool isEqualVectorWithLimitedPrecision(godot::Vector3 v1, godot::Vector3 v2, int precision)
	{
		if (isEqualWithLimitedPrecision(v1.x, v2.x, precision) && isEqualWithLimitedPrecision(v1.y, v2.y, precision) && isEqualWithLimitedPrecision(v1.z, v2.z, precision))
			return true;
		else
			return false;
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

