workspace "Debut"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Debut/vendor/GLFW/include"

include "Debut/vendor/GLFW"

project "Debut"
	location "Debut"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "dbtpch.h"
	pchsource "Debut/src/Debut/dbtpch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.GLFW}"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include"
	}

	links
	{
		"GLFW",
		"opengl32"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS",
			"DBT_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "DBT_DEBUG"
		symbols "On"

	filter "configurations:Debug"
		defines "DBT_RELEASE"
		optimize "On"

	filter "configurations:Debug"
		defines "DBT_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Debut/vendor/spdlog/include",
		"Debut/src"
	}

	links
	{
		"Debut"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "DBT_DEBUG"
		symbols "On"

	filter "configurations:Debug"
		defines "DBT_RELEASE"
		optimize "On"

	filter "configurations:Debug"
		defines "DBT_DIST"
		optimize "On"