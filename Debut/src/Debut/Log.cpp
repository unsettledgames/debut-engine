#include "Debut/dbtpch.h"
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Debut
{
	void Log::Init()
	{
		// Set the format of log messages (https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)
		// Timestamp, name of the logger, message, coloured depending on the severity
		spdlog::set_pattern("%^[%T] %n: %v%$");

		Log::GetCoreLogger()->set_level(spdlog::level::trace);
		Log::GetClientLogger()->set_level(spdlog::level::trace);

		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
	}
}