#include "dbtpch.h"
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Debut
{
	// Engine logger
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	// Game logger
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		// Set the format of log messages (https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)
		// Timestamp, name of the logger, message, coloured depending on the severity
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("DEBUT");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_CoreLogger->set_level(spdlog::level::trace);

		DBT_CORE_INFO("Initialized logger\n");
	}
}