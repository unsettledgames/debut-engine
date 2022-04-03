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
IncludeDir["glm"] = "Debut/vendor/glm"
IncludeDir["stb_image"] = "Debut/vendor/stb_image"
IncludeDir["spdlog"] = "Debut/vendor/spdlog/include"

include "Debut/vendor/glfw"
include "Debut/vendor/glad"
include "Debut/vendor/imgui"

project "Debut"
	location "Debut"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "Debut/dbtpch.h"
	pchsource "Debut/src/Debut/dbtpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/glm/glm/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.h"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.spdlog}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS",
			"DBT_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines {"DBT_DEBUG", "DBT_PROFILE"}
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		defines {"DBT_RELEASE"}
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		defines "DBT_DIST"
		optimize "on"

	filter "files:Debut/vendor/**.cpp"
		flags {NoPCH}
	

project "Debutant"
	location "Debutant"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.spdlog}"
	}

	links
	{
		"Debut"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "DBT_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "DBT_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "DBT_DIST"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src/",
		"%{IncludeDir.spdlog}",
		"Debut/src",
		"%{IncludeDir.imgui}",
		"Debut/vendor/glm"
	}

	links
	{
		"Debut"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "DBT_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "DBT_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "DBT_DIST"
		optimize "on"