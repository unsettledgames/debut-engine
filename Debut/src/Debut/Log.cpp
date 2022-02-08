#include "Debut/dbtpch.h"
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Debut
{
	_Log Log = _Log::_Log();

	void _Log::Init()
	{
		// Set the format of log messages (https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)
		// Timestamp, name of the logger, message, coloured depending on the severity
		spdlog::set_pattern("%^[%T] %n: %v%$");

		m_CoreLogger = spdlog::stdout_color_mt("DEBUT");
		m_CoreLogger->set_level(spdlog::level::trace);

		m_ClientLogger = spdlog::stdout_color_mt("APP");
		m_ClientLogger->set_level(spdlog::level::trace);

		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
		DBT_CORE_INFO("Initialized logger\n");
		DBT_INFO("Client\n");
	}

	void _Log::Shutdown()
	{
		spdlog::shutdown();
	}
}