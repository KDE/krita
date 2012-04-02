project(calligra)

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

# define the generic version of the Calligra libraries here
# this makes it easy to advance it when the next Calligra release comes
set(GENERIC_CALLIGRA_LIB_VERSION "9.0.0")
set(GENERIC_CALLIGRA_LIB_SOVERSION "9")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

#
# WARNING: don't change the SHOULD_BUILD_app if an application
# doesn't build, but rather add the following line:
# option(BUILD_app "Build directory app" OFF)
#
#eg:
#option(BUILD_kexi "Build directory kexi" OFF) # kexi isn't buildable at the moment
option(BUILD_koabstraction "Build directory koabstraction" ON) # koabstraction isn't buildable at the moment
option(BUILD_mobile "Build directory mobile" ON) # mobile depends on koabstraction it builds but does not run

option(GHNS "support Get Hot New Stuff" ON)
option(NEPOMUK "support NEPOMUK Tagging" ON)
option(TINY "compile a tiny Calligra" OFF)
option(CREATIVEONLY "compile only Karbon and Krita" OFF)
option(QT3SUPPORT "Build the parts of Calligra that still depend on Qt3" ON)

IF (TINY)
    set(SHOULD_BUILD_WORDS TRUE)
    set(SHOULD_BUILD_STAGE TRUE)
    set(SHOULD_BUILD_SHEETS TRUE)
    set(SHOULD_BUILD_KARBON FALSE)
    set(SHOULD_BUILD_KRITA FALSE)
    set(SHOULD_BUILD_KEXI FALSE)
    set(SHOULD_BUILD_FLOW FALSE)
    set(SHOULD_BUILD_PLAN FALSE)
    set(SHOULD_BUILD_KDGANTT FALSE)
    set(SHOULD_BUILD_KOUNAVAIL FALSE)
    set(SHOULD_BUILD_SCRIPTING FALSE)
    set(SHOULD_BUILD_MOBILE TRUE)
    set(SHOULD_BUILD_KOREPORT FALSE)
    set(SHOULD_BUILD_BRAINDUMP FALSE)
    set(SHOULD_BUILD_CALLIGRA FALSE)
    set(SHOULD_BUILD_CSTESTER FALSE)
    set(SHOULD_BUILD_KOABSTRACTION TRUE)
    set(SHOULD_BUILD_ACTIVE TRUE)
ELSEIF (CREATIVEONLY)
    set(SHOULD_BUILD_WORDS FALSE)
    set(SHOULD_BUILD_STAGE FALSE)
    set(SHOULD_BUILD_SHEETS FALSE)
    set(SHOULD_BUILD_KARBON TRUE)
    set(SHOULD_BUILD_KRITA TRUE)
    set(SHOULD_BUILD_KEXI FALSE)
    set(SHOULD_BUILD_FLOW FALSE)
    set(SHOULD_BUILD_PLAN FALSE)
    set(SHOULD_BUILD_KDGANTT FALSE)
    set(SHOULD_BUILD_KOUNAVAIL FALSE)
    set(SHOULD_BUILD_SCRIPTING TRUE)
    set(SHOULD_BUILD_MOBILE FALSE)
    set(SHOULD_BUILD_KOREPORT FALSE)
    set(SHOULD_BUILD_BRAINDUMP FALSE)
    set(SHOULD_BUILD_CALLIGRA FALSE)
    set(SHOULD_BUILD_CSTESTER FALSE)
    set(SHOULD_BUILD_KOABSTRACTION FALSE)
    set(SHOULD_BUILD_ACTIVE FALSE)
ELSE()
    set(SHOULD_BUILD_WORDS TRUE)
    set(SHOULD_BUILD_STAGE TRUE)
    set(SHOULD_BUILD_SHEETS TRUE)
    set(SHOULD_BUILD_KARBON TRUE)
    set(SHOULD_BUILD_KRITA TRUE)
    set(SHOULD_BUILD_KEXI TRUE)
    set(SHOULD_BUILD_FLOW TRUE)
    set(SHOULD_BUILD_PLAN TRUE)
    set(SHOULD_BUILD_KDGANTT TRUE)
    set(SHOULD_BUILD_KOUNAVAIL TRUE)
    set(SHOULD_BUILD_SCRIPTING TRUE)
    set(SHOULD_BUILD_MOBILE TRUE)
    set(SHOULD_BUILD_KOREPORT TRUE)
    set(SHOULD_BUILD_BRAINDUMP TRUE)
    set(SHOULD_BUILD_CALLIGRA TRUE)
    set(SHOULD_BUILD_CSTESTER TRUE)
    set(SHOULD_BUILD_KOABSTRACTION TRUE)
    set(SHOULD_BUILD_ACTIVE TRUE)
ENDIF()

if(KDE4_BUILD_TESTS)
    # only with this definition will the FOO_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################
set(KDE_MIN_VERSION "4.3.0")
find_package(KDE4 4.3.0 REQUIRED)

if (QT3SUPPORT)
    find_package(Qt4 4.6.0 REQUIRED QtCore QtGui QtXml QtScript QtSvg QtTest QtWebKit QtDBus QtOpenGL Qt3Support)
else (QT3SUPPORT)
    find_package(Qt4 4.6.0 REQUIRED QtCore QtGui QtXml QtScript QtSvg QtTest QtWebKit QtDBus QtOpenGL)
endif (QT3SUPPORT)

include(KDE4Defaults)
include(MacroLibrary)
include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)

set(SHOULD_BUILD_TEXTSHAPE TRUE)
if ((${QTVERSION} VERSION_EQUAL 4.8.0) OR (${QTVERSION} VERSION_EQUAL 4.8.1))
    if (NOT IHAVEPATCHEDQT)
        set(SHOULD_BUILD_WORDS FALSE)
        set(SHOULD_BUILD_STAGE FALSE)
        set(SHOULD_BUILD_TEXTSHAPE FALSE)
        MESSAGE(STATUS "WARNING: You are using a version of Qt that causes crashes. As a result Words and Stage will not be built. Please upgrade to 4.8.2. You can also patch Qt and when building Calligra set IHAVEPATCHEDQT. Patch against Qt can be found in qt48setx.patch")
    endif (NOT IHAVEPATCHEDQT)
endif ((${QTVERSION} VERSION_EQUAL 4.8.0) OR (${QTVERSION} VERSION_EQUAL 4.8.1))

if(NOT ${KDE_VERSION} VERSION_GREATER 4.4.0)
    set(GHNS FALSE)
endif(NOT ${KDE_VERSION} VERSION_GREATER 4.4.0)

macro_optional_find_package(LibAttica)
macro_log_feature(LIBATTICA_FOUND "LibAttica" "Attica is used for Get Hot New Stuff." "https://projects.kde.org/projects/kdesupport/attica" FALSE "" "You need at least version 3.0 for uploading of resources to work.")

if (NOT LIBATTICA_FOUND)
    set(GHNS FALSE)
endif (NOT LIBATTICA_FOUND)

if(NOT ${KDE_VERSION} VERSION_GREATER 4.6.4)
  set(NEPOMUK FALSE)
endif(NOT ${KDE_VERSION} VERSION_GREATER 4.6.4)

macro_ensure_out_of_source_build("Compiling Calligra inside the source directory is not possible. Please refer to the build instruction http://community.kde.org/Calligra/Building/Building_Calligra")

 ###########################
############################
## Required dependencies  ##
############################
###########################

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Boost REQUIRED) # for pigment and stage
if (NOT Boost_FOUND)
    MESSAGE(FATAL_ERROR "Did not find Boost. Boost is required for the core libraries, stage, sheets and krita.")
endif (NOT Boost_FOUND)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

 ################################
#################################
## Check for OpenEXR           ##
#################################
################################


macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)


 ###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Look for OpenGL
##
set(HAVE_OPENGL 0)
if (NOT WIN32)
macro_optional_find_package(OpenGL)

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

macro_log_feature(HAVE_OPENGL "OpenGL" "OpenGL support" "" FALSE "" "Required by parts of Krita and optionally by flake")
endif (NOT WIN32)

##
## Test for kdepimlibs
##
macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "KDE PIMLibs" "KDE Personal Information Management Libraries" "http://www.kde.org/" FALSE "" "Required by Plan and the KDE address book integration (available as a module in KDE)")


##
## Test for eigen2
##
macro_optional_find_package(Eigen2)
macro_log_feature(EIGEN2_FOUND "Eigen" "C++ template library for linear algebra" "http://eigen.tuxfamily.org" FALSE "2.0" "Required by Tables and Krita")

##
## Test for QCA2
##
macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0" "Required for encrypted OpenDocument files and encrypted xls files support (available as a module in kdesupport)")

##
## Test for exiv2
##
set(EXIV2_MIN_VERSION "0.16")
macro_optional_find_package(Exiv2)
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata library and tools" "http://www.exiv2.org" FALSE "0.16" "Required by Krita")

##
## Test for soprano
##
macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "KDE4 RDF handling library" "http://soprano.sourceforge.net/" FALSE "" "Required to handle RDF metadata in ODF")
if(NOT Soprano_FOUND)
    set(SHOULD_BUILD_RDF FALSE)
    set(SOPRANO_INCLUDE_DIR "")
else(NOT Soprano_FOUND)
    set(SHOULD_BUILD_RDF TRUE)
endif(NOT Soprano_FOUND)
if( Soprano_FOUND )
   add_definitions( -DSHOULD_BUILD_RDF )
endif( Soprano_FOUND )


##
## Test for marble
##
macro_optional_find_package(Marble)
macro_log_feature(MARBLE_FOUND "Marble" "KDE4 World Globe Widget library" "http://techbase.kde.org/Projects/Marble/" FALSE "" "Required by RDF, Kexi Forms and Reports to show locations on a map")
if(NOT MARBLE_FOUND)
    set(CAN_USE_MARBLE FALSE)
    set(MARBLE_INCLUDE_DIR "")
else(NOT MARBLE_FOUND)
    set(CAN_USE_MARBLE TRUE)
endif(NOT MARBLE_FOUND)
if( MARBLE_FOUND )
    add_definitions( -DCAN_USE_MARBLE )
    ##
    ## Marble changed addMarbleWidget to setMarbleWidget in MarbleControlBox.h
    ## with commit ea177ca. This is for compatibility with older versions.
    ##
    find_file(MARBLECONTROLBOX_H MarbleControlBox.h ${MARBLE_INCLUDE_DIR})
    if( MARBLECONTROLBOX_H )
	file(READ ${MARBLECONTROLBOX_H} MARBLECONTROLBOX_H_CONTENT)
	string(REGEX MATCH "setMarbleWidget" SETMARBLEWIDGET ${MARBLECONTROLBOX_H_CONTENT})
        if( SETMARBLEWIDGET )
            add_definitions(-DHAVE_SETMARBLEWIDGET)
        endif( SETMARBLEWIDGET)
    endif( MARBLECONTROLBOX_H )
endif( MARBLE_FOUND )



##
## Test for lcms
##
macro_optional_find_package(LCMS)
set(REQUIRED_LCMS_VERSION 118)
if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS TRUE)
else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(HAVE_REQUIRED_LCMS_VERSION FALSE)
    set(HAVE_LCMS FALSE)
endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})

macro_optional_find_package(LCMS2)
macro_log_feature(LCMS2_FOUND "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "2.0" "Optional replacement for LittleCMS 1.18. Can be used for color management and Krita")
if(LCMS2_FOUND)
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS2 TRUE)
    set(HAVE_LCMS TRUE)
endif(LCMS2_FOUND)

##
## Test for Nepomuk
##
if(NEPOMUK)
    macro_optional_find_package(Nepomuk)
    if(NOT NEPOMUK_FOUND)
        set(NEPOMUK FALSE)
    endif(NOT NEPOMUK_FOUND)
    add_definitions(-DDISABLE_NEPOMUK_LEGACY)
    macro_log_feature(NEPOMUK_FOUND "Nepomuk" "Handles metadata of resources" FALSE  "Required to handle metadata of resources")
endif(NEPOMUK)


macro_log_feature(HAVE_REQUIRED_LCMS_VERSION "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "1.18" "Required for color management and Krita")

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
## Test for Fontconfig
##
macro_optional_find_package(Fontconfig)
macro_log_feature(FONTCONFIG_FOUND "Fontconfig" "Library for configuring and customizing font access" "http://fontconfig.org" FALSE "" "Required to handle exact font size")

##
## Test for Freetype
##
macro_optional_find_package(Freetype)
macro_log_feature(FREETYPE_FOUND "Freetype" "A Free, High-Quality, and Portable Font Engine" "http://www.freetype.org/" FALSE "" "Required to handle exact font size")

if(NOT FONTCONFIG_FOUND OR NOT FREETYPE_FOUND) 
    set(FONTCONFIG_INCLUDE_DIR "")
    set(FREETYPE_INCLUDE_DIRS "")
else(NOT FONTCONFIG_FOUND OR NOT FREETYPE_FOUND)
	add_definitions( -DSHOULD_BUILD_FONT_CONVERSION )
endif(NOT FONTCONFIG_FOUND OR NOT FREETYPE_FOUND)
  
##
## Test for Qt Webkit
##
macro_log_feature(QT_QTWEBKIT_FOUND "Qt Webkit" "Qt binding for Webkit, the HTML engine." "http://qt.nokia.com" FALSE "" "Required for the web shape, web Kexi widget and web report element")
if(QT_QTWEBKIT_FOUND)
    add_definitions( -DCAN_USE_QTWEBKIT )
endif(QT_QTWEBKIT_FOUND)

##
## QImageBlitz (for Kexi)
##
#macro_optional_find_package(QImageBlitz)
#macro_log_feature(QIMAGEBLITZ_FOUND "QImageBlitz" "Fast, smooth scaling of QImages" "sourceforge.net/projects/qimageblitz/" FALSE "" "Required for Kexi")

##
## Test endianess
##
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

##
## Generate a file for prefix information
##

 #############################
##############################
## Add the benchmark target ##
##############################
#############################

include(MacroCalligraAddBenchmark)

 ####################
#####################
## Define includes ##
#####################
####################

# WARNING: make sure that QT_INCLUDES is the first directory to be added to include_directory before
#          any other include directory

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces )

# koplugin is at the bottom of the stack
set(KOPLUGIN_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koplugin)

# koodf is at the bottom of the stack
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf
                   ${CMAKE_BINARY_DIR}/libs/odf
                   ${KDE4_INCLUDES})

set(KUNDO2_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kundo2)

# pigment depends on koplugin and lcms
set(PIGMENT_INCLUDES ${KOPLUGIN_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/pigment
                   ${CMAKE_SOURCE_DIR}/libs/pigment/compositeops
                   ${CMAKE_SOURCE_DIR}/libs/pigment/resources
                   ${QT_INCLUDES}
                   ${KDE4_INCLUDES})

# flake depends on koodf and pigment
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake
                   ${KOODF_INCLUDES}
                   ${KOPLUGIN_INCLUDES}
                   ${PIGMENT_INCLUDES}
                   ${KUNDO2_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools
                   ${CMAKE_SOURCE_DIR}/libs/flake/svg
                   ${CMAKE_BINARY_DIR}/libs/flake)

# KoText depends on koplugin, odf
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext
                   ${CMAKE_BINARY_DIR}/libs/kotext
                   ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                   ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                   ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                   ${SOPRANO_INCLUDE_DIR}
                   ${FLAKE_INCLUDES}
                   ${KOODF_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/main)

# TextLayout depends on kotext
set(TEXTLAYOUT_INCLUDES ${KOTEXT_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/textlayout)

# komain depends on kotext & flake
set(KOMAIN_INCLUDES ${KDE4_INCLUDES}
                    ${TEXTLAYOUT_INCLUDES}
                    ${MARBLE_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/main
                    ${CMAKE_SOURCE_DIR}/libs/main/rdf
                    ${CMAKE_SOURCE_DIR}/libs/main/config)

if(SHOULD_BUILD_SCRIPTING)
  set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)
endif(SHOULD_BUILD_SCRIPTING)

# kopageapp
set(KOPAGEAPP_INCLUDES ${TEXTLAYOUT_INCLUDES}
                    ${PIGMENT_INCLUDES}
                    ${KOMAIN_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

# koproperty
SET(KOPROPERTY_TARGETNAME koproperty)                           # TODO remove when we move to independent place
SET(KOPROPERTY_LIBS ${KOPROPERTY_TARGETNAME})                   # TODO remove when we move to independent place
SET(KOPROPERTY_LIB_VERSION ${GENERIC_CALLIGRA_LIB_VERSION})      # TODO remove when we move to independent place
SET(KOPROPERTY_LIB_SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION})  # TODO remove when we move to independent place
SET(KOPROPERTY_USE_KOLIBS TRUE)                                 # TODO remove when we move to independent place
set(KOPROPERTY_INCLUDES ${KOODF_INCLUDES}
                        ${CMAKE_SOURCE_DIR}/libs
                        ${CMAKE_SOURCE_DIR}/libs/widgets)

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


### Tables: eigen2, boost
if(NOT EIGEN2_FOUND)
    set(SHOULD_BUILD_SHEETS FALSE)
endif(NOT EIGEN2_FOUND)

### Krita: pigment, eigen2, exiv2
if(NOT EIGEN2_FOUND OR NOT EXIV2_FOUND OR NOT HAVE_REQUIRED_LCMS_VERSION)
   set(SHOULD_BUILD_KRITA FALSE)
endif(NOT EIGEN2_FOUND OR NOT EXIV2_FOUND OR NOT HAVE_REQUIRED_LCMS_VERSION)

### Active needs sheets, stage and words
if (NOT SHOULD_BUILD_SHEETS OR NOT SHOULD_BUILD_STAGE OR NOT SHOULD_BUILD_WORDS)
    set(SHOULD_BUILD_ACTIVE FALSE)
endif (NOT SHOULD_BUILD_SHEETS OR NOT SHOULD_BUILD_STAGE OR NOT SHOULD_BUILD_WORDS)


### Active: only build with Qt 4.7.0 or better 
if (NOT ${QTVERSION} VERSION_GREATER 4.7.0)
    set(SHOULD_BUILD_ACTIVE FALSE)
endif (NOT ${QTVERSION} VERSION_GREATER 4.7.0)

if (NOT QT_QTDECLARATIVE_FOUND)
    set(SHOULD_BUILD_ACTIVE FALSE)
endif (NOT QT_QTDECLARATIVE_FOUND)

if(${KDE_VERSION} VERSION_LESS 4.8.0)
    set(SHOULD_BUILD_ACTIVE FALSE)
endif(${KDE_VERSION} VERSION_LESS 4.8.0)

#if(NOT QIMAGEBLITZ_FOUND)
#    set (SHOULD_BUILD_KEXI FALSE)
#endif(NOT QIMAGEBLITZ_FOUND)

if (NOT QT3SUPPORT)
    set (SHOULD_BUILD_KEXI FALSE)
endif (NOT QT3SUPPORT)

### HACK ###
# Those BUILD_ are illegally used below before they are defined, which cause problem on a first build
option(BUILD_kexi   "Build directory kexi" ON)
option(BUILD_koreport "Build directory libs/koreport" ON)
### HACK ###

if(BUILD_kexi AND SHOULD_BUILD_KEXI)
    ##
    ## Test for sqlite
    ##
    set(SQLITE_MIN_VERSION 3.6.16)
    set(SQLITE_RECOMMENDED_VERSION 3.7.10)
    set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
    find_package(CalligraSqlite 3.6.16)
    macro_log_feature(SQLITE_FOUND "SQLite" "Embedded SQL database engine with enabled extensions loading"
                      "http://www.sqlite.org" FALSE "${SQLITE_MIN_VERSION}"
                      "Required by Kexi for its default database driver.")
    if(NOT SQLITE_FOUND)
        set(SHOULD_BUILD_KEXI FALSE)
        message(STATUS "WARNING: Because of that, Kexi will not be built.")
    endif(NOT SQLITE_FOUND)

    ##
    ## Test for ICU
    ##
    find_package(ICU)
    macro_log_feature(ICU_FOUND "ICU" "International Components for Unicode (ICU) Library"
                      "http://icu-project.org" FALSE "" "Required by Kexi")

    if(NOT ICU_FOUND)
        set(SHOULD_BUILD_KEXI FALSE)
        message(STATUS "WARNING: Because of that, Kexi will not be built.")
    endif(NOT ICU_FOUND)
endif(BUILD_kexi AND SHOULD_BUILD_KEXI)

 ###################
####################
## Subdirectories ##
####################
###################

if(SHOULD_BUILD_WORDS)
   macro_optional_add_subdirectory(words)
endif(SHOULD_BUILD_WORDS)

if (SHOULD_BUILD_FLOW)
    macro_optional_add_subdirectory(flow)
endif (SHOULD_BUILD_FLOW)

if (SHOULD_BUILD_KOUNAVAIL)
    macro_optional_add_subdirectory(kounavail)
endif (SHOULD_BUILD_KOUNAVAIL)

if (SHOULD_BUILD_PLAN)
    macro_optional_add_subdirectory(kdgantt)
endif (SHOULD_BUILD_PLAN)

if(SHOULD_BUILD_STAGE)
  macro_optional_add_subdirectory(stage)
endif(SHOULD_BUILD_STAGE)

if(SHOULD_BUILD_KEXI)
    macro_optional_add_subdirectory(kexi)
endif(SHOULD_BUILD_KEXI)

if(SHOULD_BUILD_PLAN)
    macro_optional_add_subdirectory(plan)
endif(SHOULD_BUILD_PLAN)

if(SHOULD_BUILD_SHEETS)
    macro_optional_add_subdirectory(sheets)
endif(SHOULD_BUILD_SHEETS)

if(SHOULD_BUILD_KRITA)
  macro_optional_add_subdirectory(krita)
endif(SHOULD_BUILD_KRITA)

if(SHOULD_BUILD_KARBON)
  macro_optional_add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON)

if(SHOULD_BUILD_BRAINDUMP)
  macro_optional_add_subdirectory(braindump)
endif(SHOULD_BUILD_BRAINDUMP)

if(SHOULD_BUILD_ACTIVE)
  macro_optional_add_subdirectory(active)
endif(SHOULD_BUILD_ACTIVE)


if(   NOT SHOULD_BUILD_SHEETS OR NOT BUILD_sheets
   OR NOT SHOULD_BUILD_WORDS OR NOT BUILD_words
   OR NOT SHOULD_BUILD_STAGE OR NOT BUILD_stage
  )
    set(SHOULD_BUILD_KOABSTRACTION FALSE)
endif(   NOT SHOULD_BUILD_SHEETS OR NOT BUILD_sheets
      OR NOT SHOULD_BUILD_WORDS OR NOT BUILD_words
      OR NOT SHOULD_BUILD_STAGE OR NOT BUILD_stage
     )

if(SHOULD_BUILD_KOABSTRACTION)
    # koabstraction depends on kotext & flake
    set(KOABSTRACTION_INCLUDES ${KDE4_INCLUDES}
                        ${KOTEXT_INCLUDES}
                        ${FLAKE_INCLUDES}
                        ${KOPAGEAPP_INCLUDES}
                        ${KOMAIN_INCLUDES}
                        ${CMAKE_SOURCE_DIR}/tools/koabstraction)
else(SHOULD_BUILD_KOABSTRACTION)
    set(SHOULD_BUILD_MOBILE FALSE)
endif(SHOULD_BUILD_KOABSTRACTION)

if(SHOULD_BUILD_SHEETS AND SHOULD_BUILD_WORDS)
    # build cstester
else(SHOULD_BUILD_SHEETS AND SHOULD_BUILD_WORDS)
    set(SHOULD_BUILD_CSTESTER FALSE)
endif(SHOULD_BUILD_SHEETS AND SHOULD_BUILD_WORDS)

macro_optional_add_subdirectory(doc)

# non-app directories are moved here because they can depend on SHOULD_BUILD_{appname} variables set above
add_subdirectory(libs)
add_subdirectory(interfaces)
add_subdirectory(cmake)
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

macro_display_application_enabled(words ${SHOULD_BUILD_WORDS})
macro_display_application_enabled(sheets ${SHOULD_BUILD_SHEETS})
macro_display_application_enabled(stage ${SHOULD_BUILD_STAGE})
macro_display_application_enabled(flow ${SHOULD_BUILD_FLOW})
macro_display_application_enabled(kexi ${SHOULD_BUILD_KEXI})
macro_display_application_enabled(plan ${SHOULD_BUILD_PLAN})
macro_display_application_enabled(krita ${SHOULD_BUILD_KRITA})
macro_display_application_enabled(karbon ${SHOULD_BUILD_KARBON})
macro_display_application_enabled(braindump ${SHOULD_BUILD_BRAINDUMP})
macro_display_application_enabled(mobile ${SHOULD_BUILD_MOBILE})
macro_display_application_enabled(calligra ${SHOULD_BUILD_CALLIGRA})
macro_display_application_enabled(cstester ${SHOULD_BUILD_CSTESTER})
macro_display_application_enabled(active ${SHOULD_BUILD_ACTIVE})

message(STATUS "--------------------------------------------------------------------------" )

add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
