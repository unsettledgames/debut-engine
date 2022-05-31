cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

project(Debut VERSION 1.0.0)

# Source files
file(GLOB Debut_SRC
     	"src/*.h"
     	"src/*.cpp"
	"vendor/entt/include"
	"vendor/stb_image/*.cpp"
	"vendor/stb_image/*.h"
)

# Include dependencies

add_subdirectory(vendor/box2d)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/yaml-cpp)

add_library(Debut ${Debut_SRC})

set_property(TARGET Debut PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)

# Link dependencies 

target_link_libraries(Debut 
	PUBLIC box2d
	PUBLIC glfw
	PUBLIC glm
	PUBLIC imgui
	PUBLIC spdlog
	PUBLIC yaml-cpp
)

# Define variables

target_compile_definitions(Debut PUBLIC
	GLFW_INCLUDE_NONE
	_CRT_SECURE_NO_WARNINGS
	YAML_CPP_STATIC_DEFINE
)

# Include directories

target_include_directories(Debut 
	PUBLIC src
	PRIVATE vendor/box2d/include
	PRIVATE vendor/glfw/include
	PUBLIC vendor/glm
)