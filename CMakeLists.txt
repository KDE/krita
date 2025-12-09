cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(SUPPORTED_LANGUAGES CXX C)

if (APPLE)
    list (APPEND SUPPORTED_LANGUAGES OBJC OBJCXX)
endif()

project(krita LANGUAGES ${SUPPORTED_LANGUAGES})
message(STATUS "Using CMake version: ${CMAKE_VERSION}")

set(MIN_QT_VERSION 5.12.0)
set(MIN_FRAMEWORKS_VERSION 5.44.0)

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

if (POLICY CMP0002)
    cmake_policy(SET CMP0002 NEW)
endif()

if (POLICY CMP0017)
    cmake_policy(SET CMP0017 NEW)
endif ()

if (POLICY CMP0020)
    cmake_policy(SET CMP0020 NEW)
endif ()

if (POLICY CMP0022)
    cmake_policy(SET CMP0022 NEW)
endif ()

if (POLICY CMP0026)
    cmake_policy(SET CMP0026 NEW)
endif()

if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

if (POLICY CMP0046)
    cmake_policy(SET CMP0046 NEW)
endif ()

if (POLICY CMP0059)
    cmake_policy(SET CMP0059 NEW)
endif()

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

if (POLICY CMP0064)
    cmake_policy(SET CMP0064 NEW)
endif()

if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# in CMake >=3.30; BoostConfig.cmake is shipped only by Boost >=1.82
if (POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

if (APPLE)
    add_subdirectory(packaging/macos)

    if(NOT CMAKE_GENERATOR MATCHES "Xcode")
      execute_process(COMMAND
        xcodebuild -version -sdk macosx ProductVersion
        OUTPUT_VARIABLE XCODE_VERSION)
    endif()
    message(STATUS "XCode version: ${XCODE_VERSION}")
    set(MACOSX_RPATH TRUE)
    set(APPLE_SUPPRESS_X11_WARNING TRUE)
    set(KDE_SKIP_RPATH_SETTINGS TRUE)
    set(CMAKE_FIND_FRAMEWORK LAST)
    set(CMAKE_MACOSX_RPATH 1)
    set(BUILD_WITH_INSTALL_RPATH 1)
    set(MACOS_GUI_TEST "GUI")
    if (BUILD_WITH_QT6)
        set(MACOS_MINIMUM_VERSION 12)
    else()
        set(MACOS_MINIMUM_VERSION 10.14)
    endif()
    add_definitions(-mmacosx-version-min=${MACOS_MINIMUM_VERSION} -Wno-macro-redefined -Wno-deprecated-register)
    if (CMAKE_OSX_ARCHITECTURES)
        message(STATUS "CMake OSX architectures: ${CMAKE_OSX_ARCHITECTURES}")
    endif()

endif()

function(macos_test_fixrpath)
    if (APPLE AND BUILD_TESTING)
    foreach(TEST IN LISTS ARGN)
        set_property(TARGET ${TEST}
                     PROPERTY BUILD_RPATH "@loader_path/../../../../lib;@loader_path/../lib;@loader_path/../Frameworks;${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
    endforeach()
    endif()
endfunction()

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
set(KRITA_VERSION_STRING "5.2.14")
# Major version: 3 for 3.x, 4 for 4.x, etc.
set(KRITA_STABLE_VERSION_MAJOR 5)
# Minor version: 0 for 4.0, 1 for 4.1, etc.
set(KRITA_STABLE_VERSION_MINOR 2)
# Bugfix release version, or 0 for before the first stable release
set(KRITA_VERSION_RELEASE 14)
# the 4th digit, really only used for the Windows installer:
# - [Pre-]Alpha: Starts from 0, increment 1 per release
# - Beta: Starts from 50, increment 1 per release
# - Stable: Set to 100, bump to 101 if emergency update is needed
set(KRITA_VERSION_REVISION 0)
# Uncomment the following if this is currently in the "stable" branch.
# Do not uncomment for master branch.
set(KRITA_STABLE_BRANCH 1)
#set(KRITA_ALPHA 1) # uncomment only for Alpha
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
   set(KRITA_GIT_SHA1_STRING ${GIT_SHA1} CACHE STRING "Git commit of the current build" FORCE)
   if(NOT GIT_BRANCH)
       set(GIT_BRANCH "(detached HEAD)")
   endif()
   set(KRITA_GIT_BRANCH_STRING ${GIT_BRANCH} CACHE STRING "Git branch of the current build" FORCE)
endif()

# create test make targets
enable_testing()
# collect list of broken tests, empty here to start fresh with each cmake run
set(KRITA_BROKEN_TESTS "" CACHE INTERNAL "KRITA_BROKEN_TESTS")
# Keep track of all test target (need special attention on macos)
set(KRITA_TESTS_TARGET "" CACHE INTERNAL "KRITA_TESTS_TARGET")

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
            # Enable high-entropy ASLR for 64-bit
            # The image base has to be >4GB for HEASLR to be enabled.
            # The values used here are kind of arbitrary.
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x140000000")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x180000000")
            set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--high-entropy-va -Wl,--image-base,0x180000000")
        else (USE_MINGW_HARDENING_LINKER)
            message(WARNING "Linker Security Flags not enabled!")
        endif (USE_MINGW_HARDENING_LINKER)

        # Clang does not generate DWARF aranges data by default, which makes
        # DrMingw not able to parse the DWARF debug symbols. Add -gdwarf-aranges
        # explicitly.
        # See: https://github.com/jrfonseca/drmingw/issues/42#issuecomment-516614561
        if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -gdwarf-aranges")
        endif ()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -gdwarf-aranges")
        endif ()
    elseif (MSVC)
        # Increase the stack size to match MinGW's. Prevents crashes with GMic.
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:4194304")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /STACK:4194304")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /STACK:4194304")
        option(USE_CONTROL_FLOW_GUARD "Enable Control Flow Guard hardening (MSVC)" ON)
        add_feature_info("Linker Security Flags" USE_CONTROL_FLOW_GUARD "Enable Control Flow Guard")
        if (USE_CONTROL_FLOW_GUARD)
            add_compile_options(/guard:cf)
            add_link_options(/GUARD:CF)
        endif (USE_CONTROL_FLOW_GUARD)
    endif (MINGW)
elseif(ANDROID)
    # Increase the stack size to match MinGW's. Prevents crashes with GMic.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=4194304")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,stack-size=4194304")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,stack-size=4194304")

    if (${CMAKE_ANDROID_NDK_VERSION} VERSION_GREATER_EQUAL "27.0.0"
            AND (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a" OR CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64"))
        message(STATUS "Enable 16K page alignment for Android NDK 27")
        set(PAGE_ALIGN_16K_LINKER_FLAGS -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384)
        add_link_options(${PAGE_ALIGN_16K_LINKER_FLAGS})
    endif()
endif ()

option(HIDE_SAFE_ASSERTS "Don't show message box for \"safe\" asserts, just ignore them automatically and dump a message to the terminal." ON)
add_feature_info("Hide safe asserts" HIDE_SAFE_ASSERTS "Don't show message box for \"safe\" asserts, just ignore them automatically and dump a message to the terminal.")

option(INSTALL_BENCHMARKS "Install benchmarks into the installation root to make them packagable" OFF)
add_feature_info("Install benchmarks" INSTALL_BENCHMARKS "Install benchmarks into the installation root to make them packagable")

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

option(BUILD_KRITA_QT_DESIGNER_PLUGINS "Build Qt Designer plugins for Krita widgets" OFF)
add_feature_info("Build Qt Designer plugins" BUILD_KRITA_QT_DESIGNER_PLUGINS "Builds Qt Designer plugins for Krita widgets (use -DBUILD_KRITA_QT_DESIGNER_PLUGINS=ON to enable).")

option(ENABLE_UPDATERS "Enable updaters/update notifications" ON)
configure_file(config-updaters.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-updaters.h)
add_feature_info("Enable updaters" ENABLE_UPDATERS "Enable updaters/update notifications.")

option(KRITA_ENABLE_PCH "Enable precompiled headers support" OFF)
add_feature_info("Precompiled Headers" KRITA_ENABLE_PCH "precompiled headers make build process faster on some systems")

option(DISABLE_PO_INSTALLATION "Disable installation of po files" OFF)

option(USE_EXTERNAL_RAQM "Fetch Raqm dependency online instead of the embedded one" OFF)

set(ADDRESS_SANITIZER_ENABLED FALSE)
if (ECM_ENABLE_SANITIZERS MATCHES address)
    set(ADDRESS_SANITIZER_ENABLED TRUE)
endif()

add_feature_info("ASAN address sanitizer" ADDRESS_SANITIZER_ENABLED "crash Krita if it violates address access rules (-DECM_ENABLE_SANITIZERS=address)")

# Branding. Available options: default, Beta, Plus, Next. Can be set from command line
if ("${BRANDING}" STREQUAL "")
    if (DEFINED KRITA_STABLE)
        set(BRANDING "default")
    elseif (DEFINED KRITA_BETA OR DEFINED KRITA_RC)
        set(BRANDING "Beta")
    elseif (DEFINED KRITA_STABLE_BRANCH)
        # Alpha/pre-alpha in stable branch
        set(BRANDING "Plus")
    else ()
        # Alpha/pre-alpha in master branch or other experiments
        set(BRANDING "Next")
    endif ()
endif()
message(STATUS "Branding selected: ${BRANDING}")

include(MacroJPEG)

 #########################################################
## Look for Python3 - it is also searched by KF5,       ##
## so we should request the correct version in advance  ##
#########################################################

function(TestCompileLinkPythonLibs OUTPUT_VARNAME)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_INCLUDES ${Python_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES ${Python_LIBRARIES})
    if (MINGW)
        set(CMAKE_REQUIRED_DEFINITIONS -D_hypot=hypot)
    endif (MINGW)
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

if(WIN32)
    set(Python_FIND_STRATEGY LOCATION)
    find_package(Python 3.8 COMPONENTS Development Interpreter)
    if (Python_FOUND)
        find_package(PythonLibrary 3.8)
        TestCompileLinkPythonLibs(CAN_USE_PYTHON_LIBS)
        if (NOT CAN_USE_PYTHON_LIBS)
            file(READ ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log ERROR_LOG)
            string(REPLACE "\n" "\n  " ERROR_LOG "${ERROR_LOG}")
            message(FATAL_ERROR "Compiling with Python library failed, please check whether the architecture is correct!\nCMakeError.log:\n  ${ERROR_LOG}\n\n")
        endif (NOT CAN_USE_PYTHON_LIBS)
    endif (Python_FOUND)
else(WIN32)
    find_package(PythonLibrary 3.8)
endif(WIN32)

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################

find_package(ECM 5.22 REQUIRED NOMODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
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

if (WIN32)
# KDECompilerSettings sets Windows Vista as the default,
# while MSVC's default is 0x0A00 (_WIN32_WINNT_WIN10, sdkddkver.h) and
# MinGW's is 0x0601 (_WIN32_WINNT_WIN7, _mingw.h).
# Both are enough to supply the APIs we need in main.cc, but since we
# need the Windows 8 APIs anyway for the surface orientation and Store API,
# we set the minimum here.
remove_definitions(-D_WIN32_WINNT=0x0600 -DWINVER=0x0600 -D_WIN32_IE=0x0600)
add_definitions(-D_WIN32_WINNT=0x0602 -DWINVER=0x0602  -D_WIN32_IE=0x0602)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    if (MSVC) # or CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"
        # KDE's CompilerSettings module sets -Wall -Wextra for Clang.
        # However, -Wall on clang-cl maps to -Weverything, which turns on way too
        # much, so we're using -W4 instead, which is mapped to clang's -Wall -Wextra.
        # Source: https://hg.mozilla.org/mozilla-central/rev/ffb7bfbfc328
        string(REPLACE "-Wall -Wextra" "-W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "-Wall -Wextra" "-W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        # Allow KoColorSpaceMaths operators to build.
        add_compile_options("/permissive")
    endif()

    # Remove these invalid flags.
    string(REPLACE "-fno-common" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "-fdiagnostics-color=always" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    string(REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
    string(REPLACE "-Wl,--fatal-warnings" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    string(REPLACE "-Wl,--fatal-warnings" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
endif()
endif()

# do not reorder to be alphabetical: this is the order in which the frameworks
# depend on each other.
find_package(KF5 ${MIN_FRAMEWORKS_VERSION} REQUIRED COMPONENTS
        Config
        WidgetsAddons
        Completion
        CoreAddons
        GuiAddons
        I18n
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

set(CMAKE_REQUIRED_INCLUDES ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES})


if (WIN32)
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
endif()
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

CHECK_CXX_SOURCE_COMPILES("
#include <QMdiArea>
int main(int argc, char *argv[]) {
QMdiArea area;
area.setOption(QMdiArea::AlwaysShowSubwindowNameInTitleBar);
}
"
HAVE_QMDIAREA_ALWAYS_SHOW_SUBWINDOW_TITLE
)
configure_file(config-qmdiarea-always-show-subwindow-title.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-qmdiarea-always-show-subwindow-title.h)

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

if (NOT APPLE)
find_package(Qt5Quick ${MIN_QT_VERSION})
set_package_properties(Qt5Quick PROPERTIES
        DESCRIPTION "QtQuick"
        URL "https://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used for the touch gui for Krita")

macro_bool_to_01(Qt5Quick_FOUND HAVE_QT_QUICK)

find_package(Qt5QuickWidgets ${MIN_QT_VERSION})
set_package_properties(Qt5QuickWidgets PROPERTIES
        DESCRIPTION "QtQuickWidgets"
        URL "https://www.qt.io/"
        TYPE OPTIONAL
        PURPOSE "Optionally used for the touch gui for Krita")
endif()
if (Qt5QuickWidgets_FOUND)
    # This is needed because Qt dependencies aren't added by ECM but by androideployqt,
    # so it doesn't benefit from our patch in ECM.
    list (APPEND ANDROID_EXTRA_LIBS $<TARGET_FILE:Qt5::QuickWidgets>)
endif()


if (NOT WIN32 AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)

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
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_USE_FAST_CONCATENATION 
  -DQT_USE_FAST_OPERATOR_PLUS
)

remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_FOREACH
)


# MSVC is unable to disambiguate between definitions of QVector<QPointF> 
# and QPolygonF. This is a known upstream bug e.g.:
# - https://phabricator.kde.org/D21314
# - https://codereview.qt-project.org/c/qt/qtbase/+/180229
# Starting with Qt 5.13, it is impossible to use strict iterators
# wholesale because of:
# https://github.com/qt/qtbase/commit/972f8845a85d6a07140025e4257cb8a1a2699b5d
if (NOT (MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") OR ${Qt5_VERSION} VERSION_LESS "5.13.0")
  add_definitions(-DQT_STRICT_ITERATORS)
endif()

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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4522 -wd4138 /Zc:__cplusplus")
        # libs\ui\dialogs\kis_about_application.cpp : fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

        # Enable intrinsics
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Oi")
        # Favor fast code
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ot")
        # Function-level linking
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gy")

        # Our code is UTF-8 encoded.
        add_compile_options(/utf-8)
    endif()
endif()

if (MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # Clang/CL is incompatible with this flag
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /RELEASE")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /RELEASE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /RELEASE")
endif()

# Force enable a good many optimizations
if (MSVC AND NOT BUILD_TESTING)
    # Aggresive inlining (Release)
    string(REPLACE "Ob2" "Ob3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    # Aggresive inlining (RelWithDebInfo)
    string(REPLACE "Ob1" "Ob3" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    # Whole program optimization
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
    # For all objects:
    # - strip unused functions
    # - fold identical functions
    # - link-time code generation
    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /OPT:ICF,REF /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /OPT:ICF,REF /LTCG")
    set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} /OPT:ICF,REF /LTCG")
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
# WARNING: in MSVC this will NOT catch exceptions thrown through C code
# see: 
# - https://learn.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-170#arguments
# - https://invent.kde.org/frameworks/extra-cmake-modules/-/blob/v5.101.0/kde-modules/KDECompilerSettings.cmake#L502-526
kde_enable_exceptions()

function(krita_select_pch_file targetname pch_file_name)
    #TODO: make kritaimage and kritaglobal targets to link to its own PCH

    if (targetname STREQUAL "kritaglobal")
        set(${pch_file_name} kis_qt_only_pch.h PARENT_SCOPE)
    elseif (targetname STREQUAL "kritaimage")
        set(${pch_file_name} kis_global_pch.h PARENT_SCOPE)
    else ()
        set(immediate_deps $<TARGET_PROPERTY:${targetname},LINK_LIBRARIES>)
        set(depends_on_kritaui $<IN_LIST:kritaui,${immediate_deps}>)
        set(depends_on_kritaimage $<IN_LIST:kritaimage,${immediate_deps}>)
        set(depends_on_kritalibkis $<IN_LIST:kritalibkis,${immediate_deps}>)
        set(depends_on_kritalibpaintop $<IN_LIST:kritalibpaintop,${immediate_deps}>)

        set(${pch_file_name} $<IF:$<OR:${depends_on_kritalibpaintop},${depends_on_kritalibkis},${depends_on_kritaui},${depends_on_kritaimage}>,kis_image_pch.h,kis_global_pch.h> PARENT_SCOPE)
    endif()
endfunction()

macro(kis_add_library)
    # the first run strips only options specific to kis_add_library
    # to create arguments for real add_library call
    cmake_parse_arguments(REAL "" PCH_FILE "" ${ARGV} )
    add_library(${REAL_UNPARSED_ARGUMENTS})

    if (KRITA_ENABLE_PCH)
        # the second run strips out all the option supported by the function
        # to let the script to calculate the number of source files in
        # the library
        set(options INTERFACE STATIC SHARED MODULE EXCLUDE_FROM_ALL OBJECT ALIAS)
        set(one_value_keywords PCH_FILE)
        set(multi_value_keywords)
        cmake_parse_arguments(LOCAL "${options}" "${one_value_keywords}" "" ${ARGV} )

        list(LENGTH LOCAL_UNPARSED_ARGUMENTS source_count)
        list(GET REAL_UNPARSED_ARGUMENTS 0 NEW_LIBRARY_TARGET)

        if (NOT LOCAL_PCH_FILE)
            krita_select_pch_file(${NEW_LIBRARY_TARGET} LOCAL_PCH_FILE)
            #file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/out/${NEW_LIBRARY_TARGET}.txt" CONTENT ${LOCAL_PCH_FILE})
        endif()

        # if the number of sources in the library is greater
        # than a threshold value, then we can try to use PCH
        # for this library

        if (${source_count} GREATER "2" AND
            NOT LOCAL_INTERFACE AND
            NOT LOCAL_ALIAS AND
            LOCAL_PCH_FILE)

            set_property(TARGET ${NEW_LIBRARY_TARGET} PROPERTY PCH_WARN_INVALID TRUE )
            target_precompile_headers(${NEW_LIBRARY_TARGET} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch/${LOCAL_PCH_FILE}>")
        endif()
    endif()
endmacro()

# overcome some platform incompatibilities
if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/winquirks)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
endif()

# set custom krita plugin installdir
if (ANDROID)
    # use default ABI
    if (NOT DEFINED ANDROID_ABI)
        set (ANDROID_ABI arm64-v8a)
    endif()
    if (NOT DEFINED ANDROID_SDK_ROOT)
        message (FATAL_ERROR "ANDROID_SDK_ROOT argument is not set")
    endif()

    set (KRITA_PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR})
    # set (DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/assets)

    # From ECM
    # Because Qt changes library suffix (to include arch), this changes name of
    # libraries that depend on Qt, but its Find file may not necessarily be
    # adapted for this.
    set(CMAKE_FIND_LIBRARY_SUFFIXES "_${CMAKE_ANDROID_ARCH_ABI}.so" ".so" ".a")
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
    set (Boost_COMPILER clang)
    find_package(unwindstack REQUIRED)
endif()

find_package(PNG REQUIRED "1.2.6")

if (APPLE)
    # this is not added correctly on OSX -- see https://forum.kde.org/viewtopic.php?f=139&t=101867&p=221242#p221242
    include_directories(SYSTEM ${PNG_INCLUDE_DIR})
endif()

if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1"
        KRITA_boost_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    set(Boost_COMPILER "mgw${KRITA_boost_COMPILER_VERSION}")
endif()

find_package(Boost 1.69 REQUIRED)
set_package_properties(Boost PROPERTIES
    DESCRIPTION "Boost provides free peer-reviewed portable C++ source libraries."
    URL "https://www.boost.org"
    TYPE REQUIRED)
target_link_libraries(Boost::boost
    INTERFACE
        Boost::disable_autolinking
)

# Disallow usage of std::unary_function. 
# See https://github.com/boostorg/container_hash/issues/22
# and https://releases.llvm.org/15.0.0/projects/libcxx/docs/ReleaseNotes.html#deprecations-and-removals
if(Boost_VERSION VERSION_LESS "1.81.0")
target_compile_definitions(Boost::boost
    INTERFACE
        BOOST_NO_CXX98_FUNCTION_BASE
)
endif()

find_package(Immer REQUIRED)
find_package(Zug REQUIRED)
find_package(Lager REQUIRED)

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

 ###########################
############################
## Optional dependencies  ##
############################
###########################


find_package(WebP 1.2.0 COMPONENTS demux mux)
set_package_properties(WebP PROPERTIES
    URL "https://developers.google.com/speed/webp"
    TYPE OPTIONAL
    PURPOSE "Required by the WebP plugin"
)

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

find_package(ZLIB REQUIRED)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "Compression library"
    URL "https://www.zlib.net/"
    TYPE REQUIRED
    PURPOSE "Required by Krita's PNG and PSD support")
macro_bool_to_01(ZLIB_FOUND HAVE_ZLIB)

find_package(OpenEXR)
macro_bool_to_01(OpenEXR_FOUND HAVE_OPENEXR)
if(OpenEXR_FOUND)
    set(LINK_OPENEXR_LIB OpenEXR::IlmImf)
endif()

find_package(TIFF)
set_package_properties(TIFF PROPERTIES
    DESCRIPTION "TIFF Library and Utilities"
    URL "http://www.libtiff.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita TIFF filter")
include(CheckLibTIFFPSDSupport)
configure_file(config-tiff.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-tiff.h)

find_package(JPEG)
set_package_properties(JPEG PROPERTIES
    DESCRIPTION "Free library for JPEG image compression. Note that libjpeg8 is NOT supported."
    URL "https://www.libjpeg-turbo.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita JPEG filter")
if (JPEG_FOUND)
    macro_bool_to_01(JPEG_FOUND HAVE_JPEG)
endif()

find_package(libjpeg-turbo 2.1.3 COMPONENTS turbojpeg)
set_package_properties(libjpeg-turbo PROPERTIES
    DESCRIPTION "libjpeg-turbo is a JPEG image codec that uses SIMD instructions (MMX, SSE2, AVX2, Neon, AltiVec) to accelerate baseline JPEG compression and decompression on x86, x86-64, Arm, and PowerPC systems, as well as progressive JPEG compression on x86 and x86-64 systems."
    URL "https://www.libjpeg-turbo.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita JPEG and TIFF filters")
macro_bool_to_01(libjpeg-turbo_FOUND HAVE_JPEG_TURBO)
configure_file(config-jpeg.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-jpeg.h )

if(libjpeg-turbo_FOUND)
    set(JPEG_TURBO_LIBRARIES libjpeg-turbo::turbojpeg)
endif()

find_package(GIF)
set_package_properties(GIF PROPERTIES
    DESCRIPTION "Library for loading and saving gif files."
    URL "http://giflib.sourceforge.net/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita GIF filter")

find_package(HEIF "1.11.0")
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

find_package(JPEGXL 0.7.0)
set_package_properties(JPEGXL PROPERTIES
    DESCRIPTION "JPEG XL is a royalty-free raster-graphics file format that supports both lossy and lossless compression and is experimentally supported by Chrome, Firefox, and Edge."
    URL "https://github.com/libjxl/libjxl"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita JPEG-XL filter")

find_package(FFTW3)
set_package_properties(FFTW3 PROPERTIES
    DESCRIPTION "A fast, free C FFT library"
    URL "http://www.fftw.org/"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita for fast convolution operators and some G'Mic features")
macro_bool_to_01(FFTW3_FOUND HAVE_FFTW3)
if (FFTW3_FOUND)
    # GMic uses the Threads library if available.
    find_library(FFTW3_THREADS_LIB fftw3_threads PATHS ${FFTW3_LIBRARY_DIRS})
endif()

find_package(OpenColorIO 1.1.1)
set_package_properties(OpenColorIO PROPERTIES
    DESCRIPTION "The OpenColorIO Library"
    URL "https://www.opencolorio.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita LUT docker")
macro_bool_to_01(OpenColorIO_FOUND HAVE_OCIO)
if (OPENCOLORIO_VERSION VERSION_GREATER_EQUAL "2.0.0")
    set(HAVE_OCIO_V2 TRUE)
endif()
configure_file(config-ocio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocio.h)

set_package_properties(PythonLibrary PROPERTIES
    DESCRIPTION "Python Library"
    URL "https://www.python.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(Python_Development_FOUND HAVE_PYTHONLIBS)

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

find_package(Mlt7)
set_package_properties(Mlt7 PROPERTIES DESCRIPTION "Media Lovin' Toolkit (Multimedia Framework)"
                URL "https://mltframework.org/"
                PURPOSE "Required to do audio/video syncrhonization and processing. No MLT = No Audio! ")
if (Mlt7_FOUND)
    find_package(SDL2 REQUIRED)
    set_package_properties(SDL2 PROPERTIES DESCRIPTION "Simple DirectMedia Layer 2"
                URL "https://www.libsdl.org/"
                PURPOSE "Required for MLT to render audio buffers. Required **only** when MLT is found on the system." )
endif()
macro_bool_to_01(Mlt7_FOUND HAVE_MLT)
configure_file(config-mlt.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-mlt.h)

find_package(LibMyPaint 1.4.0)
set_package_properties(LibMyPaint PROPERTIES
    DESCRIPTION "MyPaint brush engine API for C/C++"
    TYPE OPTIONAL
    PURPOSE "Required for the MyPaint brush engine")
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
find_package(Eigen3 5 NO_MODULE)
if(NOT Eigen3_FOUND)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
endif()
set_package_properties(Eigen3 PROPERTIES
    DESCRIPTION "C++ template library for linear algebra"
    URL "http://eigen.tuxfamily.org"
    TYPE REQUIRED)

##
## Test for exiv2
##
find_package(LibExiv2 0.16 REQUIRED)

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

##
## Test for xsimd
##
## xsimd has `SameMajorVersion` compatibility option set
## in `write_basic_package_version_file`, so we cannot just
## write a version range in the find_package call. Instead we
## should try all the supported versions manually
##
foreach(xsimd_version 8.1.0 9 10 11 12 13)
    if(NOT xsimd_FOUND)
        find_package(xsimd ${xsimd_version})
    endif()
endforeach()

set_package_properties(xsimd PROPERTIES
    DESCRIPTION "C++ wrappers for SIMD intrinsics"
    URL "https://github.com/xtensor-stack/xsimd"
    TYPE OPTIONAL
    PURPOSE "Required by Krita for vectorization")
macro_bool_to_01(xsimd_FOUND HAVE_XSIMD)
configure_file(config-xsimd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xsimd.h )

if(HAVE_XSIMD)
    macro(ko_compile_for_all_implementations_no_scalar _objs _src)
        if("aarch64" IN_LIST XSIMD_ARCH)
            xsimd_compile_for_all_implementations(${_objs} ${_src} FLAGS ${xsimd_ARCHITECTURE_FLAGS} ONLY NEON64)
        endif()

        if ("arm" IN_LIST XSIMD_ARCH)
            xsimd_compile_for_all_implementations(${_objs} ${_src} FLAGS ${xsimd_ARCHITECTURE_FLAGS} ONLY NEON)
        endif()

        if ("x86" IN_LIST XSIMD_ARCH OR "x86-64" IN_LIST XSIMD_ARCH)
            xsimd_compile_for_all_implementations(${_objs} ${_src} FLAGS ${xsimd_ARCHITECTURE_FLAGS} ONLY SSE2 SSSE3 SSE4_1 AVX AVX2+FMA)
        endif()
    endmacro()

    macro(ko_compile_for_all_implementations _objs _src)
        xsimd_compile_for_all_implementations(${_objs} ${_src} FLAGS ${xsimd_ARCHITECTURE_FLAGS} ONLY Scalar)
        ko_compile_for_all_implementations_no_scalar(${_objs} ${_src})
    endmacro()
endif()

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
find_package(QUAZIP 0.6)
set_package_properties(QUAZIP PROPERTIES
    DESCRIPTION "A library for reading and writing zip files"
    URL "https://stachenov.github.io/quazip/"
    TYPE REQUIRED
    PURPOSE "Needed for reading and writing KRA and ORA files"
)

##
## Test for KDcraw
##
find_package(KF5KDcraw 5.0.0)
set_package_properties(KF5KDcraw PROPERTIES
    DESCRIPTION "A thread-safe wrapper around libraw"
    URL "https://api.kde.org/libkdcraw/html/index.html"
    TYPE OPTIONAL
    PURPOSE "Needed for reading RAW files"
)

##
## Check for freetype
##
find_package(Freetype 2.11.0 REQUIRED)
set_package_properties(Freetype PROPERTIES
    DESCRIPTION "A library for rendering glyphs"
    URL "https://download.savannah.gnu.org/releases/freetype"
    TYPE REQUIRED
    PURPOSE "Needed for rendering text vector shapes."
)

##
## Check for HarfBuzz
##
find_package(HarfBuzz 4.0.0 REQUIRED)
set_package_properties(HarfBuzz PROPERTIES
    DESCRIPTION "OpenType text shaping engine"
    URL "https://harfbuzz.github.io"
    TYPE REQUIRED
    PURPOSE "Needed for rendering text vector shapes."
)
if (HarfBuzz_FOUND)
    list(APPEND ANDROID_EXTRA_LIBS ${HarfBuzz_LIBRARIES})
endif()

# Note: We now build a patched raqm from source with Krita.

##
## Check for fontconfig
##
find_package(Fontconfig 2.13.1 REQUIRED)
set_package_properties(Fontconfig PROPERTIES
    TYPE REQUIRED
    PURPOSE "Needed for getting font file information."
)

##
## Check for libunibreak
##
find_package(libunibreak 5.0 REQUIRED)
set_package_properties(libunibreak PROPERTIES
    DESCRIPTION "Implementation of the line breaking and word breaking algorithms as described in Unicode Standard Annex 14 and Unicode Standard Annex 29"
    URL "https://github.com/adah1972/libunibreak"
    TYPE REQUIRED
    PURPOSE "Needed for rendering text vector shapes."
)

##
## Build vendored libraries
##

# Allow specification of a directory with pre-downloaded
# requirements. This download directory is used for 3rdparty_vendor
# download

if(DEFINED ENV{EXTERNALS_DOWNLOAD_DIR})
    set(EXTERNALS_DOWNLOAD_DIR $ENV{EXTERNALS_DOWNLOAD_DIR})
endif()

if(NOT IS_DIRECTORY ${EXTERNALS_DOWNLOAD_DIR})
    message(WARNING "No externals download dir set, default location inside build tree is used. Use -DEXTERNALS_DOWNLOAD_DIR to override")
else()
    file(TO_CMAKE_PATH "${EXTERNALS_DOWNLOAD_DIR}" EXTERNALS_DOWNLOAD_DIR)
endif()

add_subdirectory(3rdparty_vendor)


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/po)
    find_package(KF5I18n CONFIG REQUIRED)
    if (NOT DISABLE_PO_INSTALLATION)
        ki18n_install(po)
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
)

add_subdirectory(sdk/tests)

add_subdirectory(libs)
add_subdirectory(plugins)
if (BUILD_TESTING)
    add_subdirectory(benchmarks)
endif()
add_subdirectory(krita)

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )
configure_file(KoTestConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoTestConfig.h )
configure_file(config_convolution.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_convolution.h)

check_function_exists(powf HAVE_POWF)
configure_file(config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/packaging/windows/installer/ConfigureInstallerNsis.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/packaging/windows/msix/ConfigureMsixFiles.cmake)
endif()

message("\nBroken tests:")
foreach(tst ${KRITA_BROKEN_TESTS})
    message("    * ${tst}")
endforeach()

if(APPLE)
    # fix macos test rpath
    macos_test_fixrpath(${KRITA_TESTS_TARGET})
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (ANDROID)
  # ECM passes this to Qt, which was already capable of detecting it..
  set (ANDROID_SDK_COMPILE_API "35")

  # To assist stdcpp-path in deployment.json file
  set(ANDROID_SYSROOT_PREFIX "${ANDROID_TOOLCHAIN_ROOT}/sysroot/usr" CACHE STRING "" FORCE)

  # copied regex from ECM's toolchain
  string(REGEX REPLACE "-(clang)?([0-9].[0-9])?$" "" ECM_ANDROID_STL_ARCH ${ANDROID_TOOLCHAIN_NAME})

  if (NOT (CMAKE_CXX_STANDARD_LIBRARIES MATCHES "[^ ]*c\\+\\+[^ ]*\\.so"))

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
