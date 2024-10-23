# Should really specify a range of tested versions.
cmake_minimum_required(VERSION 3.24)

cmake_policy(SET CMP0091 NEW)

# Set default build-type (AKA the configuration in other IDEs).
set(CMAKE_BUILD_TYPE_INIT Release)

# Setup Release and Debug build-types (only).
# No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
# Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (isMultiConfig)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
else()
	if (NOT DEFINED CMAKE_BUILD_TYPE)
		message(STATUS "Viewer -- Default to Release build.")
		set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Build Type" FORCE)
	endif()
	message(STATUS "Viewer -- Build type set to: ${CMAKE_BUILD_TYPE}")
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose Build Type")

	# Set the valid options for cmake-gui drop-down list. CMake tools for vscode does not (but should) respect this.
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# This include specifies the project and version.
include("Src/Version.cmake.h")
project(tacentview VERSION "${VIEWER_VERSION}" LANGUAGES C CXX)
message(STATUS "Viewer -- Name ${PROJECT_NAME} Version ${PROJECT_VERSION}")
message(STATUS "Viewer -- Major:${PROJECT_VERSION_MAJOR} Minor:${PROJECT_VERSION_MINOR} Revision:${PROJECT_VERSION_PATCH}")
message(STATUS "Viewer -- CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

# Find Git module.
find_package(Git)
if (Git_FOUND)
	message(STATUS "Viewer -- Git found: ${GIT_EXECUTABLE}")
endif()

# We want a better default for install prefix. It is bad form to be modifying
# system files from a cmake build of anything. Really quite surprised someone
# thinks the cmake defaults are good.
# message(STATUS "init=${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT}")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "..." FORCE)
endif()
message(STATUS "Viewer -- InstallPrefix ${CMAKE_INSTALL_PREFIX}")

include(FetchContent)

# Grab the Tacent library from github at configure time. Set desired options here first.
if (CMAKE_SYSTEM_NAME MATCHES Windows)
	option(TACENT_UTF16_API_CALLS "Build Tacent With UTF16 API Calls" On)
endif()

# See https://cmake.org/cmake/help/latest/guide/using-dependencies/index.html#fetchcontent-and-find-package-integration
# for how FIND_PACKAGE_ARGS allows FetchContent_MakeAvailable to try find_package() first.
FetchContent_Declare(
	tacent
	GIT_REPOSITORY https://github.com/bluescan/tacent.git
	# GIT_TAG 4941d5455d16768652aea0cd67381b71cb23ea64
	# GIT_TAG v0.8.18
	FIND_PACKAGE_ARGS NAMES tacent
)
FetchContent_MakeAvailable(tacent)

# After this call you can use variables tacent_POPULATED (a bool), tacent_BINARY_DIR, and tacent_SOURCE_DIR
FetchContent_GetProperties(tacent)
message(STATUS "Viewer -- tacent_POPULATED: ${tacent_POPULATED}")
message(STATUS "Viewer -- tacent_BINARY_DIR: ${tacent_BINARY_DIR}")
message(STATUS "Viewer -- tacent_SOURCE_DIR: ${tacent_SOURCE_DIR}")

# Files needed to create executable.
add_executable(
	${PROJECT_NAME}
	Src/ColourDialogs.cpp
	Src/ColourDialogs.h
	Src/Command.cpp
	Src/Command.h
	Src/CommandHelp.cpp
	Src/CommandHelp.h
	Src/CommandOps.cpp
	Src/CommandOps.h
	Src/Config.cpp
	Src/Config.h
	Src/ContactSheet.cpp
	Src/ContactSheet.h
	Src/Crop.cpp
	Src/Crop.h
	Src/Details.cpp
	Src/Details.h
	Src/Dialogs.cpp
	Src/Dialogs.h
	Src/FileDialog.cpp
	Src/FileDialog.h
	Src/GuiUtil.cpp
	Src/GuiUtil.h
	Src/Image.cpp
	Src/Image.h
	Src/ImportRaw.cpp
	Src/ImportRaw.h
	Src/InputBindings.cpp
	Src/InputBindings.h
	Src/MultiFrame.cpp
	Src/MultiFrame.h
	Src/OpenSaveDialogs.cpp
	Src/OpenSaveDialogs.h
	Src/Preferences.cpp
	Src/Preferences.h
	Src/Profile.cpp
	Src/Profile.h
	Src/Properties.cpp
	Src/Properties.h
	Src/Quantize.cpp
	Src/Quantize.h
	Src/Resize.cpp
	Src/Resize.h
	Src/Rotate.cpp
	Src/Rotate.h
	Src/TacentView.cpp
	Src/TacentView.h
	Src/ThumbnailView.cpp
	Src/ThumbnailView.h
	Src/Undo.cpp
	Src/Undo.h
	Src/Version.cmake.h
	Src/Version.cpp
	$<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/Windows/TacentView.rc>

	Contrib/imgui/imgui.cpp
	Contrib/imgui/imgui_demo.cpp
	Contrib/imgui/imgui_draw.cpp
	Contrib/imgui/imgui_tables.cpp
	Contrib/imgui/imgui_widgets.cpp
	Contrib/imgui/misc/cpp/imgui_stdlib.cpp
	Contrib/imgui/backends/imgui_impl_glfw.cpp
	Contrib/imgui/backends/imgui_impl_opengl2.cpp

	Contrib/glad/src/glad.c

	Contrib/clip/clip.cpp
	$<$<PLATFORM_ID:Windows>:Contrib/clip/clip_win.cpp>
	$<$<PLATFORM_ID:Windows>:Contrib/clip/clip_win_bmp.cpp>
	$<$<PLATFORM_ID:Windows>:Contrib/clip/clip_win_wic.cpp>
	$<$<PLATFORM_ID:Linux>:Contrib/clip/clip_x11.cpp>
	Contrib/clip/image.cpp
)

# Include directories needed to build.
target_include_directories(
	"${PROJECT_NAME}"
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Src
		${CMAKE_CURRENT_SOURCE_DIR}/Contrib/imgui
		${CMAKE_CURRENT_SOURCE_DIR}/Contrib/imgui/misc/cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Contrib/imgui/backends
		${CMAKE_CURRENT_SOURCE_DIR}/Contrib/glad/include
		${CMAKE_CURRENT_SOURCE_DIR}/Contrib/glfw/include
		${CMAKE_CURRENT_SOURCE_DIR}/Contrib/clip
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		ARCHITECTURE_X64
		GLFW_INCLUDE_NONE
		CLIP_ENABLE_IMAGE
		$<$<PLATFORM_ID:Linux>:HAVE_PNG_H>
		$<$<CONFIG:Debug>:CONFIG_DEBUG>
		$<$<CONFIG:Release>:CONFIG_RELEASE>
		$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_DEPRECATE>

		$<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
		$<$<PLATFORM_ID:Linux>:PLATFORM_LINUX>

		$<$<BOOL:${PACKAGE_PORTABLE}>:PACKAGE_PORTABLE>
		$<$<BOOL:${PACKAGE_DEV}>:PACKAGE_DEV>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${PACKAGE_SNAP}>>:PACKAGE_SNAP>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${PACKAGE_DEB}>>:PACKAGE_DEB>
		$<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${PACKAGE_NIX}>>:PACKAGE_NIX>

		# These shouldn't actually be necessary as there are no direct Windows API calls
		# in TacentView (they are abstracted away by the Tacent libraries). But just in case
		# anything in the viewer were to call an OS-level function, these enable the UTF-16
		# versions if the TACENT_UTF16_API_CALLS option is set.
		$<$<AND:$<PLATFORM_ID:Windows>,$<BOOL:${TACENT_UTF16_API_CALLS}>>:UNICODE>	# C++	UFF-16
		$<$<AND:$<PLATFORM_ID:Windows>,$<BOOL:${TACENT_UTF16_API_CALLS}>>:_UNICODE>	# C		UTF-16
		$<$<AND:$<PLATFORM_ID:Windows>,$<BOOL:${TACENT_UTF16_API_CALLS}>>:TACENT_UTF16_API_CALLS>
)

# Set compiler option flags based on specific compiler and configuration.
target_compile_options(
	${PROJECT_NAME}
	PRIVATE
		# MSVC compiler. Warning /utf-8 is needed for MSVC to support UTF-8 source files. Basically u8 string literals won't work without it.
		$<$<CXX_COMPILER_ID:MSVC>:/utf-8 /W2 /GS /Gy /Zc:wchar_t /Gm- /Zc:inline /fp:precise /WX- /Zc:forScope /Gd /FC>

		# Clang compiler.
		$<$<CXX_COMPILER_ID:Clang>:-Wno-switch>

		# GNU compiler.
		$<$<CXX_COMPILER_ID:GNU>:-Wno-unused-result>
		$<$<CXX_COMPILER_ID:GNU>:-Wno-multichar>

		# Clang and GNU.
		$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wno-format-security>

		$<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-O0>
		$<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/Od>
		$<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-O2>
		$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2>
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# This is how you set things like CMAKE_DEBUG_POSTFIX for a target.
if (MSVC)
	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES
		# DEBUG_POSTFIX "d"												# Add a 'd' before the extension for debug builds.
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"	# Use multithreaded or multithreaded-debug runtime on windows.
		WIN32_EXECUTABLE TRUE
		# More prop-value pairs here.
	)
endif()

# Dependencies.
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		Foundation Math System Image

		# $<$<PLATFORM_ID:Windows>:kernel32.lib>
		# $<$<PLATFORM_ID:Windows>:user32.lib>
		# $<$<PLATFORM_ID:Windows>:gdi32.lib>
		# $<$<PLATFORM_ID:Windows>:winspool.lib>
		# $<$<PLATFORM_ID:Windows>:shell32.lib>
		# $<$<PLATFORM_ID:Windows>:ole32.lib>
		# $<$<PLATFORM_ID:Windows>:oleaut32.lib>
		# $<$<PLATFORM_ID:Windows>:uuid.lib>
		# $<$<PLATFORM_ID:Windows>:comdlg32.lib>
		# $<$<PLATFORM_ID:Windows>:advapi32.lib>

		$<$<PLATFORM_ID:Windows>:shlwapi.lib>
		$<$<PLATFORM_ID:Windows>:Dbghelp.lib>
		$<$<PLATFORM_ID:Windows>:opengl32.lib>
		$<$<PLATFORM_ID:Windows>:uxtheme.lib>
		$<$<PLATFORM_ID:Windows>:dwmapi.lib>
		$<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/Contrib/glfw/glfw3.lib>

		$<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/Contrib/glfw/libglfw3.a>
		$<$<PLATFORM_ID:Linux>:m>
		$<$<PLATFORM_ID:Linux>:stdc++>
		$<$<PLATFORM_ID:Linux>:dl>
		$<$<PLATFORM_ID:Linux>:X11>
		$<$<PLATFORM_ID:Linux>:xcb>		# For clipboard support.
)

if (MSVC)
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
		$<IF:$<BOOL:${TACENT_UTF16_API_CALLS}>,/ENTRY:wmainCRTStartup,/ENTRY:mainCRTStartup>
		#"/SUBSYSTEM:CONSOLE"
		#"/SUBSYSTEM:WINDOWS"
	)
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		target_link_options(
			${PROJECT_NAME}
			PRIVATE
				"/NODEFAULTLIB:LIBCMT.lib"
	)
	endif()
endif()

# Install
set(VIEWER_INSTALL_DIR "${CMAKE_BINARY_DIR}/ViewerInstall")
message(STATUS "Viewer -- ${PROJECT_NAME} will be installed to ${VIEWER_INSTALL_DIR}")
set(VIEWER_PACKAGE_DIR "${VIEWER_INSTALL_DIR}/Package")
message(STATUS "Viewer -- ${PROJECT_NAME} will be packaged to ${VIEWER_PACKAGE_DIR}")

# Installation.

# if (CMAKE_SYSTEM_NAME MATCHES Linux)
#	include(GNUInstallDirs)
#	message(STATUS "Viewer -- CMAKE_INSTALL_BINDIR has value ${CMAKE_INSTALL_BINDIR}")
#	message(STATUS "Viewer -- CMAKE_INSTALL_DATADIR has value ${CMAKE_INSTALL_DATADIR}")
#	install(DIRECTORY Assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/tacentview/Assets)
# endif()

install(
	TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION "${VIEWER_INSTALL_DIR}"
)
install(DIRECTORY Assets/ DESTINATION "${VIEWER_INSTALL_DIR}/Assets")

# Packaging.
if (CMAKE_SYSTEM_NAME MATCHES Linux)
	if (PACKAGE_DEB)
		install(DIRECTORY Linux/ DESTINATION "${VIEWER_PACKAGE_DIR}")
		configure_file("Linux/create_deb.sh.in" "${VIEWER_PACKAGE_DIR}/create_deb.sh" @ONLY)
		install(CODE
			"
			execute_process(COMMAND \"chmod\" \"a+x\" \"ViewerInstall/Package/create_deb.sh\")
			execute_process(
				COMMAND \"ViewerInstall/Package/create_deb.sh\"
				RESULT_VARIABLE package_result
			)
			if (NOT package_result EQUAL \"0\")
				message(FATAL_ERROR \"Viewer -- Deb package creation failed.\")
			else()
				message(STATUS \"Viewer -- Deb package creation succeeded.\")
			endif()
			"
		)
	endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
	# The portable package for Windows generates a zip.
	if (PACKAGE_PORTABLE)
		install(DIRECTORY Windows/ DESTINATION "${VIEWER_PACKAGE_DIR}")
		configure_file("Windows/create_zip.ps1.in" "${VIEWER_PACKAGE_DIR}/create_zip.ps1" @ONLY)
		install(CODE
			"
			execute_process(
				COMMAND powershell -ExecutionPolicy Bypass -File \"ViewerInstall/Package/create_zip.ps1\"
				RESULT_VARIABLE package_result
			)
			if (NOT package_result EQUAL \"0\")
				message(FATAL_ERROR \"Viewer -- ZIP package creation failed.\")
			else()
				message(STATUS \"Viewer -- ZIP package creation succeeded.\")
			endif()
			"
		)
	endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES Linux)
	# The portable package for Linux generates a tgz.
	if (PACKAGE_PORTABLE)
		install(DIRECTORY Linux/ DESTINATION "${VIEWER_PACKAGE_DIR}")
		configure_file("Linux/create_tgz.sh.in" "${VIEWER_PACKAGE_DIR}/create_tgz.sh" @ONLY)
		install(CODE
			"
			execute_process(
				COMMAND bash \"ViewerInstall/Package/create_tgz.sh\"
				RESULT_VARIABLE package_result
			)
			if (NOT package_result EQUAL \"0\")
				message(FATAL_ERROR \"Viewer -- TGZ package creation failed.\")
			else()
				message(STATUS \"Viewer -- TGZ package creation succeeded.\")
			endif()
			"
		)
	endif()
endif()
