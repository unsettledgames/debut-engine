#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define DBT_CORE_FATAL(...)		::Debut::Log.GetCoreLogger()->critical(__VA_ARGS__)
#define DBT_CORE_ERROR(...)		::Debut::Log.GetCoreLogger()->error(__VA_ARGS__)
#define DBT_CORE_WARN(...)		::Debut::Log.GetCoreLogger()->warn(__VA_ARGS__)
#define DBT_CORE_INFO(...)		::Debut::Log.GetCoreLogger()->info(__VA_ARGS__)
#define DBT_CORE_TRACE(...)		::Debut::Log.GetCoreLogger()->trace(__VA_ARGS__)

#define DBT_FATAL(...)		::Debut::Log.GetClientLogger()->critical(__VA_ARGS__)
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

		std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_CoreLogger; }

		std::shared_ptr<spdlog::logger>& GetClientLogger() { return m_ClientLogger; }

	private:
		std::shared_ptr<spdlog::logger> m_CoreLogger;
		std::shared_ptr<spdlog::logger> m_ClientLogger;
	};

	extern DBT_API _Log Log;
}


