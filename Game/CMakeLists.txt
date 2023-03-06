cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0111 OLD)

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
project(TestGame CSharp)

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")

file(GLOB_RECURSE DBT_SCRIPT_SOURCES
     	"Source/*.cs"
)

add_library(TestGame SHARED ${DBT_SCRIPT_SOURCES})
add_library(DebutScripting SHARED IMPORTED)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DOTNET_REFERENCES "DebutScripting"
    VS_DOTNET_REFERENCE_DebutScripting ${CMAKE_SOURCE_DIR}/../Resources/Mono/Debut/${CMAKE_BUILD_TYPE}/DebutScripting.dll)