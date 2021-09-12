project (krita-and-all-its-deps)

#
# Build all dependencies for Krita and finally Krita itself.
# Parameters: EXTERNALS_DOWNLOAD_DIR place to download all packages
#             INSTALL_ROOT place to install everything to
#             MXE_TOOLCHAIN: the toolchain file to cross-compile using MXE
#
# Example usage: cmake ..\kritadeposx -DEXTERNALS_DOWNLOAD_DIR=/dev2/d -DINSTALL_ROOT=/dev2/i -DWIN64_BUILD=TRUE  -DBOOST_LIBRARYDIR=/dev2/i/lib   -G "Visual Studio 11 Win64"


if(APPLE)
        execute_process(COMMAND sysctl -n hw.optional.arm64 OUTPUT_VARIABLE apple_has_arm64_optional)
        if(apple_has_arm64_optional)
                message(STATUS "Building on macos arm")
                cmake_minimum_required(VERSION 3.19.3)
	else()
        cmake_minimum_required(VERSION 3.7.2)
	endif()
else(APPLE)
	cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR)
endif()

if(NOT SUBMAKE_JOBS)
	set(SUBMAKE_JOBS 1)
endif()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "Compiling in the source directory is not supported. Use for example 'mkdir build; cd build; cmake ..'.")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Tools must be obtained to work with:
include (ExternalProject)

LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/kde_macro")
include (KritaToNativePath)

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

if (WIN32)
        option(ENABLE_MSYS2_FIXES "Enable patches that are needed for MSYS2+Clang Krita build" OFF)
	option(QT_ENABLE_DEBUG_INFO "Build Qt with debug info included" OFF)
	option(QT_ENABLE_DYNAMIC_OPENGL "Build Qt with dynamic ANGLE support '-opengl dynamic -angle' (needs env var 'WindowsSdkDir' set to path of Windows 10 SDK)" ON)
    if (QT_ENABLE_DYNAMIC_OPENGL)
		if (DEFINED ENV{WindowsSdkDir})
			message(STATUS "WindowsSdkDir is set to '$ENV{WindowsSdkDir}'")
		else (DEFINED ENV{WindowsSdkDir})
			message(FATAL_ERROR "Environment variable 'WindowsSdkDir' not set! Please set it to path of Windows 10 SDK or disable QT_ENABLE_DYNAMIC_OPENGL")
		endif ()
    endif ()

    if(ENABLE_MSYS2_FIXES)
        find_package(ZLIB)
        find_package(Iconv)
        find_package(EXPAT)
        find_package(Gettext)
        find_package(OpenSSL)
    endif()

    if (NOT ZLIB_FOUND)
        set (ZLIB_DEP ext_zlib)
    endif()
    
    if (NOT Iconv_FOUND)
        set (ICONV_DEP ext_iconv)
    endif()
    
    if (NOT EXPAT_FOUND)
        set (EXPAT_DEP ext_expat)
    endif()
    
    if (NOT Gettext_FOUND)
        set (GETTEXT_DEP ext_gettext)
    endif()
    
    if (NOT OpenSSL_FOUND)
        set (OPENSSL_DEP ext_openssl)
    endif()
          
endif (WIN32)

if (WIN32)
    option(USE_QT_TABLET_WINDOWS "Do not use Krita's forked Wintab and Windows Ink support on Windows, but leave everything to Qt." ON)
endif ()

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
elseif (MSVC)
	set(SECURITY_C_FLAGS ${CMAKE_C_FLAGS})
	set(SECURITY_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# Increase the stack size to match MinGW's. Prevents crashes with GMic.
	set(SECURITY_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:4194304")
	set(SECURITY_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /STACK:4194304")
	set(SECURITY_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /STACK:4194304")
	option(USE_CONTROL_FLOW_GUARD "Enable Control Flow Guard hardening (MSVC)" ON)
	if (USE_CONTROL_FLOW_GUARD)
		set(SECURITY_C_FLAGS "${SECURITY_C_FLAGS} /guard:cf")
		set(SECURITY_CXX_FLAGS "${SECURITY_CXX_FLAGS} /guard:cf")
		set(SECURITY_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /GUARD:CF")
		set(SECURITY_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /GUARD:CF")
		set(SECURITY_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /GUARD:CF")
	endif (USE_CONTROL_FLOW_GUARD)
	set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
		-DCMAKE_C_FLAGS=${SECURITY_C_FLAGS}
		-DCMAKE_CXX_FLAGS=${SECURITY_CXX_FLAGS}
		-DCMAKE_EXE_LINKER_FLAGS=${SECURITY_EXE_LINKER_FLAGS}
		-DCMAKE_SHARED_LINKER_FLAGS=${SECURITY_SHARED_LINKER_FLAGS}
		-DCMAKE_MODULE_LINKER_FLAGS=${SECURITY_MODULE_LINKER_FLAGS}
	)
endif ()

if (DEFINED EP_PREFIX)
	set_directory_properties(PROPERTIES EP_PREFIX ${EP_PREFIX})
endif ()

if (WIN32)
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
    set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
                        -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
                        -DCMAKE_INCLUDE_PATH:PATH=${CMAKE_PREFIX_PATH}/include
                        -DCMAKE_LIBRARY_PATH:PATH=${CMAKE_PREFIX_PATH}/lib
                        -DCMAKE_MACOSX_RPATH=ON
                        -DKDE_SKIP_RPATH_SETTINGS=ON
                        -DBUILD_WITH_INSTALL_RPATH=ON
                        -DAPPLE_SUPPRESS_X11_WARNING=ON)
    set(PATCH_COMMAND patch)
endif ()

if (ANDROID)
  # HACK: copied from ECM, this makes sure that we don't build an executable (kf5s do that), but a DSO.
  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

  # Increase the stack size to match MinGW's. Prevents crashes with GMic.
  set(SECURITY_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=4194304")
  set(SECURITY_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,stack-size=4194304")
  set(SECURITY_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,stack-size=4194304")

  set (GLOBAL_PROFILE ${GLOBAL_PROFILE}
                     -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                     -DANDROID_PLATFORM=${ANDROID_PLATFORM}
                     -DANDROID_ABI=${ANDROID_ABI}
                     -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
                     -DCMAKE_CXX_LINK_EXECUTABLE=${CMAKE_CXX_LINK_EXECUTABLE}
                     -DCMAKE_EXE_LINKER_FLAGS=${SECURITY_EXE_LINKER_FLAGS}
                     -DCMAKE_SHARED_LINKER_FLAGS=${SECURITY_SHARED_LINKER_FLAGS}
                     -DCMAKE_MODULE_LINKER_FLAGS=${SECURITY_MODULE_LINKER_FLAGS}
                     )

  # see: https://github.com/android/ndk/issues/929
  unset (CMAKE_SYSROOT)
endif()

if (UNIX AND NOT APPLE)
	set(LINUX true)
    set(PATCH_COMMAND patch)
endif ()

function(TestCompileLinkPythonLibs OUTPUT_VARNAME)
	include(CheckCXXSourceCompiles)
	set(CMAKE_REQUIRED_INCLUDES ${Python_INCLUDE_DIRS})
	set(CMAKE_REQUIRED_LIBRARIES ${Python_LIBRARIES})
	if (MINGW)
		set(CMAKE_REQUIRED_DEFINITIONS -D_hypot=hypot)
	endif ()
	unset(${OUTPUT_VARNAME} CACHE)
	CHECK_CXX_SOURCE_COMPILES("
// https://bugs.python.org/issue22411
#if defined(_MSC_VER)
#  ifdef _DEBUG
#    undef _DEBUG
#  endif /* _DEBUG */
#endif /* _MSC_VER */
#include <Python.h>
int main(int argc, char *argv[]) {
	Py_InitializeEx(0);
}" ${OUTPUT_VARNAME})
endfunction()

function(DumpSitePackages PYTHONPATH)
    if (WIN32)
        krita_to_native_path("${${PYTHONPATH}}" _krita_pythonpath)
        string(TOLOWER "${_krita_pythonpath}" _krita_pythonpath)
    else()
        set(_krita_pythonpath ${${PYTHONPATH}})
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND}
        -E env PYTHONPATH=${_krita_pythonpath}
        ${Python_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_paths());"
        OUTPUT_VARIABLE __sysconfig)
    message(STATUS "Python's system directories: ${__sysconfig}")
    execute_process(COMMAND ${CMAKE_COMMAND}
        -E env PYTHONPATH=${_krita_pythonpath}
        ${Python_EXECUTABLE} -c "from setuptools.command import easy_install; print(easy_install.get_site_dirs())"
        OUTPUT_VARIABLE __setuptools)
    message(STATUS "Python's setuptools directories: ${__setuptools}")
endfunction()

if (WIN32)
    option(ENABLE_PYTHON_DEPS "Enable Python deps (sip, pyqt)" ON)
    if (ENABLE_PYTHON_DEPS)
        set(KRITA_PYTHONPATH "${INSTALL_ROOT}/lib/site-packages;$ENV{PYTHONPATH}")
        message(STATUS "Krita's PEP-0250 root: ${KRITA_PYTHONPATH}")
        if (ENABLE_PYTHON_2)
            message(FATAL_ERROR "Python 2.7 is not supported on Windows at the moment.")
        else(ENABLE_PYTHON_2)
            find_package(Python 3.8 EXACT COMPONENTS Development Interpreter)
        endif()
        if (Python_FOUND)
            message(STATUS "Python requirements met.")
            TestCompileLinkPythonLibs(CAN_USE_PYTHON_LIBS)
            DumpSitePackages(KRITA_PYTHONPATH)
            if (NOT CAN_USE_PYTHON_LIBS)
                message(FATAL_ERROR "Compiling with Python library failed, please check whether the architecture is correct!")
            endif ()
        else ()
            message(FATAL_ERROR "Python requirements not met. To disable Python deps, set ENABLE_PYTHON_DEPS to OFF.")
        endif ()
    endif ()
elseif(UNIX)
    if (APPLE)
        set(PYTHON_VERSION "3.9")
    else()
        set(PYTHON_VERSION "3.8")
    endif()

    set(KRITA_PYTHONPATH "${INSTALL_ROOT}/lib/python${PYTHON_VERSION}/site-packages")
    message(STATUS "Krita's PEP-0250 root: ${KRITA_PYTHONPATH}")
endif ()

# this list must be dependency-ordered
if (NOT WIN32 OR OPENSSL_DEP)
    add_subdirectory( ext_openssl )
endif()

if (ENABLE_PYTHON_DEPS OR NOT MINGW)
	add_subdirectory( ext_python )
endif ()
if (WIN32)
    add_subdirectory( ext_patch )
endif ()
add_subdirectory( ext_lzma )

if (NOT WIN32 OR ICONV_DEP)
    add_subdirectory( ext_iconv )
endif()

if (NOT WIN32 OR GETTEXT_DEP)
    add_subdirectory( ext_gettext )
endif()

if (NOT WIN32 OR ZLIB_DEP)
    add_subdirectory( ext_zlib )
endif()

add_subdirectory( ext_boost )
add_subdirectory( ext_jpeg )
add_subdirectory( ext_tiff )
add_subdirectory( ext_png )
if (WIN32)
    add_subdirectory( ext_icoutils )
endif ()
add_subdirectory( ext_eigen3 )

if (NOT WIN32 OR EXPAT_DEP)
    add_subdirectory( ext_expat ) # for exiv2
endif()

add_subdirectory( ext_exiv2 )
add_subdirectory( ext_lcms2 )
add_subdirectory( ext_openexr )
add_subdirectory( ext_vc )
add_subdirectory( ext_gsl )
add_subdirectory( ext_fftw3 )
add_subdirectory( ext_ocio )
add_subdirectory( ext_openjpeg )
add_subdirectory( ext_fontconfig)
add_subdirectory( ext_freetype)
add_subdirectory( ext_qt )
add_subdirectory( ext_poppler )
add_subdirectory( ext_libraw )
if (ANDROID)
  # gettext replacement
  add_subdirectory(ext_libintl-lite)
endif()
add_subdirectory( ext_frameworks )
if (ENABLE_PYTHON_DEPS OR NOT MINGW)
    add_subdirectory( ext_sip )
    add_subdirectory( ext_pyqt )
endif ()

if (WIN32)
    add_subdirectory( ext_drmingw )
#    add_subdirectory( ext_ffmpeg )
endif ()

if (NOT APPLE)
	add_subdirectory( ext_gmic )
endif ()

if(UNIX)
    add_subdirectory( ext_pkgconfig )
endif()

add_subdirectory( ext_heif )
add_subdirectory(ext_giflib)
add_subdirectory(ext_quazip)
add_subdirectory(ext_seexpr)
add_subdirectory(ext_json_c)
add_subdirectory(ext_mypaint)

if (UNIX AND NOT APPLE)
    add_subdirectory(ext_fcitx-qt)
endif()

add_subdirectory(ext_webp)
