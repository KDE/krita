project (krita-and-all-its-deps)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 OLD)
endif()

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

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

#
# If you add a new dependency into 3rdparty folder, do **not** overide
# BUILD_COMMAND and INSTALL_COMMAND with their '-j${SUBMAKE_JOBS}' equivalents,
# unless you need a really custom command for this dep. CMake will pass the
# correct threading option to make/ninja automatically. The variable below is
# Used **only** by custom builds, like sip and boost.
#

if (NOT SUBMAKE_JOBS)
    include(ProcessorCount)
    ProcessorCount(NUM_CORES)
    if  (NOT NUM_CORES EQUAL 0)
        if (NUM_CORES GREATER 2)
            # be nice...
            MATH( EXPR NUM_CORES "${NUM_CORES} - 2" )
        endif()
        set(SUBMAKE_JOBS ${NUM_CORES})
    else()
        set(SUBMAKE_JOBS 1)
    endif()
endif()

MESSAGE("SUBMAKE_JOBS: " ${SUBMAKE_JOBS})

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "Compiling in the source directory is not supported. Use for example 'mkdir build; cd build; cmake ..'.")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Tools must be obtained to work with:
include (ExternalProject)

LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/kde_macro")
LIST (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
include (KritaToNativePath)
include (KritaExternalProject)

# allow specification of a directory with pre-downloaded
# requirements
if(DEFINED ENV{EXTERNALS_DOWNLOAD_DIR})
    set(EXTERNALS_DOWNLOAD_DIR $ENV{EXTERNALS_DOWNLOAD_DIR})
endif()

if(NOT IS_DIRECTORY ${EXTERNALS_DOWNLOAD_DIR})
    message(FATAL_ERROR "No externals download dir set. Use -DEXTERNALS_DOWNLOAD_DIR")
else()
    file(TO_CMAKE_PATH "${EXTERNALS_DOWNLOAD_DIR}" EXTERNALS_DOWNLOAD_DIR)
endif()

if(NOT IS_DIRECTORY ${INSTALL_ROOT})
    message(FATAL_ERROR "No install dir set. Use -DINSTALL_ROOT")
else()
    file(TO_CMAKE_PATH "${INSTALL_ROOT}" INSTALL_ROOT)
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

if (UNIX AND NOT APPLE)
        set(LINUX true)
    set(PATCH_COMMAND patch)
endif ()

if (WIN32 OR LINUX)
option(QT_ENABLE_DEBUG_INFO "Build Qt with full debug info included" OFF)
option(QT_ENABLE_ASAN "Build Qt with ASAN" OFF)
endif()

include(FeatureSummary)

if (WIN32)
    set(_is_perl_required)
    set(_perl_type OPTIONAL)
else()
    set(_is_perl_required REQUIRED)
    set(_perl_type REQUIRED)
endif()
find_package(Perl ${_is_perl_required})
set_package_properties(Perl PROPERTIES
    URL "https://www.perl.org/"
    DESCRIPTION "Perl is a highly capable, feature-rich programming language with over 30 years of development."
    TYPE ${_perl_type}
    PURPOSE "Needed for building Qt from source."
)

if (WIN32)
    option(ENABLE_MSYS2_FIXES "Enable patches that are needed for MSYS2+Clang Krita build" OFF)

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
elseif (APPLE OR ANDROID)
    set (ZLIB_DEP ext_zlib)
    set (ICONV_DEP ext_iconv)
    set (GETTEXT_DEP ext_gettext)
else()
    # In Linux we require system versions for compatibility reasons.
    find_package(ZLIB REQUIRED)
    set_package_properties(ZLIB PROPERTIES
        DESCRIPTION "Compression library"
        URL "https://www.zlib.net/"
        TYPE REQUIRED
        PURPOSE "Needed for building libpng from source.")
    find_package(Iconv REQUIRED)
    set_package_properties(Iconv PROPERTIES
        URL "https://www.gnu.org/software/libiconv/"
        DESCRIPTION "Conversion library"
        TYPE REQUIRED
        PURPOSE "Needed for building exiv2 and gettext from source."
    )
endif (WIN32)

set(SECURITY_EXE_LINKER_FLAGS "")
set(SECURITY_SHARED_LINKER_FLAGS "")
set(SECURITY_MODULE_LINKER_FLAGS "")
if (MINGW)
	option(USE_MINGW_HARDENING_LINKER "Enable DEP (NX), ASLR and high-entropy ASLR linker flags (mingw-w64)" ON)
	if (USE_MINGW_HARDENING_LINKER)
		set(SECURITY_EXE_LINKER_FLAGS "-Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
		set(SECURITY_SHARED_LINKER_FLAGS "-Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
		set(SECURITY_MODULE_LINKER_FLAGS "-Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
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
	else ()
		message(WARNING "Linker Security Flags not enabled!")
	endif ()

    # Generate reduced debug info
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g1")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g1")

    # Clang does not generate DWARF aranges data by default, which makes
    # DrMingw not able to parse the DWARF debug symbols. Add -gdwarf-aranges
    # explicitly.
    # See: https://github.com/jrfonseca/drmingw/issues/42#issuecomment-516614561
    #
    # `-fdebug-info-for-profiling` is needed for proper C++ function signatures
    # when using Clang with `-g1`.
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -gdwarf-aranges -fdebug-info-for-profiling")
    endif ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -gdwarf-aranges -fdebug-info-for-profiling")
    endif ()

    set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
        -DCMAKE_C_FLAGS_RELWITHDEBINFO=${CMAKE_C_FLAGS_RELWITHDEBINFO}
        -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
    )
elseif (MSVC)
    set(SECURITY_C_FLAGS "")
    set(SECURITY_CXX_FLAGS "")
	# Increase the stack size to match MinGW's. Prevents crashes with GMic.
    set(SECURITY_EXE_LINKER_FLAGS "/STACK:4194304")
    set(SECURITY_SHARED_LINKER_FLAGS "/STACK:4194304")
    set(SECURITY_MODULE_LINKER_FLAGS "/STACK:4194304")
	option(USE_CONTROL_FLOW_GUARD "Enable Control Flow Guard hardening (MSVC)" ON)
	if (USE_CONTROL_FLOW_GUARD)
        set(SECURITY_C_FLAGS "/guard:cf")
        set(SECURITY_CXX_FLAGS "/guard:cf")
        set(SECURITY_EXE_LINKER_FLAGS "/GUARD:CF")
        set(SECURITY_SHARED_LINKER_FLAGS "/GUARD:CF")
        set(SECURITY_MODULE_LINKER_FLAGS "/GUARD:CF")
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
   set(PATCH_COMMAND patch)
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
    set(PARENT_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
    string(REPLACE ";" "$<SEMICOLON>" CMAKE_OSX_ARCHITECTURES_ESCAPED "${CMAKE_OSX_ARCHITECTURES}")
    set(GLOBAL_PROFILE ${GLOBAL_PROFILE}
                        -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
                        -DCMAKE_INCLUDE_PATH:PATH=${CMAKE_PREFIX_PATH}/include
                        -DCMAKE_LIBRARY_PATH:PATH=${CMAKE_PREFIX_PATH}/lib
                        -DCMAKE_MACOSX_RPATH=ON
                        -DKDE_SKIP_RPATH_SETTINGS=ON
                        -DBUILD_WITH_INSTALL_RPATH=ON
                        -DAPPLE_SUPPRESS_X11_WARNING=ON
                        -DCMAKE_FIND_FRAMEWORK=LAST
                        -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES_ESCAPED}
    )

    list(LENGTH CMAKE_OSX_ARCHITECTURES MACOS_ARCHS)
    list(JOIN CMAKE_OSX_ARCHITECTURES " " CMAKE_OSX_ARCHITECTURES_STR)

    foreach(arch ${CMAKE_OSX_ARCHITECTURES})
        string(APPEND MACOS_ARCH_FLAGS "-arch ${arch} ")
    endforeach()
    string(STRIP "${MACOS_ARCH_FLAGS}" MACOS_ARCH_FLAGS)

    set(GLOBAL_AUTOMAKE_PROFILE 
        "CFLAGS=${MACOS_ARCH_FLAGS}"
        "CXXFLAGS=${MACOS_ARCH_FLAGS}"
    )

    set(PATCH_COMMAND patch)

endif ()

if (ANDROID)
  # Increase the stack size to match MinGW's. Prevents crashes with GMic.
  set(SECURITY_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=4194304")
  set(SECURITY_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,stack-size=4194304")
  set(SECURITY_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,stack-size=4194304")

  string(REPLACE ";" "$<SEMICOLON>" _escape_find_root_path "${CMAKE_FIND_ROOT_PATH}")
  # stl must be consistent: https://github.com/android/ndk/issues/1441
  set (GLOBAL_PROFILE ${GLOBAL_PROFILE}
                     -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                     -DANDROID_PLATFORM=${ANDROID_PLATFORM}
                     -DANDROID_ABI=${ANDROID_ABI}
                     -DANDROID_STL=${ANDROID_STL}
                     -DCMAKE_FIND_ROOT_PATH=${_escape_find_root_path}
                     -DCMAKE_EXE_LINKER_FLAGS=${SECURITY_EXE_LINKER_FLAGS}
                     -DCMAKE_SHARED_LINKER_FLAGS=${SECURITY_SHARED_LINKER_FLAGS}
                     -DCMAKE_MODULE_LINKER_FLAGS=${SECURITY_MODULE_LINKER_FLAGS})
endif()

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
        set(Python_FIND_STRATEGY LOCATION)
        find_package(Python 3.8 COMPONENTS Development Interpreter)
        if (Python_FOUND)
            message(STATUS "Python requirements met.")
            TestCompileLinkPythonLibs(CAN_USE_PYTHON_LIBS)
            DumpSitePackages(KRITA_PYTHONPATH)
            if (NOT CAN_USE_PYTHON_LIBS)
                file(READ ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log ERROR_LOG)
                string(REPLACE "\n" "\n  " ERROR_LOG "${ERROR_LOG}")
                message(FATAL_ERROR "Compiling with Python library failed, please check whether the architecture is correct!\nCMakeError.log:\n  ${ERROR_LOG}\n\n")
            endif ()
        else ()
            message(FATAL_ERROR "Python requirements not met. To disable Python deps, set ENABLE_PYTHON_DEPS to OFF.")
        endif ()
    endif ()
elseif(UNIX)
    set(PYTHON_VERSION "3.10")

    set(KRITA_PYTHONPATH "${INSTALL_ROOT}/lib/python${PYTHON_VERSION}/site-packages")
    message(STATUS "Krita's PEP-0250 root: ${KRITA_PYTHONPATH}")
endif ()

# Prepare meson-compatible environment variables
if (WIN32)
    krita_to_native_path("${KRITA_PYTHONPATH}" _krita_pythonpath)
    string(TOLOWER ${_krita_pythonpath} _krita_pythonpath)
    krita_to_native_environment_path_list("${_krita_pythonpath}" _krita_pythonpath)
else()
    set(_krita_pythonpath ${KRITA_PYTHONPATH})
endif()

message(STATUS "Python environment for Krita: ${_krita_pythonpath}")

macro(mesonify VAR DEST)
    set(${DEST} "${${VAR}}")
    separate_arguments(${DEST})
    if (MSVC) # Fix compiler flags
        list(TRANSFORM ${DEST} REPLACE "^\/" "-")
    endif()
    list(TRANSFORM ${DEST} REPLACE "(.+)" "\'\\1\'")
    list(FILTER ${DEST} EXCLUDE REGEX "^$")
    list(JOIN ${DEST} "," ${DEST})
    set(${DEST} "[${${DEST}}]")
endmacro()

if (ANDROID OR (CMAKE_CROSSCOMPILING AND NOT APPLE))
    set(CROSS_COMPILE_FLAGS "--sysroot=${CMAKE_SYSROOT}")
    set(CROSS_LINKER_FLAGS "--sysroot=${CMAKE_SYSROOT}")
elseif (APPLE)
    set(CROSS_COMPILE_FLAGS "-isysroot ${CMAKE_OSX_SYSROOT}")
    set(CROSS_LINKER_FLAGS "-isysroot ${CMAKE_OSX_SYSROOT}")
endif()

if (ANDROID OR CMAKE_CROSSCOMPILING)
    set(CROSS_COMPILE_FLAGS "${CROSS_COMPILE_FLAGS} --target=${CMAKE_C_COMPILER_TARGET}")
    set(CROSS_LINKER_FLAGS "${CROSS_EXE_LINKER_FLAGS} --target=${CMAKE_C_COMPILER_TARGET}")
endif()

if (ANDROID)
# Meson injects -D_FILE_OFFSET_BITS=64 which triggers off_t functions.
# Alternatively, increase API level to 24.
    set(CROSS_COMPILE_FLAGS "${CROSS_COMPILE_FLAGS} -D_LIBCPP_HAS_NO_OFF_T_FUNCTIONS")
endif()

if (CMAKE_OSX_ARCHITECTURES)
    foreach(arch ${CMAKE_OSX_ARCHITECTURES})
        string(APPEND CROSS_COMPILE_FLAGS_${arch} "${CROSS_COMPILE_FLAGS} -arch ${arch}")
    endforeach()
endif()

mesonify(SECURITY_C_FLAGS _security_c_flags)
mesonify(SECURITY_CXX_FLAGS _security_cxx_flags)
mesonify(SECURITY_EXE_LINKER_FLAGS _security_exe_linker_flags)
mesonify(CROSS_COMPILE_FLAGS _cross_compile_flags)
mesonify(CROSS_LINKER_FLAGS _cross_linker_flags)

# Block any libraries not coming from our PATH when crosscompiling
if (UNIX AND CMAKE_CROSSCOMPILING)
    set(_pkg_config_libdir "pkg_config_libdir = ''")
else()
    # In Windows either we pick up the MSYS2 pkg-config
    # or we ship our own, both use the correct architecture.
    # Linux also uses the correct architecture.
    set(_pkg_config_libdir)
endif()

include(TestBigEndian)
test_big_endian(IS_ENDIAN)
set(CROSSCOMPILING REQUIRED)

if (IS_ENDIAN)
    set(_endian "big")
else()
    set(_endian "little")
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} _system_name)

if (ANDROID OR CMAKE_CROSSCOMPILING)
    set(EXTRA_MESON_FLAGS
        --cross-file=${CMAKE_CURRENT_BINARY_DIR}/meson-compiler.ini
        --cross-file=${CMAKE_CURRENT_BINARY_DIR}/meson-host.ini
    )
    set(CROSSCOMPILING REQUIRED)
else()
    set(EXTRA_MESON_FLAGS
        --native-file=${CMAKE_CURRENT_BINARY_DIR}/meson-compiler.ini
    )
    set(CROSSCOMPILING)
endif()

if (MSVC)
    set(_c_ld "c_ld = ['${CMAKE_LINKER}'] + cross_link_args")
    set(_cpp_ld "cpp_ld = ['${CMAKE_LINKER}'] + cross_link_args")
endif()

if (CMAKE_CROSSCOMPILING AND NOT APPLE)
find_package(Python 3.0 COMPONENTS Interpreter REQUIRED)
set_package_properties(Python PROPERTIES
    DESCRIPTION "Python Library"
    URL "https://www.python.org"
    TYPE REQUIRED
    PURPOSE "Required to run Meson when crosscompiling")
endif()
find_program(MESON_BINARY_PATH meson)
find_program(NINJA_BINARY_PATH ninja)
find_package(PkgConfig QUIET ${CROSSCOMPILING})

if (NOT Python_EXECUTABLE)
    set(Python_EXECUTABLE ${EXTPREFIX}/bin/python3)
    if(NOT EXISTS "${Python_EXECUTABLE}")
        message("WARNING: using system python3!")
        SET(Python_EXECUTABLE python3)
    endif()

    set(PYTHON_DEP ext_python)
endif()

# python needs openssl
if (NOT WIN32 OR OPENSSL_DEP)
    add_subdirectory( ext_openssl )
endif()

# Meson needs the sitecustomize.py to set the site-packages root
add_subdirectory( ext_python )

if (NOT NINJA_BINARY_PATH)
    set(NINJA_DEP ext_ninja)
    message(STATUS "Ninja not available, building 3rdparty version.")
    add_subdirectory(ext_ninja)
endif()

if (NOT MESON_BINARY_PATH)
    set(MESON_DEP ext_meson)
    set(MESON_BINARY_PATH ${EXTPREFIX}/bin/meson)
    message(STATUS "Meson not available, building 3rdparty version.")
    add_subdirectory(ext_meson)
endif()

set(MESON_NASM_PATH "${EXTPREFIX}/bin/nasm${CMAKE_EXECUTABLE_SUFFIX}")

if (NOT PKG_CONFIG_FOUND OR WIN32)
    set(PKG_CONFIG_DEP ext_pkgconfig)
    if (NOT WIN32)
        set(ENV{PKG_CONFIG} ${EXTPREFIX}/bin/pkg-config)
    else()
        set(ENV{PKG_CONFIG} ${EXTPREFIX}/bin/pkgconf)
    endif()
    # meson currently reads from the variable directly
    set(PKG_CONFIG_EXECUTABLE $ENV{PKG_CONFIG})
    message(STATUS "pkg-config not available, building 3rdparty version.")
    message(STATUS "$PKG_CONFIG is set to $ENV{PKG_CONFIG}")
    add_subdirectory(ext_pkgconfig)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/meson-compiler.ini.in
    ${CMAKE_CURRENT_BINARY_DIR}/meson-compiler.ini
)

# Meson's CPU family is a bit different from what
# Android SDK exports as CMAKE_SYSTEM_PROCESSOR
set (MESON_CPU_FAMILY ${CMAKE_SYSTEM_PROCESSOR})
if (${MESON_CPU_FAMILY} STREQUAL "armv7-a")
    set(MESON_CPU_FAMILY "arm")
elseif (${MESON_CPU_FAMILY} STREQUAL "i686")
    set(MESON_CPU_FAMILY "x86")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/meson-host.ini.in
    ${CMAKE_CURRENT_BINARY_DIR}/meson-host.ini
)

# Prepare file for crosscompile multiple archs
if (CMAKE_OSX_ARCHITECTURES)
    foreach(arch ${CMAKE_OSX_ARCHITECTURES})
        mesonify(CROSS_COMPILE_FLAGS_${arch} _cross_compile_flags)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/meson-compiler.ini.in
            ${CMAKE_CURRENT_BINARY_DIR}/meson-compiler_${arch}.ini
        )
    endforeach()
endif()

LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/modules/avcodecs")

include(FeatureSummary)

# ======================== AUDIO ===========================

find_package(FLAC 1.3.3 QUIET)
set_package_properties(FLAC PROPERTIES
    DESCRIPTION "libFLAC flac audio codec"
    URL "https://xiph.org/flac/"
)
if (NOT FLAC_FOUND)
    message(STATUS "FLAC not available, building 3rdparty version.")
    set(FLAC_DEP ext_flac)
endif()

find_package(Ogg 1.3.2 QUIET)
set_package_properties(Ogg PROPERTIES
    DESCRIPTION "ogg is a library for manipulating ogg bitstreams"
    URL "https://xiph.org/vorbis/"
)
if (NOT Ogg_FOUND)
    message(STATUS "libogg not available, building 3rdparty version.")
    set(OGG_DEP ext_libogg)
endif()

find_package(Opus 1.3.1 QUIET)
set_package_properties(Opus PROPERTIES
    DESCRIPTION "Opus IETF audio codec"
    URL "https://opus-codec.org/"
)
if (NOT Opus_FOUND)
    message(STATUS "Opus not available, building 3rdparty version.")
    set(OPUS_DEP ext_opus)
endif()

# ======================== VIDEO ===========================

find_package(VPX 1.7.0 QUIET)
set_package_properties(VPX PROPERTIES
    DESCRIPTION "libvpx vp8/vp9 video encoder."
    URL "https://www.webmproject.org/"
)
if (NOT VPX_FOUND AND NOT WIN32)
    message(STATUS "libvpx not available, building 3rdparty version.")
    set(VPX_DEP ext_vpx)
endif()

find_package(SDL 2.24.0 QUIET)
set_package_properties(SDL PROPERTIES
    DESCRIPTION "Low-level access to audio, keyboard, mouse, joystick, and graphics"
    URL "https://www.libsdl.org/"
)
if (NOT SDL_FOUND)
    message(STATUS "SDL not available, building 3rdparty version.")
    set(SDL_DEP ext_sdl2)
endif()

set(QT_DEP "ext_qt")
# ================== END OF FFMPEG DEPS =====================

# this list must be dependency-ordered
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
add_subdirectory( ext_gsl )
add_subdirectory( ext_fftw3 )
add_subdirectory( ext_ocio )
add_subdirectory( ext_openjpeg )

if(WIN32)
    add_subdirectory(ext_googleangle)
endif()

if (WIN32 AND NOT PERL_FOUND)
    add_subdirectory( ext_perl )
endif()

add_subdirectory(ext_icu)
add_subdirectory( ext_qt )
if (ANDROID)
  # gettext replacement
  add_subdirectory(ext_libintl-lite)
  add_subdirectory(ext_libunwindstack-ndk)
endif()
add_subdirectory( ext_frameworks )
add_subdirectory( ext_libraw )
if (ENABLE_PYTHON_DEPS OR NOT MINGW)
    add_subdirectory( ext_sip )
    add_subdirectory( ext_pyqt )
endif ()

if (WIN32)
    add_subdirectory( ext_drmingw )
endif ()

add_subdirectory( ext_lager )
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
add_subdirectory(ext_jpegxl)
add_subdirectory(ext_xsimd)

add_subdirectory(ext_freetype)
add_subdirectory(ext_fontconfig)
add_subdirectory(ext_poppler)
add_subdirectory(ext_fribidi)
add_subdirectory(ext_unibreak)

add_subdirectory(ext_ffmpeg)
add_subdirectory(ext_mlt)
