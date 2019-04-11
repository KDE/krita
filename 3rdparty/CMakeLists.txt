project (krita-and-all-its-deps)

#
# Build all dependencies for Krita and finally Krita itself.
# Parameters: EXTERNALS_DOWNLOAD_DIR place to download all packages
#             INSTALL_ROOT place to install everything to
#             MXE_TOOLCHAIN: the toolchain file to cross-compile using MXE
#
# Example usage: cmake ..\kritadeposx -DEXTERNALS_DOWNLOAD_DIR=/dev2/d -DINSTALL_ROOT=/dev2/i -DWIN64_BUILD=TRUE  -DBOOST_LIBRARYDIR=/dev2/i/lib   -G "Visual Studio 11 Win64"


cmake_minimum_required(VERSION 2.8.6)

if(NOT SUBMAKE_JOBS)
	set(SUBMAKE_JOBS 1)
endif()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "Compiling in the source directory is not supported. Use for example 'mkdir build; cd build; cmake ..'.")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Tools must be obtained to work with:
include (ExternalProject)

# allow specification of a directory with pre-downloaded
# requirements
if(NOT IS_DIRECTORY ${EXTERNALS_DOWNLOAD_DIR})
    message(FATAL_ERROR "No externals download dir set. Use -DEXTERNALS_DOWNLOAD_DIR")
endif()

if(NOT IS_DIRECTORY ${INSTALL_ROOT})
    message(FATAL_ERROR "No install dir set. Use -DINSTALL_ROOT")
endif()

set(TOP_INST_DIR ${INSTALL_ROOT})
set(EXTPREFIX "${TOP_INST_DIR}")
set(CMAKE_PREFIX_PATH "${EXTPREFIX}")

if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
    SET(GLOBAL_PROFILE
        -DCMAKE_MODULE_LINKER_FLAGS=/machine:x64
        -DCMAKE_EXE_LINKER_FLAGS=/machine:x64
        -DCMAKE_SHARED_LINKER_FLAGS=/machine:x64
        -DCMAKE_STATIC_LINKER_FLAGS=/machine:x64
    )
endif ()

message( STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message( STATUS "CMAKE_CL_64: ${CMAKE_CL_64}")

set(GLOBAL_BUILD_TYPE RelWithDebInfo)
set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DBUILD_TESTING=false)

if (MINGW)
	option(QT_ENABLE_DEBUG_INFO "Build Qt with debug info included" OFF)
	option(QT_ENABLE_DYNAMIC_OPENGL "Build Qt with dynamic ANGLE support '-opengl dynamic -angle' (needs env var 'WindowsSdkDir' set to path of Windows 10 SDK)" ON)
    if (QT_ENABLE_DYNAMIC_OPENGL)
		if (DEFINED ENV{WindowsSdkDir})
			message(STATUS "WindowsSdkDir is set to '$ENV{WindowsSdkDir}'")
		else (DEFINED ENV{WindowsSdkDir})
			message(FATAL_ERROR "Environment variable 'WindowsSdkDir' not set! Please set it to path of Windows 10 SDK or disable QT_ENABLE_DYNAMIC_OPENGL")
		endif ()
	endif ()
endif (MINGW)

set(SECURITY_EXE_LINKER_FLAGS "")
set(SECURITY_SHARED_LINKER_FLAGS "")
set(SECURITY_MODULE_LINKER_FLAGS "")
if (MINGW)
	option(USE_MINGW_HARDENING_LINKER "Enable DEP (NX), ASLR and high-entropy ASLR linker flags (mingw-w64)" ON)
	if (USE_MINGW_HARDENING_LINKER)
		set(SECURITY_EXE_LINKER_FLAGS "-Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
		set(SECURITY_SHARED_LINKER_FLAGS "-Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
		set(SECURITY_MODULE_LINKER_FLAGS "-Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
		if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
			# Enable high-entropy ASLR for 64-bit
			# The image base has to be >4GB for HEASLR to be enabled.
			# The values used here are kind of arbitrary.
			set(SECURITY_EXE_LINKER_FLAGS "${SECURITY_EXE_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x140000000")
			set(SECURITY_SHARED_LINKER_FLAGS "${SECURITY_SHARED_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x180000000")
			set(SECURITY_MODULE_LINKER_FLAGS "${SECURITY_MODULE_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x180000000")
			set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
				-DCMAKE_EXE_LINKER_FLAGS=${SECURITY_EXE_LINKER_FLAGS}
				-DCMAKE_SHARED_LINKER_FLAGS=${SECURITY_SHARED_LINKER_FLAGS}
				-DCMAKE_MODULE_LINKER_FLAGS=${SECURITY_MODULE_LINKER_FLAGS}
			)
		endif ()
	else ()
		message(WARNING "Linker Security Flags not enabled!")
	endif ()
endif ()

if (DEFINED EP_PREFIX)
	set_directory_properties(PROPERTIES EP_PREFIX ${EP_PREFIX})
endif ()

if (MSVC)
    message(FATAL_ERROR "Krita cannot be built with MSVC. See the README.md file!")
endif()

if (MINGW)
   set(PATCH_COMMAND myptch)
endif()

if (MSYS)
	set(PATCH_COMMAND patch)
    set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
                           -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN}
                           -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                           -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                           -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                           -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib
                           -DZLIB_ROOT=${CMAKE_PREFIX_PATH}
    )
    set(GLOBAL_AUTOMAKE_PROFILE  --host=i686-pc-mingw32 )
endif()

if (APPLE)
    set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DCMAKE_MACOSX_RPATH=ON -DKDE_SKIP_RPATH_SETTINGS=ON -DBUILD_WITH_INSTALL_RPATH=ON -DAPPLE_SUPPRESS_X11_WARNING=ON)
    set(PATCH_COMMAND patch)
endif ()

if (UNIX AND NOT APPLE)
	set(LINUX true)
    set(PATCH_COMMAND patch)
endif ()

function(TestCompileLinkPythonLibs OUTPUT_VARNAME)
	include(CheckCXXSourceCompiles)
	set(CMAKE_REQUIRED_INCLUDES ${PYTHON_INCLUDE_PATH})
	set(CMAKE_REQUIRED_LIBRARIES ${PYTHON_LIBRARIES})
	if (MINGW)
		set(CMAKE_REQUIRED_DEFINITIONS -D_hypot=hypot)
	endif ()
	unset(${OUTPUT_VARNAME} CACHE)
	CHECK_CXX_SOURCE_COMPILES("
#include <Python.h>
int main(int argc, char *argv[]) {
	Py_InitializeEx(0);
}" ${OUTPUT_VARNAME})
endfunction()

if (MINGW)
	option(ENABLE_PYTHON_DEPS "Enable Python deps (sip, pyqt)" ON)
	if (ENABLE_PYTHON_DEPS)
        if (ENABLE_PYTHON_2)
            message(FATAL_ERROR "Python 2.7 is not supported on Windows at the moment.")
        else(ENABLE_PYTHON_2)
            find_package(PythonInterp 3.6 EXACT)
            find_package(PythonLibs 3.6 EXACT)
        endif()
		if (PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
			message(STATUS "Python requirements met.")
			TestCompileLinkPythonLibs(CAN_USE_PYTHON_LIBS)
			if (NOT CAN_USE_PYTHON_LIBS)
				message(FATAL_ERROR "Compiling with Python library failed, please check whether the architecture is correct!")
			endif ()
		else (PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
			message(FATAL_ERROR "Python requirements not met. To disable Python deps, set ENABLE_PYTHON_DEPS to OFF.")
		endif ()
	endif ()
endif ()

# this list must be dependency-ordered
if (ENABLE_PYTHON_DEPS OR NOT MINGW)
	add_subdirectory( ext_python )
endif ()
if (MINGW)
    add_subdirectory( ext_patch )
    add_subdirectory( ext_png2ico )
endif ()
add_subdirectory( ext_lzma )
add_subdirectory( ext_iconv )
add_subdirectory( ext_gettext )
add_subdirectory( ext_zlib )
add_subdirectory( ext_boost )
add_subdirectory( ext_jpeg )
add_subdirectory( ext_tiff )
add_subdirectory( ext_png )
add_subdirectory( ext_eigen3 )
add_subdirectory( ext_expat ) # for exiv2
add_subdirectory( ext_exiv2 )
add_subdirectory( ext_ilmbase )
add_subdirectory( ext_lcms2 )
add_subdirectory( ext_openexr )
add_subdirectory( ext_openssl )
add_subdirectory( ext_vc )
add_subdirectory( ext_gsl )
add_subdirectory( ext_fftw3 )
add_subdirectory( ext_ocio )
add_subdirectory( ext_fontconfig)
add_subdirectory( ext_freetype)
add_subdirectory( ext_qt )
add_subdirectory( ext_poppler )
add_subdirectory( ext_libraw )
add_subdirectory( ext_frameworks )
if (ENABLE_PYTHON_DEPS OR NOT MINGW)
	add_subdirectory( ext_sip )
	add_subdirectory( ext_pyqt )
endif ()

if (MINGW)
    add_subdirectory( ext_drmingw )
#    add_subdirectory( ext_ffmpeg )
endif ()

if (NOT APPLE)
	add_subdirectory( ext_gmic )
endif ()

add_subdirectory(ext_giflib)
add_subdirectory(ext_quazip)
