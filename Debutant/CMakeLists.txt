cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

OPTION(WINDOWS_BUILD "Enable to build for Windows" ON) # Enabled by default

project(Debut VERSION 1.0.0)

# Source files
file(GLOB_RECURSE Debutant_SRC
     	"src/*.h"
     	"src/*.cpp"
)

# Include dependencies
add_subdirectory(../Debut ../Debut/Build)

add_executable(Debutant ${Debutant_SRC})

set_property(TARGET Debutant PROPERTY CXX_STANDARD 17)
set_property(TARGET Debutant PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)

# Link dependencies 

target_link_libraries(Debutant
	PUBLIC Debut
)

IF(WINDOWS_BUILD)
    ADD_DEFINITIONS(-DDBT_PLATFORM_WINDOWS)
ENDIF(WINDOWS_BUILD)

# Include directories

target_include_directories(Debut 
	PUBLIC src
	PRIVATE ../Debut/src
)

# Keep the project structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Debutant_SRC})