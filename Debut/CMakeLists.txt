cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

OPTION(WINDOWS_BUILD "Enable to build for Windows" ON) # Enabled by default
set(CMAKE_CXX_STANDARD 17)

project(Debut VERSION 1.0.0)

# Source files
file(GLOB_RECURSE Debut_SRC
     	"src/*.h"
     	"src/*.cpp"
)

file (GLOB Debut_EXT
	"vendor/entt/include/entt.hpp"
	"vendor/stb_image/*.cpp"
	"vendor/stb_image/*.h"
	"vendor/other/earcut.hpp"
	"vendor/other/earcut.cpp"
	"vendor/imguizmo/*.cpp"
	"vendor/imguizmo/*.h"
)

# Configure dependencies
# Box2D
set(BOX2D_BUILD_TESTBED OFF CACHE INTERNAL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE INTERNAL "" FORCE)

# YAML-cpp
set(YAML_BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE INTERNAL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)

# Assimp
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
configure_file("${PROJECT_SOURCE_DIR}/vendor/assimp/include/assimp/config.h.in" "${PROJECT_SOURCE_DIR}/vendor/assimp/include/assimp/config.h")
configure_file("${PROJECT_SOURCE_DIR}/vendor/assimp/contrib/zlib/zconf.h.cmakein" "vendor/assimp/contrib/zlib/zconf.h")

# Jolt Physics
set(TARGET_UNIT_TESTS OFF CACHE INTERNAL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE INTERNAL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE INTERNAL "" FORCE)
set(TARGET_SAMPLES OFF CACHE INTERNAL "" FORCE)
set(TARGET_VIEWER OFF CACHE INTERNAL "" FORCE)

add_library(Debut STATIC ${Debut_SRC} ${Debut_EXT})

set_property(TARGET Debut PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)
set_property(TARGET Debut PROPERTY CXX_STANDARD 17)


# Define variables
target_compile_definitions(Debut PUBLIC
	GLFW_INCLUDE_NONE
	_CRT_SECURE_NO_WARNINGS
	YAML_CPP_STATIC_DEFINE
	NOMINMAX
	JPH_DISABLE_CUSTOM_ALLOCATOR
)

target_compile_definitions(Debut PUBLIC
	$<$<CONFIG:Debug>:DBT_DEBUG=1>
	$<$<CONFIG:Release>:DBT_RELEASE=1>
)

# Include dependencies
add_subdirectory(vendor/glad)
add_subdirectory(vendor/box2d)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/imguizmo)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/lz4/build/cmake)
add_subdirectory(vendor/jolt-physics/Build)

target_compile_definitions(Jolt PRIVATE JPH_DISABLE_CUSTOM_ALLOCATOR=1)


# Link dependencies 

target_link_libraries(Debut 
	PUBLIC box2d
	PUBLIC glad
	PUBLIC glfw
	PUBLIC glm
	PUBLIC imgui
	PUBLIC spdlog
	PUBLIC yaml-cpp
	PUBLIC Dwmapi.dll
	PRIVATE Bcrypt.lib
	PRIVATE Version.lib
	PRIVATE Ws2_32.lib
	PRIVATE Winmm.lib
	PUBLIC assimp
	PUBLIC Jolt
	PRIVATE lz4_static
	
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/eglib.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/libgcmonosgen.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/libmini-sgen.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/libmonoruntime-sgen.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/libmono-static-sgen.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/libmonoutils.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/mono-2.0-sgen.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/lib/MonoPosixHelper.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/bin/mono-2.0-sgen.lib
	PRIVATE ${CMAKE_SOURCE_DIR}/Debut/vendor/mono/bin/MonoPosixHelper.lib
)

IF(WINDOWS_BUILD)
    ADD_DEFINITIONS(-DDBT_PLATFORM_WINDOWS)
ENDIF(WINDOWS_BUILD)

# Include directories

target_include_directories(Debut 
	PUBLIC src
	PRIVATE vendor/glad/include
	PRIVATE vendor/box2d/include
	PRIVATE vendor/glfw/include
	PUBLIC vendor/imgui
	PUBLIC vendor/imguizmo
	PUBLIC vendor/stb_image
	PUBLIC vendor/glm
	PUBLIC vendor/entt/include
	PUBLIC vendor/assimp/include
	PUBLIC vendor/jolt-physics/Jolt/
	PRIVATE vendor/lz4/lib
	PRIVATE vendor/other
	PRIVATE vendor/mono/include
)

# Keep the project structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Debut_SRC})