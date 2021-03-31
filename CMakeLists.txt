project(krita)

message(STATUS "Using CMake version: ${CMAKE_VERSION}")

cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR)

set(MIN_QT_VERSION 5.9.0)
set(MIN_FRAMEWORKS_VERSION 5.44.0)

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )


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

if (POLICY CMP0071)
    cmake_policy(SET CMP0071 OLD)
endif()

if (APPLE)
    set(APPLE_SUPPRESS_X11_WARNING TRUE)
    set(KDE_SKIP_RPATH_SETTINGS TRUE)
    set(CMAKE_MACOSX_RPATH 1)
    set(BUILD_WITH_INSTALL_RPATH 1)
    add_definitions(-mmacosx-version-min=10.12 -Wno-macro-redefined -Wno-deprecated-register)
    if (CMAKE_OSX_ARCHITECTURES)
        message(STATUS "CMake OSX architectures: ${CMAKE_OSX_ARCHITECTURES}")
    endif()
endif()

if (CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9 AND NOT WIN32)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-suggest-override> -Wextra -Wno-class-memaccess)
endif()

 ######################
#######################
## Constants defines ##
#######################
######################

# define common versions of Krita applications, used to generate kritaversion.h
# update these version for every release:
set(KRITA_VERSION_STRING "5.0.0-prealpha")
# Major version: 3 for 3.x, 4 for 4.x, etc.
set(KRITA_STABLE_VERSION_MAJOR 5)
# Minor version: 0 for 4.0, 1 for 4.1, etc.
set(KRITA_STABLE_VERSION_MINOR 0)
# Bugfix release version, or 0 for before the first stable release
set(KRITA_VERSION_RELEASE 0)
# the 4th digit, really only used for the Windows installer:
# - [Pre-]Alpha: Starts from 0, increment 1 per release
# - Beta: Starts from 50, increment 1 per release
# - Stable: Set to 100, bump to 101 if emergency update is needed
set(KRITA_VERSION_REVISION 0)
set(KRITA_ALPHA 1) # uncomment only for Alpha
#set(KRITA_BETA 1) # uncomment only for Beta
#set(KRITA_RC 1) # uncomment only for RC

if(NOT DEFINED KRITA_ALPHA AND NOT DEFINED KRITA_BETA AND NOT DEFINED KRITA_RC)
    set(KRITA_STABLE 1) # do not edit
endif()

message(STATUS "Krita version: ${KRITA_VERSION_STRING}")

# Define the generic version of the Krita libraries here
# This makes it easy to advance it when the next Krita release comes.
# 14 was the last GENERIC_KRITA_LIB_VERSION_MAJOR of the previous Krita series
# (2.x) so we're starting with 15 in 3.x series, 16 in 4.x series
if(KRITA_STABLE_VERSION_MAJOR EQUAL 5)
    math(EXPR GENERIC_KRITA_LIB_VERSION_MAJOR "${KRITA_STABLE_VERSION_MINOR} + 17")
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
get_git_head_hash(GIT_SHA1)
get_git_branch(GIT_BRANCH)

if(GIT_SHA1)
   string(SUBSTRING ${GIT_SHA1} 0 7 GIT_SHA1)
   set(KRITA_GIT_SHA1_STRING ${GIT_SHA1})
   if(GIT_BRANCH)
       set(KRITA_GIT_BRANCH_STRING ${GIT_BRANCH})
   else()
       set(KRITA_GIT_BRANCH_STRING "(detached HEAD)")
   endif()
endif()

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
add_feature_info("Hide safe asserts" HIDE_SAFE_ASSERTS "Don't show message box for \"safe\" asserts, just ignore them automatically and dump a message to the terminal.")

option(CRASH_ON_SAFE_ASSERTS "Crash unconditionally whenever a \"safe\" assert happens. Useful for running unittests" OFF)
add_feature_info("Crash on safe asserts" CRASH_ON_SAFE_ASSERTS "Crash unconditionally whenever a \"safe\" assert happens. Useful for running unittests")

configure_file(config-safe-asserts.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-safe-asserts.h)

option(USE_LOCK_FREE_HASH_TABLE "Use lock free hash table instead of blocking." ON)
configure_file(config-hash-table-implementation.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hash-table-implementation.h)
add_feature_info("Lock free hash table" USE_LOCK_FREE_HASH_TABLE "Use lock free hash table instead of blocking.")

option(FOUNDATION_BUILD "A Foundation build is a binary release build that can package some extra things like color themes. Linux distributions that build and install Krita into a default system location should not define this option to true." OFF)
add_feature_info("Foundation Build" FOUNDATION_BUILD "A Foundation build is a binary release build that can package some extra things like color themes. Linux distributions that build and install Krita into a default system location should not define this option to true.")

option(KRITA_ENABLE_BROKEN_TESTS "Enable tests that are marked as broken" OFF)
add_feature_info("Enable Broken Tests" KRITA_ENABLE_BROKEN_TESTS "Runs broken test when \"make test\" is invoked (use -DKRITA_ENABLE_BROKEN_TESTS=ON to enable).")

option(LIMIT_LONG_TESTS "Run long running unittests in a limited quick mode" ON)
configure_file(config-limit-long-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-limit-long-tests.h)
add_feature_info("Limit long tests" LIMIT_LONG_TESTS "Run long running unittests in a limited quick mode")

option(ENABLE_PYTHON_2 "Enables the compiler to look for Python 2.7 instead of Python 3. Some packaged scripts are not compatible with Python 2 and this should only be used if you really have to use 2.7." OFF)

option(BUILD_KRITA_QT_DESIGNER_PLUGINS "Build Qt Designer plugins for Krita widgets" OFF)
add_feature_info("Build Qt Designer plugins" BUILD_KRITA_QT_DESIGNER_PLUGINS "Builds Qt Designer plugins for Krita widgets (use -DBUILD_KRITA_QT_DESIGNER_PLUGINS=ON to enable).")

option(ENABLE_UPDATERS "Enable updaters/update notifications" ON)
configure_file(config-updaters.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-updaters.h)
add_feature_info("Enable updaters" ENABLE_UPDATERS "Enable updaters/update notifications.")

# Branding. Available options: default, Beta, Plus, Next. Can be set from command line
if (NOT DEFINED BRANDING)
	set(BRANDING "default")
endif()
message(STATUS "Branding selected: ${BRANDING}")

include(MacroJPEG)

 #########################################################
## Look for Python3 It is also searched by KF5,         ##
## so we should request the correct version in advance  ##
#########################################################

function(TestCompileLinkPythonLibs OUTPUT_VARNAME)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_INCLUDES ${PYTHON_INCLUDE_PATH})
    set(CMAKE_REQUIRED_LIBRARIES ${PYTHON_LIBRARIES})
    if (MINGW)
        set(CMAKE_REQUIRED_DEFINITIONS -D_hypot=hypot)
    endif (MINGW)
    unset(${OUTPUT_VARNAME} CACHE)
    CHECK_CXX_SOURCE_COMPILES("
#include <Python.h>
int main(int argc, char *argv[]) {
    Py_InitializeEx(0);
}" ${OUTPUT_VARNAME})
endfunction()

if(MINGW)
    if(ENABLE_PYTHON_2)
        message(FATAL_ERROR "Python 2.7 is not supported on Windows at the moment.")
    else(ENABLE_PYTHON_2)
        find_package(PythonInterp 3.8 EXACT)
        find_package(PythonLibs 3.8 EXACT)
    endif(ENABLE_PYTHON_2)
    if (PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
        if(ENABLE_PYTHON_2)
            find_package(PythonLibrary 2.7)
        else(ENABLE_PYTHON_2)
            find_package(PythonLibrary 3.8)
        endif(ENABLE_PYTHON_2)
        TestCompileLinkPythonLibs(CAN_USE_PYTHON_LIBS)
        if (NOT CAN_USE_PYTHON_LIBS)
            message(FATAL_ERROR "Compiling with Python library failed, please check whether the architecture is correct. Python will be disabled.")
        endif (NOT CAN_USE_PYTHON_LIBS)
    endif (PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
else(MINGW)
    if(ENABLE_PYTHON_2)
        find_package(PythonInterp 2.7)
        find_package(PythonLibrary 2.7)
    else(ENABLE_PYTHON_2)
        find_package(PythonInterp 3.0)
        find_package(PythonLibrary 3.0)
    endif(ENABLE_PYTHON_2)
endif(MINGW)

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################

find_package(ECM 5.22 REQUIRED NOMODULE)
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

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po)
    # Block KDE's translation infrastructure, we'll use our own
    add_custom_target(fetch-translations)
endif()
include(KDECMakeSettings)
include(KDECompilerSettings)

# do not reorder to be alphabetical: this is the order in which the frameworks
# depend on each other.
find_package(KF5 ${MIN_FRAMEWORKS_VERSION} REQUIRED COMPONENTS
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
        Sql
)

if (ANDROID)
    find_package(Qt5 ${MIN_QT_VERSION}
        REQUIRED COMPONENTS
        AndroidExtras
    )
endif()

if (WIN32)
    set(CMAKE_REQUIRED_INCLUDES ${Qt5Core_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES ${Qt5Core_LIBRARIES})

    CHECK_CXX_SOURCE_COMPILES("
#include <QCoreApplication>
int main(int argc, char *argv[]) {
QCoreApplication::setAttribute(Qt::AA_MSWindowsUseWinTabAPI);
}
"
    QT_HAS_WINTAB_SWITCH
    )

    unset(CMAKE_REQUIRED_INCLUDES)
    unset(CMAKE_REQUIRED_LIBRARIES)

    option(USE_QT_TABLET_WINDOWS "Do not use Krita's forked Wintab and Windows Ink support on Windows, but leave everything to Qt." ON)
	add_feature_info("Use Qt's Windows Tablet Support" USE_QT_TABLET_WINDOWS "Do not use Krita's forked Wintab and Windows Ink support on Windows, but leave everything to Qt.")
    configure_file(config_use_qt_tablet_windows.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_use_qt_tablet_windows.h)
endif ()     


set(CMAKE_REQUIRED_INCLUDES ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES})

CHECK_CXX_SOURCE_COMPILES("
#include <QSurfaceFormat>
int main(int argc, char *argv[]) {
QSurfaceFormat fmt;
fmt.setColorSpace(QSurfaceFormat::scRGBColorSpace);
fmt.setColorSpace(QSurfaceFormat::bt2020PQColorSpace);
}
"
HAVE_HDR
)
configure_file(config-hdr.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hdr.h)

CHECK_CXX_SOURCE_COMPILES("
#include <QGuiApplication>
int main(int argc, char *argv[]) {
QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
}
"
HAVE_HIGH_DPI_SCALE_FACTOR_ROUNDING_POLICY
)
configure_file(config-high-dpi-scale-factor-rounding-policy.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-high-dpi-scale-factor-rounding-policy.h)

if (WIN32)
    CHECK_CXX_SOURCE_COMPILES("
#include <QtPlatformHeaders/QWindowsWindowFunctions>
int main(int argc, char *argv[]) {
QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
}
"
    HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
    )
    configure_file(config-set-has-border-in-full-screen-default.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-set-has-border-in-full-screen-default.h)
endif (WIN32)

unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)


include (MacroAddFileDependencies)
include (MacroBoolTo01)
include (MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("Compiling Krita inside the source directory is not possible. Please refer to the build instruction https://community.kde.org/Krita#Build_Instructions")


# Note: OPTIONAL_COMPONENTS does not seem to be reliable
# (as of ECM 5.15.0, CMake 3.2)

find_package(Qt5Multimedia ${MIN_QT_VERSION})
set_package_properties(Qt5Multimedia PROPERTIES
        DESCRIPTION "Qt multimedia integration"
        URL "https://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide sound support for animations")

macro_bool_to_01(Qt5Multimedia_FOUND HAVE_QT_MULTIMEDIA)
configure_file(config-qtmultimedia.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-qtmultimedia.h )

if (NOT APPLE)
find_package(Qt5Quick ${MIN_QT_VERSION})
set_package_properties(Qt5Quick PROPERTIES
        DESCRIPTION "QtQuick"
        URL "https://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used for the touch gui for Krita")

macro_bool_to_01(Qt5Quick_FOUND HAVE_QT_QUICK)
if (Qt5Quick_FOUND)
    get_target_property(Qt5Quick_LIBRARY Qt5::Quick "LOCATION")
    list (APPEND ANDROID_EXTRA_LIBS ${Qt5Quick_LIBRARY})

    get_target_property(Qt5Qml_LIBRARY Qt5::Qml "LOCATION")
    list (APPEND ANDROID_EXTRA_LIBS ${Qt5Qml_LIBRARY})
endif()


find_package(Qt5QuickWidgets ${MIN_QT_VERSION})
set_package_properties(Qt5QuickWidgets PROPERTIES
        DESCRIPTION "QtQuickWidgets"
        URL "https://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used for the touch gui for Krita")
endif()

if (Qt5QuickWidgets_FOUND)
    get_target_property(Qt5QuickWidgets_LIBRARY Qt5::QuickWidgets "LOCATION")
    list (APPEND ANDROID_EXTRA_LIBS ${Qt5QuickWidgets_LIBRARY})
endif()

if (NOT WIN32 AND NOT APPLE AND NOT ANDROID)

    find_package(Qt5 ${MIN_QT_VERSION} REQUIRED X11Extras)

    find_package(Qt5DBus ${MIN_QT_VERSION})
    set(HAVE_DBUS ${Qt5DBus_FOUND})
    set_package_properties(Qt5DBus PROPERTIES
        DESCRIPTION "Qt DBUS integration"
        URL "https://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide a dbus api on Linux")

    find_package(KF5Crash ${MIN_FRAMEWORKS_VERSION})
    macro_bool_to_01(KF5Crash_FOUND HAVE_KCRASH)
    set_package_properties(KF5Crash PROPERTIES
        DESCRIPTION "KDE's Crash Handler"
        URL "https://api.kde.org/frameworks-api/frameworks5-apidocs/kcrash/html/index.html"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide crash reporting on Linux")

    find_package(X11 REQUIRED COMPONENTS Xinput)
    set(HAVE_X11 TRUE)
    add_definitions(-DHAVE_X11)

else()
    set(HAVE_DBUS FALSE)
    set(HAVE_X11 FALSE)
endif()


add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_STRICT_ITERATORS
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_USE_FAST_CONCATENATION 
  -DQT_USE_FAST_OPERATOR_PLUS
)


#if (${Qt5_VERSION} VERSION_GREATER "5.14.0" )
#    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50F00)
#elseif (${Qt5_VERSION} VERSION_GREATER "5.13.0" )
#    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50E00)
#elseif (${Qt5_VERSION} VERSION_GREATER "5.12.0" )
#    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50D00)
#elseif (${Qt5_VERSION} VERSION_GREATER "5.11.0" )
#    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50C00)
#if(${Qt5_VERSION} VERSION_GREATER "5.10.0" )
#    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50B00)
#if(${Qt5_VERSION} VERSION_GREATER "5.9.0" )
#    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50A00)
#else()
    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50900)
#endif()
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DTRANSLATION_DOMAIN=\"krita\")

#
# The reason for this mode is that the Debug mode disable inlining
#
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fext-numeric-literals")
endif()

option(KRITA_DEVS "For Krita developers. This modifies the DEBUG build type to use -O3 -g, while still enabling Q_ASSERT. This is necessary because the Qt5 cmake modules normally append QT_NO_DEBUG to any build type that is not labeled Debug")
if (KRITA_DEVS)
    set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g" CACHE STRING "" FORCE)
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

# KDECompilerSettings adds the `--export-all-symbols` linker flag.
# We don't really need it.
if(MINGW)
    string(REPLACE "-Wl,--export-all-symbols" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    string(REPLACE "-Wl,--export-all-symbols" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
endif(MINGW)

if(MINGW)
    # Hack CMake's variables to tell AR to create thin archives to reduce unnecessary writes.
    # Source of definition: https://github.com/Kitware/CMake/blob/v3.14.1/Modules/Platform/Windows-GNU.cmake#L128
    # Thin archives: https://sourceware.org/binutils/docs/binutils/ar.html#index-thin-archives
    macro(mingw_use_thin_archive lang)
        foreach(rule CREATE_SHARED_MODULE CREATE_SHARED_LIBRARY LINK_EXECUTABLE)
            string(REGEX REPLACE "(<CMAKE_AR> [^ T]+) " "\\1T " CMAKE_${lang}_${rule} "${CMAKE_${lang}_${rule}}")
        endforeach()
    endmacro()
    mingw_use_thin_archive(CXX)
endif(MINGW)

# enable exceptions globally
kde_enable_exceptions()

set(KRITA_DEFAULT_TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/sdk/tests/data/)
macro(macro_add_unittest_definitions)
  add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
  if(WIN32)
    add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  else()
    add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  endif()
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
if (ANDROID)
    # use default ABI
    if (NOT ANDROID_ABI)
        set (ANDROID_ABI arm64-v8a)
    endif()
    set (ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT})
    set (KRITA_PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR})
    # set (DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/assets)
else()
    set (KRITA_PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR}/kritaplugins)
endif()


 ###########################
############################
## Required dependencies  ##
############################
###########################

# FIXME: Still hardcoded
if (ANDROID)
    set (Boost_INCLUDE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/i/${ANDROID_ABI}/include/boost-1_70)
    set (Boost_LIBRARY_DIR          ${CMAKE_CURRENT_BINARY_DIR}/i/${ANDROID_ABI}/lib)
    set (KF5_LIBRARIES              ${CMAKE_CURRENT_BINARY_DIR}/i/lib)
endif()

find_package(PNG REQUIRED "1.2.6")
list (APPEND ANDROID_EXTRA_LIBS ${PNG_LIBRARY})

if (APPLE)
    # this is not added correctly on OSX -- see https://forum.kde.org/viewtopic.php?f=139&t=101867&p=221242#p221242
    include_directories(SYSTEM ${PNG_INCLUDE_DIR})
endif()

add_definitions(-DBOOST_ALL_NO_LIB)
find_package(Boost 1.55 REQUIRED COMPONENTS system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
##
## Test for GNU Scientific Library
##
find_package(GSL)
set_package_properties(GSL PROPERTIES
    URL "https://www.gnu.org/software/gsl"
    TYPE RECOMMENDED
    PURPOSE "Required by Krita's Transform tool.")
macro_bool_to_01(GSL_FOUND HAVE_GSL)
configure_file(config-gsl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-gsl.h )
if (GSL_FOUND)
    list (APPEND ANDROID_EXTRA_LIBS ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
endif()

 ###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Test for KSeExpr
##
find_package(KSeExpr 4.0.0.0)
set_package_properties(KSeExpr PROPERTIES
    URL "https://invent.kde.org/graphics/kseexpr"
    TYPE OPTIONAL
    PURPOSE "Required by the SeExpr generator layer"
)
macro_bool_to_01(KSeExpr_FOUND HAVE_SEEXPR)
configure_file(config-seexpr.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-seexpr.h )
if(KSeExpr_FOUND)
    get_target_property(KSeExpr_LIBRARY KSeExpr::KSeExpr "LOCATION")
    get_target_property(KSeExprUI_LIBRARY KSeExpr::KSeExprUI "LOCATION") 
    list (APPEND ANDROID_EXTRA_LIBS ${KSeExpr_LIBRARY} ${KSeExprUI_LIBRARY})
endif()

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "Compression library"
    URL "https://www.zlib.net/"
    TYPE OPTIONAL
    PURPOSE "Optionally used by the G'Mic and the PSD plugins")
macro_bool_to_01(ZLIB_FOUND HAVE_ZLIB)

find_package(OpenEXR)
set_package_properties(OpenEXR PROPERTIES
    DESCRIPTION "High dynamic-range (HDR) image file format"
    URL "https://www.openexr.com"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita OpenEXR filter")
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
    include_directories(SYSTEM ${OPENEXR_INCLUDE_DIRS})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif()

find_package(TIFF)
set_package_properties(TIFF PROPERTIES
    DESCRIPTION "TIFF Library and Utilities"
    URL "http://www.libtiff.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita TIFF filter")
if (TIFF_FOUND)
    list (APPEND ANDROID_EXTRA_LIBS ${TIFF_LIBRARY})
endif()

find_package(JPEG)
set_package_properties(JPEG PROPERTIES
    DESCRIPTION "Free library for JPEG image compression. Note: libjpeg8 is NOT supported."
    URL "https://www.libjpeg-turbo.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita JPEG filter")
if (JPEG_FOUND)
    list (APPEND ANDROID_EXTRA_LIBS ${JPEG_LIBRARY})
    macro_bool_to_01(JPEG_FOUND HAVE_JPEG)
endif()

find_package(GIF)
set_package_properties(GIF PROPERTIES
    DESCRIPTION "Library for loading and saving gif files."
    URL "http://giflib.sourceforge.net/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita GIF filter")
if (GIF_FOUND)
    list (APPEND ANDROID_EXTRA_LIBS ${GIF_LIBRARY})
endif()

find_package(HEIF "1.10.0")
set_package_properties(HEIF PROPERTIES
    DESCRIPTION "Library for loading and saving heif files."
    URL "https://github.com/strukturag/libheif"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita HEIF filter")

find_package(OpenJPEG "2.3.0")
set_package_properties(OpenJPEG PROPERTIES
    DESCRIPTION "Library for loading and saving jp2000 files."
    URL "https://www.openjpeg.org/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita JP2000 filter")

set(LIBRAW_MIN_VERSION  "0.16")
find_package(LibRaw ${LIBRAW_MIN_VERSION})
set_package_properties(LibRaw PROPERTIES
    DESCRIPTION "Library to decode RAW images"
    URL "https://www.libraw.org/"
    TYPE OPTIONAL
    PURPOSE "Required to build the raw import plugin")

find_package(FFTW3)
set_package_properties(FFTW3 PROPERTIES
    DESCRIPTION "A fast, free C FFT library"
    URL "http://www.fftw.org/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita for fast convolution operators and some G'Mic features")
macro_bool_to_01(FFTW3_FOUND HAVE_FFTW3)
if (FFTW3_FOUND)
    list (APPEND ANDROID_EXTRA_LIBS ${FFTW3_LIBRARY})
endif()

find_package(OCIO)
set_package_properties(OCIO PROPERTIES
    DESCRIPTION "The OpenColorIO Library"
    URL "https://www.opencolorio.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita LUT docker")
macro_bool_to_01(OCIO_FOUND HAVE_OCIO)

set_package_properties(PythonLibrary PROPERTIES
    DESCRIPTION "Python Library"
    URL "https://www.python.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(PYTHONLIBS_FOUND HAVE_PYTHONLIBS)

find_package(SIP "4.19.13")
set_package_properties(SIP PROPERTIES
    DESCRIPTION "Support for generating SIP Python bindings"
    URL "https://www.riverbankcomputing.com/software/sip/download"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(SIP_FOUND HAVE_SIP)

find_package(PyQt5 "5.6.0")
set_package_properties(PyQt5 PROPERTIES
    DESCRIPTION "Python bindings for Qt5."
    URL "https://www.riverbankcomputing.com/software/pyqt/download5"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(PYQT5_FOUND HAVE_PYQT5)

find_package(LibMyPaint)
set_package_properties(LibMyPaint PROPERTIES
    DESCRIPTION "MyPaint brush engine API for C/C++"
    TYPE OPTIONAL
    PURPOSE "Required for the mypaint brush engine")
if(LibMyPaint_FOUND)
    if(LibMyPaint_VERSION VERSION_LESS "1.4.0")
        message(WARNING "Please update libmypaint to version above 1.4.0 as lower versions are known to crash")
    endif()
endif()
macro_bool_to_01(LibMyPaint_FOUND HAVE_LIBMYPAINT)
configure_file(config-mypaint.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-mypaint.h )

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
find_package(LibExiv2 0.16 REQUIRED)
if (ANDROID)
    list (APPEND ANDROID_EXTRA_LIBS ${LibExiv2_LIBRARIES})
    # because libexiv2 depends on libexpat and it is installed in the same folder
    get_filename_component (_base_dir ${LibExiv2_LIBRARIES} DIRECTORY)
    list (APPEND ANDROID_EXTRA_LIBS ${_base_dir}/libexpat.so)
endif()

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
    if(NOT ${LCMS2_VERSION} VERSION_LESS 2.4 )
        set(HAVE_LCMS24 TRUE)
    endif()
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS2 TRUE)

    include(CheckIncludeFileCXX)

    set(CMAKE_REQUIRED_INCLUDES ${LCMS2_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES ${LCMS2_LIBRARIES})

    if(LCMS2_FAST_FLOAT_INCLUDE_DIR)
        message(STATUS "Found LittleCMS's fast float plugin")
        set(HAVE_LCMS_FAST_FLOAT_PLUGIN 1)
    endif()
endif()
list (APPEND ANDROID_EXTRA_LIBS ${LCMS2_LIBRARIES})

##
## Test for Vc
##
set(OLD_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} )
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )
set(HAVE_VC FALSE)
if (NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    if(NOT MSVC)
        find_package(Vc 1.1.0)
        set_package_properties(Vc PROPERTIES
            DESCRIPTION "Portable, zero-overhead SIMD library for C++"
            URL "https://github.com/VcDevel/Vc"
            TYPE OPTIONAL
            PURPOSE "Required by the Krita for vectorization")
        macro_bool_to_01(Vc_FOUND HAVE_VC)
    endif()
endif()
configure_file(config-vc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-vc.h )

if(HAVE_VC)
    message(STATUS "Vc found!")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                        "${CMAKE_SOURCE_DIR}/cmake/vc")

    include (VcMacros)

    if(Vc_COMPILER_IS_CLANG)
        set(ADDITIONAL_VC_FLAGS "-ffp-contract=fast")
        if(NOT WIN32)
            set(ADDITIONAL_VC_FLAGS "${ADDITIONAL_VC_FLAGS} -fPIC")
        endif()
    elseif (NOT MSVC)
        set(ADDITIONAL_VC_FLAGS "-fabi-version=0 -ffp-contract=fast")
        if(NOT WIN32)
            set(ADDITIONAL_VC_FLAGS "${ADDITIONAL_VC_FLAGS} -fPIC")
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

##
## Test endianness
##
include (TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

##
## Test for qt-poppler
##
find_package(Poppler COMPONENTS Qt5)
set_package_properties(Poppler PROPERTIES
    DESCRIPTION "A PDF rendering library"
    URL "https://poppler.freedesktop.org/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PDF filter.")


##
## Test for quazip
##
find_package(QuaZip 0.6)
set_package_properties(QuaZip PROPERTIES
    DESCRIPTION "A library for reading and writing zip files"
    URL "https://stachenov.github.io/quazip/"
    TYPE REQUIRED
    PURPOSE "Needed for reading and writing KRA and ORA files"
)

# FIXME: better way to do this?
list (APPEND ANDROID_EXTRA_LIBS ${QUAZIP_LIBRARIES}
                                ${EXPAT_LIBRARY}
                                ${KF5_LIBRARIES}/libKF5Completion.so
                                ${KF5_LIBRARIES}/libKF5WindowSystem.so
                                ${KF5_LIBRARIES}/libKF5WidgetsAddons.so
                                ${KF5_LIBRARIES}/libKF5ItemViews.so
                                ${KF5_LIBRARIES}/libKF5ItemModels.so
                                ${KF5_LIBRARIES}/libKF5GuiAddons.so
                                ${KF5_LIBRARIES}/libKF5I18n.so
                                ${KF5_LIBRARIES}/libKF5CoreAddons.so
                                ${KF5_LIBRARIES}/libKF5ConfigGui.so
                                ${KF5_LIBRARIES}/libKF5ConfigCore.so)



# Source translations
option(FETCH_TRANSLATIONS "Retrieve translations before building Krita" ON)
add_feature_info("Retrieve translations before building" FETCH_TRANSLATIONS "Retrieve the translations from KDE before building Krita")

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po AND FETCH_TRANSLATIONS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set_package_properties(Python3 PROPERTIES
        DESCRIPTION "Python is an interpreted, high-level and general-purpose programming language."
        URL "https://python.org/"
        TYPE OPTIONAL
        PURPOSE "Required to manually parse and fetch available translations.")
    find_package(Subversion 1.4)
    set_package_properties(Subversion PROPERTIES
        DESCRIPTION "Subversion is an open source version control system"
        URL "https://subversion.apache.org/"
        TYPE OPTIONAL
        PURPOSE "Required to access the KDE i18n repository.")

    set(TRANSLATIONS ${CMAKE_CURRENT_BINARY_DIR}/po)

    if (KRITA_STABLE)
        set(tr_EXTRA_FLAGS "")
    else()
        set(tr_EXTRA_FLAGS "--unstable")
    endif()

    get_filename_component(absolute_podir ${TRANSLATIONS} ABSOLUTE)
    string(MD5 pathmd5 ${absolute_podir})

    find_file (KRITA_PO_SCRIPT copy_po.py PATHS ${CMAKE_CURRENT_SOURCE_DIR}/build-tools REQUIRED)
    if(NOT TARGET fetch-${pathmd5})
        file(MAKE_DIRECTORY ${TRANSLATIONS})

        add_custom_target(fetch-${pathmd5}
            COMMENT "Retrieving translations..."
            COMMAND ${Python3_EXECUTABLE} ${KRITA_PO_SCRIPT} ${tr_EXTRA_FLAGS}
            BYPRODUCTS ${TRANSLATIONS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po OR EXISTS ${CMAKE_CURRENT_BINARY_DIR}/po )
    find_package(KF5I18n CONFIG REQUIRED)
    ki18n_install(po)
    if (TRANSLATIONS)
        add_dependencies(pofiles-${pathmd5} fetch-${pathmd5})
        add_dependencies(tsfiles-${pathmd5} fetch-${pathmd5})
    endif()
endif()

##
## Test for Atomics
##
include(CheckAtomic)

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
if (BUILD_TESTING)
    add_subdirectory(benchmarks)
endif()
add_subdirectory(krita)

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )
configure_file(config_convolution.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_convolution.h)
configure_file(config-ocio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocio.h )

check_function_exists(powf HAVE_POWF)
configure_file(config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/packaging/windows/installer/ConfigureInstallerNsis.cmake)
endif()

message("\nBroken tests:")
foreach(tst ${KRITA_BROKEN_TESTS})
    message("    * ${tst}")
endforeach()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (ANDROID)
  # ECM passes this to Qt, which was already capable of detecting it..
  set (ANDROID_SDK_COMPILE_API "29")
  if ((${NDK_VERSION} GREATER_EQUAL 20))
    set (USE_LLVM ON)
  endif()

  if (NOT (CMAKE_CXX_STANDARD_LIBRARIES MATCHES "[^ ]*c\\+\\+[^ ]*\\.so"))
    # copied regex from ECM's toolchain
    string(REGEX REPLACE "-(clang)?([0-9].[0-9])?$" "" ECM_ANDROID_STL_ARCH ${ANDROID_TOOLCHAIN_NAME})

    set (KRITA_ANDROID_STL_PATH "${ANDROID_TOOLCHAIN_ROOT}/sysroot/usr/lib/${ECM_ANDROID_STL_ARCH}/lib${ANDROID_STL}.so")
    if (NOT EXISTS ${KRITA_ANDROID_STL_PATH})
      message(FATAL_ERROR "STL does not exist at the right location, please use NDK r20+")
    endif()

    set (CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} \"${KRITA_ANDROID_STL_PATH}\"" CACHE STRING "" FORCE)
  endif()
endif()

if(DEFINED QTANDROID_EXPORTED_TARGET AND NOT TARGET "create-apk")
    set (_CMAKE_ANDROID_DIR "${ECM_DIR}/../toolchain")
    list(LENGTH QTANDROID_EXPORTED_TARGET targetsCount)
    include(${_CMAKE_ANDROID_DIR}/ECMAndroidDeployQt.cmake)

    math(EXPR last "${targetsCount}-1")
    foreach(idx RANGE 0 ${last})
        list(GET QTANDROID_EXPORTED_TARGET ${idx} exportedTarget)
        list(GET ANDROID_APK_DIR ${idx} APK_DIR)
        if(APK_DIR AND NOT EXISTS "${ANDROID_APK_DIR}/AndroidManifest.xml" AND IS_ABSOLUTE ANDROID_APK_DIR)
            message(FATAL_ERROR "Cannot find ${APK_DIR}/AndroidManifest.xml according to ANDROID_APK_DIR. ${ANDROID_APK_DIR} ${exportedTarget}")
        elseif(NOT APK_DIR)
            get_filename_component(_qt5Core_install_prefix "${Qt5Core_DIR}/../../../" ABSOLUTE)
            set(APK_DIR "${_qt5Core_install_prefix}/src/android/templates/")
        endif()

        ecm_androiddeployqt("${exportedTarget}" "${ECM_ADDITIONAL_FIND_ROOT_PATH}")
        set_target_properties(create-apk-${exportedTarget} PROPERTIES ANDROID_APK_DIR "${APK_DIR}")
    endforeach()
elseif(ANDROID)
    message(STATUS "You can export a target by specifying -DQTANDROID_EXPORTED_TARGET=<targetname> and -DANDROID_APK_DIR=<paths>")
endif()

