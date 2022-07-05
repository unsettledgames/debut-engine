#pragma once

#include <memory>

#ifdef DBT_PLATFORM_WINDOWS
	#include <fcntl.h>
	#include <io.h>
	#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)

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
	#define SET_BINARY_MODE(file)
	#error Debut only supports Windows atm
#endif

#ifdef DBT_DEBUG
	#define DBT_ASSERTS 1
	#define DBT_PROFILE 1
#endif 

#define BIT(x) (1 << x)

#define DBT_BIND(x) [this](auto&&... args) -> decltype(auto) {return this->x(std::forward<decltype(args)>(args)...);}


namespace Debut
{

	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ...args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateScope(Args&& ...args)
	{
		return std::unique_ptr<T>(std::forward<Args>(args)...);
	}

	/**
		using ShaderRef = std::shared_ptr<Shader>;
	*/
}