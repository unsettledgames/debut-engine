#include "Debut/dbtpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Debut
{
	_Log Log = _Log::_Log();

	void _Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		
		m_AppLogger = spdlog::stdout_color_mt("APP");
		m_CoreLogger = spdlog::stdout_color_mt("DEBUT");

		Log.CoreInfo("Initialized log");
	}
}