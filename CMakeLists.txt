project(krita)

message(STATUS "Using CMake version: ${CMAKE_VERSION}")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

cmake_policy(SET CMP0002 OLD)
#cmake_policy CMP0017 was introduced in version 2.8.4
if (NOT ${CMAKE_VERSION} VERSION_LESS 2.8.3)
    cmake_policy(SET CMP0017 NEW)
endif ()

if (NOT ${CMAKE_VERSION} VERSION_LESS 2.8.12)
    cmake_policy(SET CMP0022 OLD)
endif ()

if (NOT ${CMAKE_VERSION} VERSION_LESS 3.0.0)
    cmake_policy(SET CMP0026 OLD) 
    cmake_policy(SET CMP0046 OLD)
endif ()

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.3.0)
    cmake_policy(SET CMP0059 OLD)
    cmake_policy(SET CMP0063 OLD)
endif()

# QT5TODO: remove KDE4_BUILD_TESTS once all kde4_add_unit_test have been converted
# transitional forward compatibility:
# BUILD_TESTING is cmake standard, KDE4_BUILD_TESTS not used by ECM/KF5, but only
# macros in cmake/transitional. Just, Macros from cmake/transitional,
# incl. kde4_add_unit_test, are only picked up if no macros from kdelibs4 are installed,
# because that transitional path is appended. Prepending instead might possibly unwantedly
# mask ECM/KF5 macros. Not tested.
# So have BUILD_TESTING define KDE4_BUILD_TESTS.
if (BUILD_TESTING)
    set(KDE4_BUILD_TESTS TRUE)
else()
    set(KDE4_BUILD_TESTS FALSE)
endif()

 ######################
#######################
## Constants defines ##
#######################
######################

# define common versions of Calligra applications, used to generate calligraversion.h
# update these version for every release:
set(CALLIGRA_VERSION_STRING "3.0 Alpha")
set(CALLIGRA_STABLE_VERSION_MAJOR 3) # 3 for 3.x, 4 for 4.x, etc.
set(CALLIGRA_STABLE_VERSION_MINOR 0) # 0 for 3.0, 1 for 3.1, etc.
set(CALLIGRA_VERSION_RELEASE 89)     # 89 for Alpha, increase for next test releases, set 0 for first Stable, etc.
set(CALLIGRA_ALPHA 1) # uncomment only for Alpha
#set(CALLIGRA_BETA 1) # uncomment only for Beta
#set(CALLIGRA_RC 1) # uncomment only for RC
set(CALLIGRA_YEAR 2015) # update every year

if(NOT DEFINED CALLIGRA_ALPHA AND NOT DEFINED CALLIGRA_BETA AND NOT DEFINED CALLIGRA_RC)
    set(CALLIGRA_STABLE 1) # do not edit
endif()

message(STATUS "Calligra version: ${CALLIGRA_VERSION_STRING}")

# Define the generic version of the Calligra libraries here
# This makes it easy to advance it when the next Calligra release comes.
# 14 was the last GENERIC_CALLIGRA_LIB_VERSION_MAJOR of the previous Calligra series
# (2.x) so we're starting with 15 in 3.x series.
if(CALLIGRA_STABLE_VERSION_MAJOR EQUAL 3)
    math(EXPR GENERIC_CALLIGRA_LIB_VERSION_MAJOR "${CALLIGRA_STABLE_VERSION_MINOR} + 15")
else()
    # let's make sure we won't forget to update the "15"
    message(FATAL_ERROR "Reminder: please update offset == 15 used to compute GENERIC_CALLIGRA_LIB_VERSION_MAJOR to something bigger")
endif()
set(GENERIC_CALLIGRA_LIB_VERSION "${GENERIC_CALLIGRA_LIB_VERSION_MAJOR}.0.0")
set(GENERIC_CALLIGRA_LIB_SOVERSION "${GENERIC_CALLIGRA_LIB_VERSION_MAJOR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      "${CMAKE_SOURCE_DIR}/cmake/modules")
LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/transitional")

message("Module path:" ${CMAKE_MODULE_PATH})

# fetch git revision for the current build

set(CALLIGRA_GIT_SHA1_STRING "")
set(CALLIGRA_GIT_BRANCH_STRING "")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
get_git_branch(GIT_BRANCH)

if(GIT_SHA1 AND GIT_BRANCH)
    string(SUBSTRING ${GIT_SHA1} 0 7 GIT_SHA1)
    set(CALLIGRA_GIT_SHA1_STRING ${GIT_SHA1})
    set(CALLIGRA_GIT_BRANCH_STRING ${GIT_BRANCH})
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

 ############
#############
## Options ##
#############
############

option(USEOPENGL "Allow the use of OpenGL for Krita" ON)
if (WIN32)
    option(USE_BREAKPAD "Build the crash handler for Krita (only on windows)" OFF)
endif ()
option(GHNS "support Get Hot New Stuff" OFF)

option(PACKAGERS_BUILD "Build support of multiple CPU architectures in one binary. Should be used by packagers only." ON)

 #######################
########################
## Productset setting ##
########################
#######################

# For predefined productsets see the definitions in CalligraProducts.cmake and
# in the files in the folder cmake/productsets.

# Finding out the products & features to build is done in 5 steps:
# 1. have the user define the products/features wanted, by giving a productset
# 2. estimate all additional required products/features
# 3. estimate which of the products/features can be build by external deps
# 4. find which products/features have been temporarily disabled due to problems
# 5. estimate which of the products/features can be build by internal deps

# get the special macros
include(CalligraProductSetMacros)
include(MacroJPEG)

# get the definitions of products, features and product sets
include(CalligraProducts.cmake)

set(PRODUCTSET_DEFAULT "ALL")
# temporary migration support
if (CREATIVEONLY)
    set(WARN_ABOUT_CREATIVEONLY TRUE)
    set(PRODUCTSET_DEFAULT "CREATIVE")
endif ()

if(NOT PRODUCTSET)
    set(PRODUCTSET ${PRODUCTSET_DEFAULT} CACHE STRING "Set of products/features to build" FORCE)
endif()

if (RELEASE_BUILD)
    set(CALLIGRA_SHOULD_BUILD_STAGING FALSE)
else ()
    set(CALLIGRA_SHOULD_BUILD_STAGING TRUE)
endif ()

# finally choose products/features to build
calligra_set_productset(${PRODUCTSET})

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################

find_package(ECM 1.7.0 REQUIRED NOMODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(GenerateExportHeader)
include(ECMMarkAsTest)

include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(KF5 5.7.0 REQUIRED COMPONENTS 
        Archive 
        Codecs 
        Completion
        Config 
        ConfigWidgets 
        CoreAddons
        Emoticons 
        GlobalAccel
        GuiAddons 
        I18n 
        IconThemes 
        ItemModels 
        ItemViews
        KDELibs4Support
        KIO 
        Kross 
        Parts 
        Sonnet
        TextEditor 
        TextWidgets 
        ThreadWeaver
        Wallet 
        WidgetsAddons 
        WindowSystem
        XmlGui 
    OPTIONAL_COMPONENTS
        NotifyConfig
        Activities
        KHtml
)
if(KF5_Activities_FOUND)
    set(HAVE_KACTIVITIES TRUE)
endif()

find_package(Qt5 5.3.0 REQUIRED COMPONENTS Core Gui Widgets Xml Network PrintSupport
                                           Script Svg Test Concurrent)
find_package(Qt5 5.3.0 COMPONENTS UiTools Sql WebKit WebKitWidgets DBus Declarative Qml Quick X11Extras)

set(QT_QTDBUS_FOUND ${Qt5DBus_FOUND})
set(QT_QTWEBKIT_FOUND ${Qt5WebKit_FOUND})
set(QT_QTDECLARATIVE_FOUND ${Qt5Declarative_FOUND})
set(QT_QTQUICK_FOUND ${Qt5Quick_FOUND})

set(QT_QTTEST_LIBRARY Qt5::Test)

if (USEOPENGL)
    find_package(Qt5 5.3.0 REQUIRED OpenGL)
    set(QT_QTOPENGL_FOUND ${Qt5OpenGL_FOUND})
endif ()


include (MacroLibrary)
include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)
include (ECMInstallIcons)

if (GHNS)
    find_package(Attica)
    find_package(NewStuff)
    macro_log_feature(LIBATTICA_FOUND "LibAttica" "Attica is used for Get Hot New Stuff." "https://projects.kde.org/projects/kdesupport/attica" FALSE "" "You need at least version 3.0 for uploading of resources to work.")

    if (NOT LIBATTICA_FOUND)
        set(GHNS FALSE)
    else ()
        message(STATUS "WARNING: You are compiling with Get Hot New Stuff enabled. Do not do that when building distribution packages. GHNS is unusable these days until someone starts maintaining it again.")
    endif ()
endif ()

macro_ensure_out_of_source_build("Compiling Calligra inside the source directory is not possible. Please refer to the build instruction http://community.kde.org/Calligra/Building/Building_Calligra")

find_package(X11)
if(X11_FOUND)
    find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS
                 X11Extras
    )
    set(HAVE_X11 TRUE)
    add_definitions(-DHAVE_X11)
else()
    set(HAVE_X11 FALSE)
endif()

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_STRICT_ITERATORS
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_USE_FAST_CONCATENATION
  -DQT_NO_URL_CAST_FROM_STRING
)

# only with this definition will all the FOO_TEST_EXPORT macro do something
# TODO: check if this can be moved to only those places which make use of it,
# to reduce global compiler definitions that would trigger a recompile of
# everything on a change (like adding/removing tests to/from the build)
if(BUILD_TESTING)
    add_definitions(-DCOMPILING_TESTS)
endif()

# overcome some platform incompatibilities
if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/winquirks)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    set(WIN32_PLATFORM_NET_LIBS ws2_32.lib netapi32.lib)
endif()

# set custom calligra plugin installdir
set(CALLIGRA_PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR}/calligra)

# TEMPORARY: for initial Qt5/KF5 build porting phase deprecation warnings are only annoying noise
# remove once code porting phase starts, perhaps first locally in product subdirs
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-deprecated -Wno-deprecated-declarations)
endif ()

 ###########################
############################
## Required dependencies  ##
############################
###########################

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
if (APPLE)
    # this is not added correctly on OSX -- see http://forum.kde.org/viewtopic.php?f=139&t=101867&p=221242#p221242
    include_directories(${PNG_INCLUDE_DIR})
endif()

add_definitions(-DBOOST_ALL_NO_LIB)
find_package(Boost REQUIRED COMPONENTS system) # for pigment and stage
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Did not find Boost. Boost is required for the core libraries, stage, sheets and krita.")
endif ()

if (APPLE)
   find_package(Carbon REQUIRED)
endif ()

 ###########################
############################
## Optional dependencies  ##
############################
###########################


##
## Check for OpenEXR
##

macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)

##
## Look for OpenGL
##
set(HAVE_OPENGL 0)
if (USEOPENGL)
    macro_optional_find_package(OpenGL)

    if(OPENGL_FOUND)
        message(STATUS "Found OpenGL: ${OPENGL_LIBRARIES}")
        if(QT_QTOPENGL_FOUND)
            message(STATUS "Found Qt OpenGL support")
            set(HAVE_OPENGL 1)
        else()
            message(STATUS "Did NOT find Qt OpenGL support. Check your Qt configuration")
        endif()
    else()
        message(STATUS "Did NOT find OpenGL libraries")
    endif()
    macro_log_feature(HAVE_OPENGL "OpenGL" "OpenGL support" "" FALSE "" "Required by Gemini, parts of Krita and optionally by flake")
endif()

##
## Test for GNU Scientific Library
##
macro_optional_find_package(GSL)
macro_log_feature(GSL_FOUND "GSL" "GNU Scientific Library" "http://www.gnu.org/software/gsl"
                  FALSE "1.7" "Required by Krita's Transform tool and Sheets' solver plugin")
macro_bool_to_01(GSL_FOUND HAVE_GSL)
configure_file(config-gsl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-gsl.h )

##
## Test for eigen3
##
macro_optional_find_package(Eigen3)
macro_log_feature(EIGEN3_FOUND "Eigen" "C++ template library for linear algebra" "http://eigen.tuxfamily.org" FALSE "3.0" "Required by Calligra Sheets and Krita")

##
## Test for exiv2
##
set(EXIV2_MIN_VERSION "0.16")
macro_optional_find_package(Exiv2)
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata library and tools" "http://www.exiv2.org" FALSE "0.16" "Required by Krita")

##
## Test for lcms
##
macro_optional_find_package(LCMS2)
macro_log_feature(LCMS2_FOUND "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "2.4" "Will be used for color management and is necesary for Krita")
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
macro_optional_find_package(Vc 0.6.70)
macro_log_feature(Vc_FOUND "Vc" "Portable, zero-overhead SIMD library for C++" "http://code.compeng.uni-frankfurt.de/projects/vc" FALSE "" "Required by the Krita for vectorization")
macro_bool_to_01(Vc_FOUND HAVE_VC)
macro_bool_to_01(PACKAGERS_BUILD DO_PACKAGERS_BUILD)

configure_file(config-vc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-vc.h )

if(HAVE_VC)
    message(STATUS "Vc found!")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                        "${CMAKE_SOURCE_DIR}/cmake/vc")

    include (VcMacros)

    if(Vc_COMPILER_IS_CLANG)
        set(ADDITIONAL_VC_FLAGS "-Wabi -ffp-contract=fast")
    else()
        set(ADDITIONAL_VC_FLAGS "-Wabi -fabi-version=0 -ffp-contract=fast")
    endif()

    #Handle Vc master
    if(Vc_VERSION_MAJOR GREATER 0 OR Vc_VERSION_MINOR GREATER 7)
        message(STATUS "Vc version is greater than 0.7, enabling AVX2 support")

        if(Vc_COMPILER_IS_GCC OR Vc_COMPILER_IS_CLANG)
            AddCompilerFlag("-std=c++11" _ok)
            if(NOT _ok)
                AddCompilerFlag("-std=c++0x" _ok)
            endif()
        endif()

        macro(ko_compile_for_all_implementations_no_scalar _objs _src)
        if(PACKAGERS_BUILD)
            vc_compile_for_all_implementations(${_objs} ${_src} FLAGS ${ADDITIONAL_VC_FLAGS} -fPIC ONLY SSE2 SSSE3 SSE4_1 AVX AVX2)
        else()
            set(${_objs} ${_src})
        endif()
        endmacro()

        macro(ko_compile_for_all_implementations _objs _src)
        if(PACKAGERS_BUILD)
            vc_compile_for_all_implementations(${_objs} ${_src} FLAGS ${ADDITIONAL_VC_FLAGS} -fPIC ONLY Scalar SSE2 SSSE3 SSE4_1 AVX AVX2)
        else()
            set(${_objs} ${_src})
        endif()
        endmacro()
    else()
        macro(ko_compile_for_all_implementations_no_scalar _objs _src)
        if(PACKAGERS_BUILD)
            vc_compile_for_all_implementations(${_objs} ${_src} FLAGS ${ADDITIONAL_VC_FLAGS} -fPIC ONLY SSE2 SSSE3 SSE4_1 AVX)
        else()
            set(${_objs} ${_src})
        endif()
        endmacro()

        macro(ko_compile_for_all_implementations _objs _src)
        if(PACKAGERS_BUILD)
            vc_compile_for_all_implementations(${_objs} ${_src} FLAGS ${ADDITIONAL_VC_FLAGS} -fPIC ONLY Scalar SSE2 SSSE3 SSE4_1 AVX)
        else()
            set(${_objs} ${_src})
        endif()
        endmacro()
    endif()

    if (NOT PACKAGERS_BUILD)
      # Optimize the whole Calligra for current architecture
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Vc_DEFINITIONS}")
    endif ()
endif()
set(CMAKE_MODULE_PATH ${OLD_CMAKE_MODULE_PATH} )

##
## Test for Xinput
##
if(NOT WIN32 AND NOT APPLE)
  set(REQUIRED_Xinput_FOUND ${X11_Xinput_FOUND})
else()
  set(REQUIRED_Xinput_FOUND TRUE)
endif()

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})

if(WIN32)
    set(KDE4_ICON_DIR ${KDE_INSTALL_FULL_ICONDIR})
    message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")

    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )


else()
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif()

##
## Test endianess
##
include (TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

##
## Test SharedMimeInfo
##
macro_optional_find_package(SharedMimeInfo)
macro_log_feature(SHARED_MIME_INFO_FOUND "SharedMimeInfo" "Shared Mime Info" "http://freedesktop.org/wiki/Software/shared-mime-info" FALSE "" "Required to determine file types OpenRaster (Krita default format).")

##
## Test for qt-poppler
##
macro_optional_find_package(Poppler)
macro_log_feature( POPPLER_FOUND "Poppler-Qt5" "A PDF rendering library" "http://poppler.freedesktop.org" FALSE "" "Required by the Krita PDF filter, Karbon PDF import filter and CSTester PDF feature")


##
## Generate a file for prefix information
##

 ###############################
################################
## Add Calligra helper macros ##
################################
###############################

include(MacroCalligraAddBenchmark)
include(MacroCalligraAddTest)

 ####################
#####################
## Define includes ##
#####################
####################

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces )


set(KOVERSION_INCLUDES ${CMAKE_SOURCE_DIR}/libs/version 
                       ${CMAKE_BINARY_DIR}/libs/version 
)

include_directories(${KOVERSION_INCLUDES})

# koplugin is at the bottom of the stack
set(KOPLUGIN_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koplugin)

set(KUNDO2_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kundo2
                    ${CMAKE_BINARY_DIR}/libs/kundo2)

# koodf is at the bottom of the stack
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf
                   ${CMAKE_SOURCE_DIR}/libs/store
                   ${CMAKE_BINARY_DIR}/libs/odf
                   ${CMAKE_BINARY_DIR}/libs/store
                   ${KOVERSION_INCLUDES} 
                   )

# pigment depends on koplugin and lcms
set(PIGMENT_INCLUDES ${KOPLUGIN_INCLUDES}
                   ${KOVERSION_INCLUDES} 
                   ${CMAKE_SOURCE_DIR}/libs/pigment
                   ${CMAKE_BINARY_DIR}/libs/pigment
                   ${CMAKE_SOURCE_DIR}/libs/pigment/compositeops
                   ${CMAKE_SOURCE_DIR}/libs/pigment/resources
                   ${Boost_INCLUDE_DIRS}
                   
                   )

# flake depends on koodf and pigment
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake
                   ${KOODF_INCLUDES}
                   ${PIGMENT_INCLUDES}
                   ${KUNDO2_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/widgetutils
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools
                   ${CMAKE_SOURCE_DIR}/libs/flake/svg
                   ${CMAKE_BINARY_DIR}/libs/flake)

# vectorimage
set(VECTORIMAGE_INCLUDES
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage/libemf
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage/libsvm
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage/libwmf)

# KoText depends on koplugin, odf
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext
                   ${CMAKE_BINARY_DIR}/libs/kotext
                   ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                   ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                   ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                   ${FLAKE_INCLUDES}
                   ${KOODF_INCLUDES})

# TextLayout depends on kotext
set(TEXTLAYOUT_INCLUDES ${KOTEXT_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/textlayout
                   ${CMAKE_BINARY_DIR}/libs/textlayout)

# Widgets depends on kritatext and flake
set(KOWIDGETS_INCLUDES ${KOTEXT_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/widgetutils
                   ${CMAKE_BINARY_DIR}/libs/widgetutils
                   ${CMAKE_SOURCE_DIR}/libs/widgets
                   ${CMAKE_BINARY_DIR}/libs/widgets)

# BasicFlakes depends on flake, widgets
set(BASICFLAKES_INCLUDES ${KOWIDGETS_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/basicflakes
                   ${CMAKE_BINARY_DIR}/libs/basicflakes
                   ${CMAKE_SOURCE_DIR}/libs/basicflakes/tools)

 ###################################################
####################################################
## Detect which products/features can be compiled ##
####################################################
###################################################

if (NOT WIN32)
    set(NOT_WIN TRUE)
endif()
if (NOT QT_MAC_USE_COCOA)
    set(NOT_COCOA TRUE)
endif()

calligra_drop_product_on_bad_condition( APP_KRITA
    EIGEN3_FOUND "Eigen devel not found"
    EXIV2_FOUND "libexiv2 devel not found"
    HAVE_REQUIRED_LCMS_VERSION "lcms devel not found"
    SHARED_MIME_INFO_FOUND "SharedMimeInfo not found"
    Boost_SYSTEM_FOUND "boost-system devel not found"
    REQUIRED_Xinput_FOUND "Xinput devel not found "
)

#############################################
####  Backward compatibility BUILD_x=off ####
#############################################

# workaround: disable directly all products which might be activated by internal
# dependencies, but belong to scope of old flag
calligra_drop_products_on_old_flag(krita APP_KRITA)

#############################################
####      Temporarily broken products    ####
#############################################

# If a product does not build due to some temporary brokeness disable it here,
# by calling calligra_disable_product with the product id and the reason,
# e.g.:
# calligra_disable_product(APP_KEXI "isn't buildable at the moment")

#############################################
####     Calculate buildable products    ####
#############################################

calligra_drop_unbuildable_products()

 ###################
####################
## Subdirectories ##
####################
###################

if(SHOULD_BUILD_APP_KRITA)
  add_subdirectory(krita)
endif()

# non-app directories are moved here because they can depend on SHOULD_BUILD_{appname} variables set above
add_subdirectory(libs)
add_subdirectory(3rdparty)
add_subdirectory(interfaces)
add_subdirectory(cmake)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(devtools)

macro_display_feature_log()

calligra_product_deps_report("product_deps")
calligra_log_should_build()

add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )

## temporary warning
if (WARN_ABOUT_CREATIVEONLY)
    message(STATUS "WARNING:\n   You are using a deprecated build flag,\n   switch from \"-DCREATIVEONLY=ON\" to \"-DPRODUCTSET=CREATIVE\"\n   and remove the line \"CREATIVEONLY:BOOL=ON\" from CMakeCache.txt")
    message(STATUS "-------------------------------------------------------------------" )
endif ()
