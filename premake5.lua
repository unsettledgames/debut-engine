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
IncludeDir["GLFW"] = "Debut/vendor/glfw/include"
IncludeDir["Glad"] = "Debut/vendor/glad/include"
IncludeDir["imgui"] = "Debut/vendor/imgui"

include "Debut/vendor/glfw"
include "Debut/vendor/glad"
include "Debut/vendor/imgui"

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
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/loguru/loguru.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/loguru",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS",
			"DBT_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
		}

	filter "configurations:Debug"
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
	filter {"files:%{prj.name}/vendor/loguru/loguru.cpp"}
		flags {"NoPCH"}

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"Debut/vendor/loguru/loguru.cpp"
	}

	includedirs
	{
		"Debut/vendor/spdlog/include",
		"Debut/vendor/loguru",
		"Debut/src"
	}

	links
	{
		"Debut"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "DBT_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "DBT_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "DBT_DIST"
		optimize "On"