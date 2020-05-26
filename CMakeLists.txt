# Should really specify a range of tested versions.
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE )
project("Tacent Viewer" VERSION 1.0 LANGUAGES C CXX)

# Include any CMake modules we need.
include(FetchContent)

find_package(Git)
if (Git_FOUND)
	message("Git found: ${GIT_EXECUTABLE}")
endif()

# Grab the Tacent library from github. At configure time, not build time.
FetchContent_Declare(
	tacent
	GIT_REPOSITORY https://github.com/bluescan/tacent.git
	GIT_TAG        v0.7.1
)
FetchContent_MakeAvailable(tacent)
