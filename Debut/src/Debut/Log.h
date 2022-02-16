#pragma once

#include "Core.h"
#include "Debut/dbtpch.h"

#define ADD_PREFIX(fmt, prefix)					\
	std::stringstream ss;						\
	ss << "[" << #prefix << "] " << fmt;		

#define DBT_CORE_FATAL(fmt, ...)				\
	ADD_PREFIX(fmt, CORE)						\
	LOG_F(FATAL, ss.str().c_str(), __VA_ARGS__)

#define DBT_CORE_ERROR(fmt, ...)				\
	ADD_PREFIX(fmt, CORE)						\
	LOG_F(ERROR, ss.str().c_str(), __VA_ARGS__)

#define DBT_CORE_WARN(fmt, ...)				\
	ADD_PREFIX(fmt, CORE)						\
	LOG_F(WARNING, ss.str().c_str(), __VA_ARGS__)

#define DBT_CORE_INFO(fmt, ...)				\
	ADD_PREFIX(fmt, CORE)						\
	LOG_F(INFO, ss.str().c_str(), __VA_ARGS__)


#define DBT_FATAL(fmt, ...)						\
	ADD_PREFIX(fmt, APP)						\
	LOG_F(FATAL, ss.str().c_str(), __VA_ARGS__)

#define DBT_ERROR(fmt, ...)						\
	ADD_PREFIX(fmt, APP)						\
	LOG_F(ERROR, ss.str().c_str(), __VA_ARGS__)

#define DBT_WARN(fmt, ...)						\
	ADD_PREFIX(fmt, APP)						\
	LOG_F(WARNING, ss.str().c_str(), __VA_ARGS__)

#define DBT_INFO(fmt, ...)						\
	ADD_PREFIX(fmt, APP)						\
	LOG_F(INFO, ss.str().c_str(), __VA_ARGS__)
	

namespace Debut
{
	class DBT_API _Log
	{
	public:
		_Log() {}

		void Init();

		template<typename ...Args>
		void CoreInfo(const char* fmt, Args &&... args)
		{
			DBT_CORE_INFO(fmt, args...);
		}

		template<typename ...Args>
		void CoreWarn(const char* fmt, Args &&... args)
		{
			DBT_CORE_WARN(fmt, args...);
		}

		template<typename ...Args>
		void CoreError(const char* fmt, Args &&... args)
		{
			DBT_CORE_ERROR(fmt, args...);
		}

		template<typename ...Args>
		void CoreFatal(const char* fmt, Args &&... args)
		{
			DBT_CORE_FATAL(fmt, args...);
		}


		template<typename ...Args>
		void AppInfo(const char* fmt, Args &&... args)
		{
			DBT_INFO(fmt, args...);
		}

		template<typename ...Args>
		void AppWarn(const char* fmt, Args &&... args)
		{
			DBT_WARN(fmt, args...);
		}

		template<typename ...Args>
		void AppError(const char* fmt, Args &&... args)
		{
			DBT_ERROR(fmt, args...);
		}

		template<typename ...Args>
		void AppFatal(const char* fmt, Args &&... args)
		{
			DBT_FATAL(fmt, args...);
		}
	};

	extern DBT_API _Log Log;
}


