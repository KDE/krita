project(koffice)

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

 ############
#############
## Options ##
#############
############

 ######################
#######################
## Constants defines ##
#######################
######################

# define the generic version of the KOffice libraries here
# this makes it easy to advance it when the next KOffice release comes
set(GENERIC_KOFFICE_LIB_VERSION "5.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "5")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

#
# WARNING: don't change the SHOULD_BUILD_app if an application
# doesn't build, but rather add the following line:
# option(BUILD_app "Build directory app" OFF)
#
#eg:
#option(BUILD_kexi "Build directory kexi" OFF) # kexi isn't buildable at the moment

option(TINY "compile a tiny KOffice" OFF)

IF (TINY)
    add_definitions("-DTINY_OFFICE")
    set(SHOULD_BUILD_KWORD TRUE)
    set(SHOULD_BUILD_KPRESENTER TRUE)
    set(SHOULD_BUILD_KSPREAD TRUE)
    set(SHOULD_BUILD_PIGMENT FALSE)
    set(SHOULD_BUILD_KARBON FALSE)
    set(SHOULD_BUILD_KRITA FALSE)
    set(SHOULD_BUILD_KEXI FALSE)
    set(SHOULD_BUILD_KIVIO FALSE)
    set(SHOULD_BUILD_KPLATO FALSE)
    set(SHOULD_BUILD_KFORMULA FALSE)
    set(SHOULD_BUILD_KCHART TRUE)
    set(SHOULD_BUILD_KDGANTT FALSE)
    set(SHOULD_BUILD_KOUNAVAIL FALSE)
ELSE(TINY)
    set(SHOULD_BUILD_KWORD TRUE)
    set(SHOULD_BUILD_KPRESENTER TRUE)
    set(SHOULD_BUILD_KSPREAD TRUE)
    set(SHOULD_BUILD_PIGMENT TRUE)
    set(SHOULD_BUILD_KARBON TRUE)
    set(SHOULD_BUILD_KRITA TRUE)
    set(SHOULD_BUILD_KEXI TRUE)
    set(SHOULD_BUILD_KIVIO TRUE)
    set(SHOULD_BUILD_KPLATO TRUE)
    set(SHOULD_BUILD_KFORMULA TRUE)
    set(SHOULD_BUILD_KCHART TRUE)
    set(SHOULD_BUILD_KDGANTT TRUE)
    set(SHOULD_BUILD_KOUNAVAIL TRUE)
ENDIF(TINY)

if(KDE4_BUILD_TESTS)
    # only with this definition will the FOO_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################
set(KDE_MIN_VERSION "4.1.0")
find_package(KDE4 4.1.0 REQUIRED)
find_package(Qt4 4.5.0 REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)
include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)

 ###########################
############################
## Required dependencies  ##
############################
###########################

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

 ################################
#################################
## Check wich pigment to build ##
#################################
################################

IF(SHOULD_BUILD_PIGMENT)
    SET(NO_PIGMENT FALSE)

    macro_optional_find_package(OpenEXR)
    macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)
    configure_file(config-openexr.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h )
    install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

    include_directories(${CMAKE_SOURCE_DIR}/libs/pigment)
    find_package(LCMS REQUIRED)
    set(REQUIRED_LCMS_VERSION 118)

    if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
        # FULL_PIGMENT is the most desirable state, we can compile Krita!
        set(HAVE_REQUIRED_LCMS_VERSION TRUE)
        set(BUILD_pigment TRUE)
    else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
        message( FATAL_ERROR "Old LCMS version found, upgrade to 1.18.")
        set(BUILD_pigment FALSE)
    endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})

    macro_log_feature(HAVE_REQUIRED_LCMS_VERSION "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "1.18" "Required by for color management and for Krita")

ELSE(SHOULD_BUILD_PIGMENT)
    SET(NO_PIGMENT TRUE)
    add_definitions("-DNO_PIGMENT")
ENDIF(SHOULD_BUILD_PIGMENT)

 ###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Look for OpenGL
##

macro_optional_find_package(OpenGL)
set(HAVE_OPENGL 0)

if(OPENGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_LIBRARIES}")
    if(QT_QTOPENGL_FOUND)
        message(STATUS "Found Qt OpenGL support")
        set(HAVE_OPENGL 1)
    else(QT_QTOPENGL_FOUND)
        message(STATUS "Did NOT find Qt OpenGL support. Check your Qt configuration")
    endif(QT_QTOPENGL_FOUND)
else(OPENGL_FOUND)
    message(STATUS "Did NOT find OpenGL libraries")
endif(OPENGL_FOUND)

configure_file(config-opengl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-opengl.h )
macro_log_feature(HAVE_OPENGL "OpenGL" "OpenGL support" "" FALSE "" "Required by parts of Krita and optionally by flake")

##
## Test for create ressources
##
macro_optional_find_package(CreateResources)
macro_log_feature(CreateResources_FOUND "Create Resources" "Create Resources (brushes, palettes etc.)" "http://create.freedesktop.org" FALSE "" "Required by KOffice for optional extra resources.")

##
## Test for kdepimlibs
##
macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "KDE PIMLibs" "KDE Personal Information Management Libraries" "http://www.kde.org/" FALSE "" "Required by KPlato and the KDE address book integration (available as a module in KDE)")

##
## Test for boost
##
macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org" FALSE "" "Required by KPresenter")

##
## Test for eigen2
##
macro_optional_find_package(Eigen2)
macro_log_feature(EIGEN2_FOUND "Eigen" "C++ template library for linear algebra" "http://eigen.tuxfamily.org" FALSE "2.0" "Required by KSpread and Krita")

##
## Test for blitz
##
macro_optional_find_package(Blitz)
macro_log_feature(BLITZ_FOUND "QImageBlitz" "Image effect library" "http://www.kde.org" FALSE "" "Required by Krita (available in kdesupport)")

##
## Test for QCA2
##
macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0" "Required for encrypted OpenDocument files support (available as a module in kdesupport)")

##
## Test for exiv2
##
set(EXIV2_MIN_VERSION "0.16")
macro_optional_find_package(Exiv2)
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata library and tools" "http://www.exiv2.org" FALSE "0.16" "Required by Krita")

##
## Test for bzip2
##
find_program(BZIP2_EXECUTABLE NAMES bzip2 )
macro_log_feature(BZIP2_EXECUTABLE "bzip2" "High-quality data compressor" "http://www.bzip.org" FALSE "" "Required by Kexi")

#Set the build of TextShape changetraker

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})

if(WIN32)
    # detect oxygen icon dir at configure time based on KDEDIRS - there may be different package installation locations
    execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path icon OUTPUT_VARIABLE _dir ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    file(TO_CMAKE_PATH "${_dir}" __dir)
    find_path(KDE4_ICON_DIR oxygen PATHS
        ${__dir}
    )
    message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")

    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )


else(WIN32)
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif(WIN32)

##
## Test endianess
##
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
configure_file(config-endian.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-endian.h )

##
## Generate a file for prefix information
##
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )

 ####################
#####################
## Define includes ##
#####################
####################

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# koaction depends on threadweaver
set(KOACTION_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koaction ${CMAKE_BINARY_DIR}/libs/koaction ${KDE4_INCLUDES})

# kostore is at the bottom of stack, so it has the dependency on the kde4 includes.
set(KOSTORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/store ${CMAKE_BINARY_DIR}/libs/store ${KDE4_INCLUDES})

# kopicture depends on kostore
set(KOPICTURE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopicture ${CMAKE_BINARY_DIR}/libs/kopicture ${KOSTORE_INCLUDES})

# koodf depends on kostore and kopicture
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf ${CMAKE_BINARY_DIR}/libs/odf ${KOPICTURE_INCLUDES})

# flake depends on koodf
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake 
                   ${KOODF_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/kobase 
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands 
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools 
                   ${CMAKE_BINARY_DIR}/libs/flake)

if (NOT NO_PIGMENT)
# pigment depends on koplugin and lcms
    set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koplugin 
                     ${CMAKE_SOURCE_DIR}/libs/pigment 
                     ${CMAKE_SOURCE_DIR}/libs/pigment/compositeops 
                     ${LCMS_INCLUDE_DIR} 
                     ${KDE4_INCLUDES} )
endif (NOT NO_PIGMENT)

# komain depends on kostore, koplugin, flake and koodf
set(KOMAIN_INCLUDES ${KDE4_INCLUDES}
                    ${FLAKE_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/koproperties
                    ${CMAKE_SOURCE_DIR}/libs/store
                    ${CMAKE_SOURCE_DIR}/libs/kotext
                    ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                    ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                    ${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen
                    ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/widgets/progress
                    ${CMAKE_SOURCE_DIR}/libs/widgets/linestyleselector
                    ${CMAKE_SOURCE_DIR}/libs/widgets/aspectbutton
                    ${CMAKE_SOURCE_DIR}/libs/widgets/openpane
                    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
                    ${CMAKE_SOURCE_DIR}/libs/widgets/resources
                    ${CMAKE_SOURCE_DIR}/libs/widgets/pagelayout
                    ${CMAKE_SOURCE_DIR}/libs/widgets/characterselectiondialog
                    ${CMAKE_SOURCE_DIR}/libs/widgets/csvimportdialog
                    ${CMAKE_SOURCE_DIR}/libs/widgets/documentsectionbox
                    ${CMAKE_SOURCE_DIR}/libs/widgets/zoom
                    ${CMAKE_SOURCE_DIR}/libs/widgets/input
                    ${CMAKE_SOURCE_DIR}/libs/kopicture
                    ${CMAKE_SOURCE_DIR}/libs/koaction
                    ${CMAKE_SOURCE_DIR}/libs/main
                    ${CMAKE_SOURCE_DIR}/libs/main/config
                    ${CMAKE_SOURCE_DIR}/libs/resources
                    ${CMAKE_SOURCE_DIR}/libs/koplugin
 )

if(BUILD_pigment)
    set(KOMAIN_INCLUDES ${KOMAIN_INCLUDES}
                    ${PIGMENT_INCLUDES}
    )
endif(BUILD_pigment)

# koresources depends on pigment
set(KORESOURCES_INCLUDES ${CMAKE_SOURCE_DIR}/libs/resources
 			 ${CMAKE_BINARY_DIR}/libs/resources 
 			 ${PIGMENT_INCLUDES} 
			 ${KDE4_INCLUDES})

# KoText depends on store, odf
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                    ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                    ${CMAKE_SOURCE_DIR}/libs/kotext
                    ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                    ${CMAKE_BINARY_DIR}/libs/kotext)

set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)

# kopageapp
set(KOPAGEAPP_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

set(KOPROPERTIES_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kobase ${CMAKE_BINARY_DIR}/libs/kobase)

 #############################################
##############################################
## Detect which applications can be compile ##
##############################################
#############################################

#
# WARNING: when adding a dependency if there is allready a line
# containing SHOULD_BUILD_app, don't add a new one, just add
# the test to the if construct
#

### Karbon: pigment
if (NOT BUILD_pigment)
    set(SHOULD_BUILD_KARBON FALSE)
endif (NOT BUILD_pigment)

### KSpread: eigen2
if(NOT EIGEN2_FOUND)
    set(SHOULD_BUILD_KSPREAD FALSE)
endif(NOT EIGEN2_FOUND)

### KPresenter: boost
if(NOT Boost_FOUND)
    set(SHOULD_BUILD_KPRESENTER FALSE)
endif(NOT Boost_FOUND)

### Kexi: bzip2
if(NOT BZIP2_EXECUTABLE )
    set(SHOULD_BUILD_KEXI FALSE)
endif(NOT BZIP2_EXECUTABLE )

### Krita: pigment, blitz, eigen2, exiv2
if(NOT BUILD_pigment OR NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)
   set(SHOULD_BUILD_KRITA FALSE)
endif(NOT BUILD_pigment OR NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)

### KPlato: kdepimlibs
if(NOT KdepimLibs_FOUND)
    set(SHOULD_BUILD_KPLATO FALSE)
endif(NOT KdepimLibs_FOUND)

 ###################
####################
## Subdirectories ##
####################
###################

add_subdirectory(libs)
add_subdirectory(interfaces)
add_subdirectory(cmake)

if(SHOULD_BUILD_KWORD)
    macro_optional_add_subdirectory(kword)
endif(SHOULD_BUILD_KWORD)

if (SHOULD_BUILD_KFORMULA)
    macro_optional_add_subdirectory(kformula)
endif (SHOULD_BUILD_KFORMULA)

if (SHOULD_BUILD_KIVIO)
    macro_optional_add_subdirectory(kivio)
endif (SHOULD_BUILD_KIVIO)

if (SHOULD_BUILD_KOUNAVAIL)
    macro_optional_add_subdirectory(kounavail)
endif (SHOULD_BUILD_KOUNAVAIL)    

if (SHOULD_BUILD_KPLATO)
    macro_optional_add_subdirectory(kdgantt)
endif (SHOULD_BUILD_KPLATO)

if (SHOULD_BUILD_KCHART)
    macro_optional_add_subdirectory(kchart)
endif (SHOULD_BUILD_KCHART)

if(SHOULD_BUILD_KPRESENTER)
  macro_optional_add_subdirectory(kpresenter)
endif(SHOULD_BUILD_KPRESENTER)

if(SHOULD_BUILD_KEXI)
    macro_optional_add_subdirectory(kexi)
endif(SHOULD_BUILD_KEXI)

if(SHOULD_BUILD_KPLATO)
    macro_optional_add_subdirectory(kplato)
endif(SHOULD_BUILD_KPLATO)

if(SHOULD_BUILD_KSPREAD)
    macro_optional_add_subdirectory(kspread)
endif(SHOULD_BUILD_KSPREAD)

if(SHOULD_BUILD_KRITA)
  macro_optional_add_subdirectory(krita)
endif(SHOULD_BUILD_KRITA)

if(SHOULD_BUILD_KARBON)
  macro_optional_add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON)

macro_optional_add_subdirectory(doc)

add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)

macro_display_feature_log()

macro(macro_display_application_enabled _appName _shouldBuild)
if( BUILD_${_appName} AND ${_shouldBuild} MATCHES "[Tt][Rr][Uu][Ee]" )
  message(STATUS ${_appName} )
endif( BUILD_${_appName} AND ${_shouldBuild} MATCHES "[Tt][Rr][Uu][Ee]" )
endmacro(macro_display_application_enabled)

message(STATUS "---------------- The following applications will be built ----------------" )

macro_display_application_enabled(kword ${SHOULD_BUILD_KWORD})
macro_display_application_enabled(kspread ${SHOULD_BUILD_KSPREAD})
macro_display_application_enabled(kpresenter ${SHOULD_BUILD_KPRESENTER})
macro_display_application_enabled(kivio ${SHOULD_BUILD_KIVIO})
macro_display_application_enabled(kexi ${SHOULD_BUILD_KEXI})
macro_display_application_enabled(kplato ${SHOULD_BUILD_KPLATO})
macro_display_application_enabled(krita ${SHOULD_BUILD_KRITA})
macro_display_application_enabled(karbon ${SHOULD_BUILD_KARBON})
macro_display_application_enabled(kformula ${SHOULD_BUILD_KFORMULA})
macro_display_application_enabled(kchart ${SHOULD_BUILD_KCHART})

message(STATUS "--------------------------------------------------------------------------" )

add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
