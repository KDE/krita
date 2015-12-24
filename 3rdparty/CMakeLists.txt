project (krita-and-all-its-deps)

#
# Build all dependencies for Krita and finally Krita itself.
# Parameters: EXTERNALS_DOWNLOAD_DIR place to download all packagfes
#             INSTALL_ROOT place to install everything to
#             WIN64_BUILD=TRUE/FALSE Whether it's a win64 build: false is default
# 
# Example usage: cmake ..\kritadeposx -DEXTERNALS_DOWNLOAD_DIR=/dev2/d -DINSTALL_ROOT=/dev2/i -DWIN64_BUILD=TRUE  -DBOOST_LIBRARYDIR=/dev2/i/lib   -G "Visual Studio 11 Win64"


cmake_minimum_required(VERSION 2.8.6)

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

if (MSCV)
    message("WIN64_BUILD" ${WIN64_BUILD})
endif()

if (WIN64_BUILD  STREQUAL "TRUE")
    SET(GLOBAL_PROFILE 
        -DCMAKE_MODULE_LINKER_FLAGS=/machine:x64 
        -DCMAKE_EXE_LINKER_FLAGS=/machine:x64 
        -DCMAKE_SHARED_LINKER_FLAGS=/machine:x64 
        -DCMAKE_STATIC_LINKER_FLAGS=/machine:x64 
        -DCMAKE_RC_COMPILER = "C:/Program Files (x86)/Windows Kits/8.0/bin/x64/rc.exe"  
    )
endif ()

message( STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message( STATUS "CMAKE_CL_64: ${CMAKE_CL_64}")

set(GLOBAL_BUILD_TYPE RelWithDebInfo)
set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DBUILD_TESTING=false)

if (MSVC)
        set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DCMAKE_EXE_LINKER_FLAGS=/PROFILE -DCMAKE_SHARED_LINKER_FLAGS=/PROFILE)
	set(PATCH_COMMAND myptch)
endif()

if (MSYS)
        set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DCMAKE_TOOLCHAIN_FILE=/data/mxe/mxe/usr/i686-w64-mingw32/share/cmake/mxe-conf.cmake 
                           -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                           -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                           -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include 
                           -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib
                           -DZLIB_ROOT=${CMAKE_PREFIX_PATH}
        )
        set(GLOBAL_AUTOMAKE_PROFILE  --host=i686-pc-mingw32 )

endif() 

if (APPLE)
    set(GLOBAL_PROFILE ${GLOBAL_PROFILE} -DCMAKE_MACOSX_RPATH=ON)
    set(PATCH_COMMAND patch)
endif ()

if (UNIX AND NOT APPLE)
	set(LINUX true)
    set(PATCH_COMMAND patch)
endif ()



# this list must be dependency-ordered
if (MSVC)
    add_subdirectory( ext_patch )
    add_subdirectory( ext_png2ico )
endif (MSVC)
add_subdirectory( ext_iconv )
add_subdirectory( ext_gettext )
add_subdirectory( ext_zlib )
add_subdirectory( ext_libxml2 )
add_subdirectory( ext_libxslt )
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
add_subdirectory( ext_vc )
add_subdirectory( ext_gsl )
add_subdirectory( ext_fftw3 )
add_subdirectory( ext_ocio )
add_subdirectory( ext_openjpeg )
if (MSVC)
    add_subdirectory( ext_pthreads )
endif (MSVC)
add_subdirectory( ext_freetype)
if (LINUX)
add_subdirectory( ext_fontconfig)
endif()
add_subdirectory( ext_poppler )
add_subdirectory( ext_libraw )
add_subdirectory( ext_frameworks )
