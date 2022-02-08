workspace "Debut"
	architecture "x86_64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Debut/vendor/glfw/include"

include "Debut/vendor/glfw"

project "Debut"
	location "Debut"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "Debut/dbtpch.h"
	pchsource "Debut/src/Debut/dbtpch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"GLFW",
		"opengl32"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS",
			"DBT_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
		}

	filter "configurations:Debug"
		staticruntime "off"
		runtime "Debug"
		defines {"DBT_DEBUG", "DBT_ASSERTS"}
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "DBT_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		runtime "Release"
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
		"%{prj.name}/src/**.cpp",
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