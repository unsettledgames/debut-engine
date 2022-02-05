#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"

namespace Debut
{
	class DBT_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	
	private:
		// Engine logger
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		// Game logger
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define DBT_CORE_FATAL(...)		Debut::Log::GetCoreLogger()->fatal(__VA_ARGS__)
#define DBT_CORE_ERROR(...)		Debut::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DBT_CORE_WARN(...)		Debut::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DBT_CORE_INFO(...)		Debut::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DBT_CORE_TRACE(...)		Debut::Log::GetCoreLogger()->trace(__VA_ARGS__)

#define DBT_FATAL(...)		Debut::Log::GetCoreLogger()->fatal(__VA_ARGS__)
#define DBT_ERROR(...)		Debut::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DBT_WARN(...)		Debut::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DBT_INFO(...)		Debut::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DBT_TRACE(...)		Debut::Log::GetCoreLogger()->trace(__VA_ARGS__)
