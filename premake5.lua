workspace "Debut"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Debutant"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Debut/vendor/glfw/include"
IncludeDir["Glad"] = "Debut/vendor/glad/include"
IncludeDir["imgui"] = "Debut/vendor/imgui"
IncludeDir["glm"] = "Debut/vendor/glm"
IncludeDir["stb_image"] = "Debut/vendor/stb_image"
IncludeDir["spdlog"] = "Debut/vendor/spdlog/include"
IncludeDir["entt"] = "Debut/vendor/entt/include"
IncludeDir["yaml_cpp"] = "Debut/vendor/yaml-cpp/include"
IncludeDir["imguizmo"] = "Debut/vendor/imguizmo"
IncludeDir["box2d"] = "Debut/vendor/box2d/include"

include "Debut/vendor/glfw"
include "Debut/vendor/glad"
include "Debut/vendor/imgui"
include "Debut/vendor/yaml-cpp"
include "Debut/vendor/box2d"

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
		"%{prj.name}/vendor/glm/glm/**.h",
		"%{prj.name}/vendor/imguizmo/**.h",
		"%{prj.name}/vendor/imguizmo/**.cpp"
	}

	removefiles
	{
		"%{prj.name}/vendor/imguizmo/example/main.cpp",
		"%{prj.name}/vendor/imguizmo/vcpkg-example/main.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.imguizmo}",
		"%{IncludeDir.box2d}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"yaml-cpp",
		"Box2D"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS",
			"DBT_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"_CRT_SECURE_NO_WARNINGS",
			"YAML_CPP_STATIC_DEFINE"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines {"DBT_DEBUG"}
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
		warnings "Off"
	flags {"NoPCH"}
	

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
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.imguizmo}",
		"%{IncludeDir.yaml_cpp}",
		"Debut/src",
		"%{IncludeDir.box2d}"
	}

	links
	{
		"Debut"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"DBT_PLATFORM_WINDOWS",
			"YAML_CPP_STATIC_DEFINE"
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