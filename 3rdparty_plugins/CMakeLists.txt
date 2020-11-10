project (krita-external-plugins)

message( STATUS "Preparing Krita external plugins build...")

#
# Build external Krita plugins.
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
message( STATUS "EXTERNALS_DOWNLOAD_DIR: ${EXTERNALS_DOWNLOAD_DIR}")
if(NOT IS_DIRECTORY ${EXTERNALS_DOWNLOAD_DIR})
    message(FATAL_ERROR "No externals download dir set. Use -DEXTERNALS_DOWNLOAD_DIR")
endif()

message( STATUS "INSTALL_ROOT: ${INSTALL_ROOT}")
if(NOT IS_DIRECTORY ${INSTALL_ROOT})
    message(FATAL_ERROR "No install dir set. Use -DINSTALL_ROOT")
endif()

set(TOP_INST_DIR ${INSTALL_ROOT})
set(EXTPREFIX "${TOP_INST_DIR}")

message( STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message( STATUS "CMAKE_CL_64: ${CMAKE_CL_64}")
message( STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

set(GLOBAL_BUILD_TYPE RelWithDebInfo)
set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DBUILD_TESTING=false)

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

if (MSVC)
    message(FATAL_ERROR "Krita cannot be built with MSVC. See the README.md file!")
endif()

if (MINGW)
   set(PATCH_COMMAND myptch)
endif()

if (APPLE)
    set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
                        -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
                        -DCMAKE_MACOSX_RPATH=ON
                        -DKDE_SKIP_RPATH_SETTINGS=ON
                        -DBUILD_WITH_INSTALL_RPATH=ON
                        -DAPPLE_SUPPRESS_X11_WARNING=ON)
    set(PATCH_COMMAND patch)
endif ()

if (ANDROID)
  set (GLOBAL_PROFILE ${GLOBAL_PROFILE}
                     -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                     -DANDROID_PLATFORM=${ANDROID_PLATFORM}
                     -DANDROID_ABI=${ANDROID_ABI}
                     -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH})

  # see: https://github.com/android/ndk/issues/929
  unset (CMAKE_SYSROOT)
endif()

if (UNIX AND NOT APPLE)
	set(LINUX true)
    set(PATCH_COMMAND patch)
endif ()

# this list must be dependency-ordered
add_subdirectory( ext_gmic )
