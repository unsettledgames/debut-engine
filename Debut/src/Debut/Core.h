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

#ifdef DBT_ASSERTS
	#define DBT_ASSERT(x, ...) { if(!(x)) { DBT_ERROR("Assertion failed. {0}", __VA_ARGS__); __debugbreak(); }}
	#define DBT_CORE_ASSERT(x, ...) { if(!(x)) { DBT_CORE_ERROR("Assertion failed. {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define DBT_ASSERT(x, ...)
	#define DBT_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT(x) std::bind(&Application::x, this, std::placeholders::_1)