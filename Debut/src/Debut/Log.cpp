#include "Debut/dbtpch.h"
#include "Log.h"

namespace Debut
{
	_Log Log = _Log::_Log();

	void _Log::Init()
	{
		Log.CoreInfo("Initialized log");
	}
}