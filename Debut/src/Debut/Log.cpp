#include "Debut/dbtpch.h"
#include "Log.h"

namespace Debut
{
	_Log Log = _Log::_Log();

	void _Log::Init()
	{
		// Set the format of log messages (https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)
		// Timestamp, name of the logger, message, coloured depending on the severity

		m_CoreLogger = el::Loggers::getLogger("default");
		m_ClientLogger = el::Loggers::getLogger("app");

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
	}
}