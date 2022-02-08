#pragma once

#include "Core.h"
#include "easylogging++.h"

#define DBT_CORE_FATAL(...)		::Debut::Log.GetCoreLogger()->fatal(__VA_ARGS__)
#define DBT_CORE_ERROR(...)		::Debut::Log.GetCoreLogger()->error(__VA_ARGS__)
#define DBT_CORE_WARN(...)		::Debut::Log.GetCoreLogger()->warn(__VA_ARGS__)
#define DBT_CORE_INFO(...)		::Debut::Log.GetCoreLogger()->info(__VA_ARGS__)
#define DBT_CORE_TRACE(...)		::Debut::Log.GetCoreLogger()->trace(__VA_ARGS__)

#define DBT_FATAL(...)		::Debut::Log.GetClientLogger()->fatal(__VA_ARGS__)
#define DBT_ERROR(...)		::Debut::Log.GetClientLogger()->error(__VA_ARGS__)
#define DBT_WARN(...)		::Debut::Log.GetClientLogger()->warn(__VA_ARGS__)
#define DBT_INFO(...)		::Debut::Log.GetClientLogger()->info(__VA_ARGS__)
#define DBT_TRACE(...)		::Debut::Log.GetClientLogger()->trace(__VA_ARGS__)

namespace Debut
{
	class DBT_API _Log
	{
	public:
		_Log() {}

		void Init();

		void Shutdown();

		el::Logger* GetCoreLogger() { return m_CoreLogger; }

		el::Logger* GetClientLogger() { return m_ClientLogger; }

	private:
		el::Logger* m_CoreLogger;
		el::Logger* m_ClientLogger;
	};

	extern DBT_API _Log Log;
}


