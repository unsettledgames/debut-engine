#pragma once

#ifdef DBT_PLATFORM_WINDOWS
	// Export names if you're building the dll
	#ifdef DBT_BUILD_DLL
		#define DBT_API	__declspec(dllexport)
	// Otherwise import them
	#else
		#define DBT_API	__declspec(dllimport)
	#endif
#else
	#error Debut only supports Windows atm
#endif