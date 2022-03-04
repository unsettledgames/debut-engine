#pragma once
#ifdef DBT_PLATFORM_WINDOWS
	#ifdef DBT_DYNAMIC_LINK
		// Export names if you're building the dll
		#ifdef DBT_BUILD_DLL
			#define DBT_API	__declspec(dllexport)
		// Otherwise import them
		#else
			#define DBT_API	__declspec(dllimport)
		#endif
	#else
		#define DBT_API
	#endif
#else
	#error Debut only supports Windows atm
#endif

#ifdef DBT_DEBUG
	#define DBT_ASSERTS
#endif 

#ifdef DBT_ASSERTS
	#define DBT_ASSERT(x, ...) { if(!(x)) { DBT_ERROR("Assertion failed. %s", __VA_ARGS__); __debugbreak(); }}
	#define DBT_CORE_ASSERT(x, ...) { if(!(x)) { DBT_CORE_ERROR("Assertion failed. %s", __VA_ARGS__); __debugbreak(); }}
#else
	#define DBT_ASSERT(x, ...)
	#define DBT_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define DBT_BIND(x) std::bind(&x, this, std::placeholders::_1)