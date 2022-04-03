#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "Debut/dbtpch.h"

#ifdef DBT_ASSERTS
	#define DBT_ASSERT(x, ...) { if(!(x)) { Debut::Log.AppFatal("Assertion failed. %s", __VA_ARGS__); __debugbreak(); }}
	#define DBT_CORE_ASSERT(x, ...) { if(!(x)) { Debut::Log.CoreFatal("Assertion failed. %s", __VA_ARGS__); __debugbreak(); }}
#else
	#define DBT_ASSERT(x, ...)
	#define DBT_CORE_ASSERT(x, ...)
#endif
	

namespace Debut
{
	class _Log
	{
	public:
		_Log() {  }

		void Init();

		template<typename ...Args>
		void CoreInfo(const char* fmt, Args &&... args)
		{
			m_CoreLogger->info(fmt, args...);
		}

		template<typename ...Args>
		void CoreWarn(const char* fmt, Args &&... args)
		{
			m_CoreLogger->warn(fmt, args...);
		}

		template<typename ...Args>
		void CoreError(const char* fmt, Args &&... args)
		{
			m_CoreLogger->error(fmt, args...);
		}

		template<typename ...Args>
		void CoreFatal(const char* fmt, Args &&... args)
		{
			m_CoreLogger->critical(fmt, args...);
		}


		template<typename ...Args>
		void AppInfo(const char* fmt, Args &&... args)
		{
			m_AppLogger->info(fmt, args...);
		}

		template<typename ...Args>
		void AppWarn(const char* fmt, Args &&... args)
		{
			m_AppLogger->warn(fmt, args...);
		}

		template<typename ...Args>
		void AppError(const char* fmt, Args &&... args)
		{
			m_AppLogger->error(fmt, args...);
		}

		template<typename ...Args>
		void AppFatal(const char* fmt, Args &&... args)
		{
			m_AppLogger->critical(fmt, args...);
		}

	private:
		std::shared_ptr<spdlog::logger> m_CoreLogger;
		std::shared_ptr<spdlog::logger> m_AppLogger;
	};

	extern DBT_API _Log Log;
}


