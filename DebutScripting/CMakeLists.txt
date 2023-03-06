cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

project(DebutScripting CSharp)

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Resources/Mono/Debut)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Resources/Mono/Debut)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Resources/Mono/Debut)

file(GLOB_RECURSE DBT_SCRIPT_SOURCES
     	"Core/*.cs"
)

add_library(DebutScripting SHARED ${DBT_SCRIPT_SOURCES})