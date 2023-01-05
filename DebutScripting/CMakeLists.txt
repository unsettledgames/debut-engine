cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

project(DebutScripting CSharp)

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")

file(GLOB_RECURSE DBT_SCRIPT_SOURCES
     	"Source/*.cs"
)

add_library(DebutScripting SHARED ${DBT_SCRIPT_SOURCES})