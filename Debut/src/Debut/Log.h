#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define DBT_CORE_FATAL(...)		::Debut::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define DBT_CORE_ERROR(...)		::Debut::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DBT_CORE_WARN(...)		::Debut::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DBT_CORE_INFO(...)		::Debut::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DBT_CORE_TRACE(...)		::Debut::Log::GetCoreLogger()->trace(__VA_ARGS__)

#define DBT_FATAL(...)		::Debut::Log::GetClientLogger()->critical(__VA_ARGS__)
#define DBT_ERROR(...)		::Debut::Log::GetClientLogger()->error(__VA_ARGS__)
#define DBT_WARN(...)		::Debut::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DBT_INFO(...)		::Debut::Log::GetClientLogger()->info(__VA_ARGS__)
#define DBT_TRACE(...)		::Debut::Log::GetClientLogger()->trace(__VA_ARGS__)

namespace Debut
{
	class DBT_API Log
	{
	public:
		static void Init();

		static void Shutdown()
		{
			spdlog::shutdown();
		}

		static std::shared_ptr<spdlog::logger> GetCoreLogger() 
		{ 
			static std::shared_ptr<spdlog::logger> s_CoreLogger = spdlog::stdout_color_mt("DEBUT");
			return s_CoreLogger; 
		}

		static std::shared_ptr<spdlog::logger> GetClientLogger() 
		{
			static std::shared_ptr<spdlog::logger> s_ClientLogger = spdlog::stdout_color_mt("APP");
			return s_ClientLogger;
		}

		static void CoreInfo()
		{
			DBT_CORE_INFO("YOOO\n");
			DBT_INFO("YOOO\n");
		}
	};
}


