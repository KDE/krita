project(krita)

message(STATUS "Using CMake version: ${CMAKE_VERSION}")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(MIN_QT_VERSION 5.6.0)
option(OVERRIDE_QT_VERSION "Use this to make it possible to build with Qt < 5.6.0. There will be bugs." OFF)
if (OVERRIDE_QT_VERSION)
    set(MIN_QT_VERSION 5.4.0)
endif()

set(MIN_FRAMEWORKS_VERSION 5.7.0)

if (POLICY CMP0002)
    cmake_policy(SET CMP0002 OLD)
endif()

if (POLICY CMP0017)
    cmake_policy(SET CMP0017 NEW)
endif ()

if (POLICY CMP0022)
    cmake_policy(SET CMP0022 OLD)
endif ()

if (POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif()

if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

if (POLICY CMP0046)
    cmake_policy(SET CMP0046 OLD)
endif ()

if (POLICY CMP0059)
    cmake_policy(SET CMP0059 OLD)
endif()

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 OLD)
endif()

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 OLD)
endif()

if (POLICY CMP0064)
    cmake_policy(SET CMP0064 OLD)
endif()


if (APPLE)
    set(APPLE_SUPPRESS_X11_WARNING TRUE)
    set(KDE_SKIP_RPATH_SETTINGS TRUE)
    set(CMAKE_MACOSX_RPATH 1)
    set(BUILD_WITH_INSTALL_RPATH 1)
    add_definitions(-mmacosx-version-min=10.9 -Wno-macro-redefined -Wno-deprecated-register)
endif()

if (LINUX)
    if (CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9 AND NOT WINDOWS)
        add_definitions(-Werror=delete-incomplete)
    endif()
endif()

 ######################
#######################
## Constants defines ##
#######################
######################

# define common versions of Krita applications, used to generate kritaversion.h
# update these version for every release:
set(KRITA_VERSION_STRING "4.0.0-pre-alpha")
set(KRITA_STABLE_VERSION_MAJOR 4) # 3 for 3.x, 4 for 4.x, etc.
set(KRITA_STABLE_VERSION_MINOR 0) # 0 for 3.0, 1 for 3.1, etc.
set(KRITA_VERSION_RELEASE 0)     # 88 for pre-alpha, 89 for Alpha, increase for next test releases, set 0 for first Stable, etc.
set(KRITA_ALPHA 1) # uncomment only for Alpha
#set(KRITA_BETA 1) # uncomment only for Beta
#set(KRITA_RC 1) # uncomment only for RC
set(KRITA_YEAR 2017) # update every year

if(NOT DEFINED KRITA_ALPHA AND NOT DEFINED KRITA_BETA AND NOT DEFINED KRITA_RC)
    set(KRITA_STABLE 1) # do not edit
endif()

message(STATUS "Krita version: ${KRITA_VERSION_STRING}")

# Define the generic version of the Krita libraries here
# This makes it easy to advance it when the next Krita release comes.
# 14 was the last GENERIC_KRITA_LIB_VERSION_MAJOR of the previous Krita series
# (2.x) so we're starting with 15 in 3.x series, 16 in 4.x series
if(KRITA_STABLE_VERSION_MAJOR EQUAL 4)
    math(EXPR GENERIC_KRITA_LIB_VERSION_MAJOR "${KRITA_STABLE_VERSION_MINOR} + 16")
else()
    # let's make sure we won't forget to update the "16"
    message(FATAL_ERROR "Reminder: please update offset == 16 used to compute GENERIC_KRITA_LIB_VERSION_MAJOR to something bigger")
endif()
set(GENERIC_KRITA_LIB_VERSION "${GENERIC_KRITA_LIB_VERSION_MAJOR}.0.0")
set(GENERIC_KRITA_LIB_SOVERSION "${GENERIC_KRITA_LIB_VERSION_MAJOR}")

LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/kde_macro")

# fetch git revision for the current build

set(KRITA_GIT_SHA1_STRING "")
set(KRITA_GIT_BRANCH_STRING "")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
get_git_branch(GIT_BRANCH)

if(GIT_SHA1 AND GIT_BRANCH)
    string(SUBSTRING ${GIT_SHA1} 0 7 GIT_SHA1)
    set(KRITA_GIT_SHA1_STRING ${GIT_SHA1})
    set(KRITA_GIT_BRANCH_STRING ${GIT_BRANCH})
endif()

if(NOT DEFINED RELEASE_BUILD)
    # estimate mode by CMAKE_BUILD_TYPE content if not set on cmdline
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
    set(RELEASE_BUILD_TYPES "release" "relwithdebinfo" "minsizerel")
    list(FIND RELEASE_BUILD_TYPES "${CMAKE_BUILD_TYPE_TOLOWER}" INDEX)
    if (INDEX EQUAL -1)
        set(RELEASE_BUILD FALSE)
    else()
        set(RELEASE_BUILD TRUE)
    endif()
endif()
message(STATUS "Release build: ${RELEASE_BUILD}")

# create test make targets
enable_testing()
# collect list of broken tests, empty here to start fresh with each cmake run
set(KRITA_BROKEN_TESTS "" CACHE INTERNAL "KRITA_BROKEN_TESTS")

 ############
#############
## Options ##
#############
############

include(FeatureSummary)

if (WIN32)
    option(USE_DRMINGW "Support the Dr. Mingw crash handler (only on windows)" ON)
    add_feature_info("Dr. Mingw" USE_DRMINGW "Enable the Dr. Mingw crash handler")

    if (MINGW)
        option(USE_MINGW_HARDENING_LINKER "Enable DEP (NX), ASLR and high-entropy ASLR linker flags (mingw-w64)" ON)
        add_feature_info("Linker Security Flags" USE_MINGW_HARDENING_LINKER "Enable DEP (NX), ASLR and high-entropy ASLR linker flags")
        if (USE_MINGW_HARDENING_LINKER)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
            set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--dynamicbase -Wl,--nxcompat -Wl,--disable-auto-image-base")
            if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
                # Enable high-entropy ASLR for 64-bit
                # The image base has to be >4GB for HEASLR to be enabled.
                # The values used here are kind of arbitrary.
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x140000000")
                set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x180000000")
                set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x180000000")
            endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        else (USE_MINGW_HARDENING_LINKER)
            message(WARNING "Linker Security Flags not enabled!")
        endif (USE_MINGW_HARDENING_LINKER)
    endif (MINGW)
endif ()

option(HIDE_SAFE_ASSERTS "Don't show message box for \"safe\" asserts, just ignore them automatically and dump a message to the terminal." ON)
configure_file(config-hide-safe-asserts.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hide-safe-asserts.h)
add_feature_info("Safe Asserts" HIDE_SAFE_ASSERTS "Don't show message box for \"safe\" asserts, just ignore them automatically and dump a message to the terminal.")

option(FOUNDATION_BUILD "A Foundation build is a binary release build that can package some extra things like color themes. Linux distributions that build and install Krita into a default system location should not define this option to true." OFF)
add_feature_info("Foundation Build" FOUNDATION_BUILD "A Foundation build is a binary release build that can package some extra things like color themes. Linux distributions that build and install Krita into a default system location should not define this option to true.")

option(KRITA_ENABLE_BROKEN_TESTS "Enable tests that are marked as broken" OFF)
add_feature_info("Enable Broken Tests" KRITA_ENABLE_BROKEN_TESTS "Runs broken test when \"make test\" is invoked (use -DKRITA_ENABLE_BROKEN_TESTS=ON to enable).")


include(MacroJPEG)

###########################################################
## Look for Python3. It is also searched by KF5,         ##
## so we should request the correct  version in advance  ##
###########################################################

if(MINGW)
    # Special check: Building on Windows and ext_python is used
    find_package(PythonInterp 3.6)
    if(PYTHONINTERP_FOUND)
        find_package(PythonLibrary 3.6)
        if(PYTHONLIBS_FOUND)
            include("${CMAKE_CURRENT_SOURCE_DIR}/PythonWindowsCheck.cmake")
        endif(PYTHONLIBS_FOUND)
    endif(PYTHONINTERP_FOUND)
else(MINGW)
    find_package(PythonInterp 3.0)
    find_package(PythonLibrary 3.0)
endif(MINGW)

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################

find_package(ECM 5.19 REQUIRED NOMODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(ECMOptionalAddSubdirectory)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(GenerateExportHeader)
include(ECMMarkAsTest)
include(ECMInstallIcons)

include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

# do not reorder to be alphabetical: this is the order in which the frameworks
# depend on each other.
find_package(KF5 ${MIN_FRAMEWORKS_VERSION} REQUIRED COMPONENTS
        Archive
        Config
        WidgetsAddons
        Completion
        CoreAddons
        GuiAddons
        I18n
        ItemModels
        ItemViews
        WindowSystem
)

# KConfig deprecated authorizeKAction. In order to be warning free,
# compile with the updated function when the dependency is new enough.
# Remove this (and the uses of the define) when the minimum KF5
# version is >= 5.24.0.
if (${KF5Config_VERSION} VERSION_LESS "5.24.0" )
    message("Old KConfig (< 5.24.0) found.")
    add_definitions(-DKCONFIG_BEFORE_5_24)
endif()

find_package(Qt5 ${MIN_QT_VERSION}
        REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Xml
        Network
        PrintSupport
        Svg
        Test
        Concurrent
)


include (MacroAddFileDependencies)
include (MacroBoolTo01)
include (MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("Compiling Krita inside the source directory is not possible. Please refer to the build instruction https://community.kde.org/Krita#Build_Instructions")

# Note: OPTIONAL_COMPONENTS does not seem to be reliable
# (as of ECM 5.15.0, CMake 3.2)

find_package(Qt5Multimedia ${MIN_QT_VERSION})
set_package_properties(Qt5Multimedia PROPERTIES
        DESCRIPTION "Qt multimedia integration"
        URL "http://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide sound support for animations")

macro_bool_to_01(Qt5Multimedia_FOUND HAVE_QT_MULTIMEDIA)
configure_file(config-qtmultimedia.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-qtmultimedia.h )

if (NOT WIN32 AND NOT APPLE)

    find_package(Qt5 ${MIN_QT_VERSION} REQUIRED X11Extras)

    find_package(Qt5DBus ${MIN_QT_VERSION})
    set(HAVE_DBUS ${Qt5DBus_FOUND})
    set_package_properties(Qt5DBus PROPERTIES
        DESCRIPTION "Qt DBUS integration"
        URL "http://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide a dbus api on Linux")

    find_package(KF5KIO ${MIN_FRAMEWORKS_VERSION})
    macro_bool_to_01(KF5KIO_FOUND HAVE_KIO)
    set_package_properties(KF5KIO PROPERTIES
        DESCRIPTION "KDE's KIO Framework"
        URL "http://api.kde.org/frameworks-api/frameworks5-apidocs/kio/html/index.html"
        TYPE OPTIONAL
        PURPOSE "Optionally used for recent document handling")

    find_package(KF5Crash ${MIN_FRAMEWORKS_VERSION})
    macro_bool_to_01(KF5Crash_FOUND HAVE_KCRASH)
    set_package_properties(KF5Crash PROPERTIES
        DESCRIPTION "KDE's Crash Handler"
        URL "http://api.kde.org/frameworks-api/frameworks5-apidocs/kcrash/html/index.html"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide crash reporting on Linux")

    find_package(X11 REQUIRED COMPONENTS Xinput)
    set(HAVE_X11 TRUE)
    add_definitions(-DHAVE_X11)

    find_package(XCB COMPONENTS XCB ATOM)
    set(HAVE_XCB ${XCB_FOUND})
else()
    set(HAVE_DBUS FALSE)
    set(HAVE_X11 FALSE)
    set(HAVE_XCB FALSE)
endif()


add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_STRICT_ITERATORS
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_USE_FAST_CONCATENATION
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_DISABLE_DEPRECATED_BEFORE=0
)

add_definitions(-DTRANSLATION_DOMAIN=\"krita\")

#
# The reason for this mode is that the Debug mode disable inlining
#
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_KRITADEVS "-O3 -g" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fext-numeric-literals")
endif()

if(UNIX)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
endif()

if(WIN32)
    if(MSVC)
        # C4522: 'class' : multiple assignment operators specified
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4522")
    endif()
endif()

# enable exceptions globally
kde_enable_exceptions()


# only with this definition will all the FOO_TEST_EXPORT macro do something
# TODO: check if this can be moved to only those places which make use of it,
# to reduce global compiler definitions that would trigger a recompile of
# everything on a change (like adding/removing tests to/from the build)
if(BUILD_TESTING)
    add_definitions(-DCOMPILING_TESTS)
endif()

set(KRITA_DEFAULT_TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/sdk/tests/data/)
macro(macro_add_unittest_definitions)
  add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
  add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  add_definitions(-DFILES_DEFAULT_DATA_DIR="${KRITA_DEFAULT_TEST_DATA_DIR}")
  add_definitions(-DSYSTEM_RESOURCES_DATA_DIR="${CMAKE_SOURCE_DIR}/krita/data/")
endmacro()




# overcome some platform incompatibilities
if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/winquirks)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    set(WIN32_PLATFORM_NET_LIBS ws2_32.lib netapi32.lib)
endif()

# set custom krita plugin installdir
set(KRITA_PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR}/kritaplugins)

 ###########################
############################
## Required dependencies  ##
############################
###########################

find_package(PNG REQUIRED)

if (APPLE)
    # this is not added correctly on OSX -- see http://forum.kde.org/viewtopic.php?f=139&t=101867&p=221242#p221242
    include_directories(SYSTEM ${PNG_INCLUDE_DIR})
endif()

add_definitions(-DBOOST_ALL_NO_LIB)
find_package(Boost 1.55 REQUIRED COMPONENTS system) # for pigment and stage
include_directories(${Boost_INCLUDE_DIRS})
##
## Test for GNU Scientific Library
##
find_package(GSL)
set_package_properties(GSL PROPERTIES
    URL "http://www.gnu.org/software/gsl"
    TYPE RECOMMENDED
    PURPOSE "Required by Krita's Transform tool.")
macro_bool_to_01(GSL_FOUND HAVE_GSL)
configure_file(config-gsl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-gsl.h )

 ###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Check for OpenEXR
##
find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "Compression library"
    URL "http://www.zlib.net/"
    TYPE OPTIONAL
    PURPOSE "Optionally used by the G'Mic and the PSD plugins")
macro_bool_to_01(ZLIB_FOUND HAVE_ZLIB)

find_package(OpenEXR)
set_package_properties(OpenEXR PROPERTIES
    DESCRIPTION "High dynamic-range (HDR) image file format"
    URL "http://www.openexr.com"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita OpenEXR filter")
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
    include_directories(SYSTEM ${OPENEXR_INCLUDE_DIR})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif()

find_package(TIFF)
set_package_properties(TIFF PROPERTIES
    DESCRIPTION "TIFF Library and Utilities"
    URL "http://www.remotesensing.org/libtiff"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita TIFF filter")

find_package(JPEG)
set_package_properties(JPEG PROPERTIES
    DESCRIPTION "Free library for JPEG image compression. Note: libjpeg8 is NOT supported."
    URL "http://www.libjpeg-turbo.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita JPEG filter")

set(LIBRAW_MIN_VERSION  "0.16")
find_package(LibRaw ${LIBRAW_MIN_VERSION})
set_package_properties(LibRaw PROPERTIES
    DESCRIPTION "Library to decode RAW images"
    URL "http://www.libraw.org"
    TYPE OPTIONAL
    PURPOSE "Required to build the raw import plugin")

find_package(FFTW3)
set_package_properties(FFTW3 PROPERTIES
    DESCRIPTION "A fast, free C FFT library"
    URL "http://www.fftw.org/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita for fast convolution operators and some G'Mic features")
macro_bool_to_01(FFTW3_FOUND HAVE_FFTW3)

find_package(OCIO)
set_package_properties(OCIO PROPERTIES
    DESCRIPTION "The OpenColorIO Library"
    URL "http://www.opencolorio.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita LUT docker")
macro_bool_to_01(OCIO_FOUND HAVE_OCIO)

##
## Look for OpenGL
##
# TODO: see if there is a better check for QtGui being built with opengl support (and thus the QOpenGL* classes)
if(Qt5Gui_OPENGL_IMPLEMENTATION)
    message(STATUS "Found QtGui OpenGL support")
else()
    message(FATAL_ERROR  "Did NOT find QtGui OpenGL support. Check your Qt configuration. You cannot build Krita without Qt OpenGL support.")
endif()

##
## Test for eigen3
##
find_package(Eigen3 3.0 REQUIRED)
set_package_properties(Eigen3 PROPERTIES
    DESCRIPTION "C++ template library for linear algebra"
    URL "http://eigen.tuxfamily.org"
    TYPE REQUIRED)

##
## Test for exiv2
##
find_package(Exiv2 0.16 REQUIRED)
set_package_properties(Exiv2 PROPERTIES
    DESCRIPTION "Image metadata library and tools"
    URL "http://www.exiv2.org"
    PURPOSE "Required by Krita")

##
## Test for lcms
##
find_package(LCMS2 2.4 REQUIRED)
set_package_properties(LCMS2 PROPERTIES
    DESCRIPTION "LittleCMS Color management engine"
    URL "http://www.littlecms.com"
    TYPE REQUIRED
    PURPOSE "Will be used for color management and is necessary for Krita")
if(LCMS2_FOUND)
    if(NOT ${LCMS2_VERSION} VERSION_LESS 2040 )
        set(HAVE_LCMS24 TRUE)
    endif()
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS2 TRUE)
endif()

##
## Test for Vc
##
set(OLD_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} )
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )
set(HAVE_VC FALSE)
if( NOT MSVC)
    find_package(Vc 1.1.0)
    set_package_properties(Vc PROPERTIES
        DESCRIPTION "Portable, zero-overhead SIMD library for C++"
        URL "https://github.com/VcDevel/Vc"
        TYPE OPTIONAL
        PURPOSE "Required by the Krita for vectorization")
    macro_bool_to_01(Vc_FOUND HAVE_VC)
endif()
configure_file(config-vc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-vc.h )

if(HAVE_VC)
    message(STATUS "Vc found!")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                        "${CMAKE_SOURCE_DIR}/cmake/vc")

    include (VcMacros)

    if(Vc_COMPILER_IS_CLANG)
        set(ADDITIONAL_VC_FLAGS "-Wabi -ffp-contract=fast -fPIC")
    elseif (NOT MSVC)
        set(ADDITIONAL_VC_FLAGS "-Wabi -fabi-version=0 -ffp-contract=fast -fPIC")
    endif()

    #Handle Vc master
    if(Vc_COMPILER_IS_GCC OR Vc_COMPILER_IS_CLANG)
        AddCompilerFlag("-std=c++11" _ok)
        if(NOT _ok)
            AddCompilerFlag("-std=c++0x" _ok)
        endif()
    endif()

    macro(ko_compile_for_all_implementations_no_scalar _objs _src)
        vc_compile_for_all_implementations(${_objs} ${_src} FLAGS ${ADDITIONAL_VC_FLAGS} ONLY SSE2 SSSE3 SSE4_1 AVX AVX2+FMA+BMI2)
    endmacro()

    macro(ko_compile_for_all_implementations _objs _src)
        vc_compile_for_all_implementations(${_objs} ${_src} FLAGS ${ADDITIONAL_VC_FLAGS} ONLY Scalar SSE2 SSSE3 SSE4_1 AVX AVX2+FMA+BMI2)
    endmacro()
endif()
set(CMAKE_MODULE_PATH ${OLD_CMAKE_MODULE_PATH} )

add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})

if(WIN32)
    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif()

##
## Test endianess
##
include (TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

##
## Test for qt-poppler
##
find_package(Poppler COMPONENTS Qt5)
set_package_properties(Poppler PROPERTIES
    DESCRIPTION "A PDF rendering library"
    URL "http://poppler.freedesktop.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PDF filter.")

 ############################
#############################
## Add Krita helper macros ##
#############################
############################

include(MacroKritaAddBenchmark)

 ####################
#####################
## Define includes ##
#####################
####################

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR}
                           ${CMAKE_SOURCE_DIR}/interfaces
)

add_subdirectory(libs)
add_subdirectory(plugins)
add_subdirectory(benchmarks)
add_subdirectory(krita)

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )
configure_file(config_convolution.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_convolution.h)
configure_file(config-ocio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocio.h )

check_function_exists(powf HAVE_POWF)
configure_file(config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

message("\nBroken tests:")
foreach(tst ${KRITA_BROKEN_TESTS})
    message("    * ${tst}")
endforeach()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
