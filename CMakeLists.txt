cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

OPTION(WINDOWS_BUILD "Enable to build for Windows" ON) # Enabled by default

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(DebutEngine VERSION 1.0.0)

# Include dependencies
add_subdirectory(Debutant Debutant/Build)