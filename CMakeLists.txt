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
set(GENERIC_KOFFICE_LIB_VERSION "6.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "6")

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
    set(SHOULD_BUILD_KWORD TRUE)
    set(SHOULD_BUILD_KPRESENTER TRUE)
    set(SHOULD_BUILD_KSPREAD TRUE)
    set(SHOULD_BUILD_KARBON FALSE)
    set(SHOULD_BUILD_KRITA FALSE)
    set(SHOULD_BUILD_KEXI FALSE)
    set(SHOULD_BUILD_KIVIO FALSE)
    set(SHOULD_BUILD_KPLATO FALSE)
    set(SHOULD_BUILD_KFORMULA FALSE)
    set(SHOULD_BUILD_KCHART TRUE)
    set(SHOULD_BUILD_KDGANTT FALSE)
    set(SHOULD_BUILD_KOUNAVAIL FALSE)
    set(SHOULD_BUILD_SCRIPTING FALSE)
    set(SHOULD_BUILD_F_OFFICE TRUE)
ELSE(TINY)
    set(SHOULD_BUILD_KWORD TRUE)
    set(SHOULD_BUILD_KPRESENTER TRUE)
    set(SHOULD_BUILD_KSPREAD TRUE)
    set(SHOULD_BUILD_KARBON TRUE)
    set(SHOULD_BUILD_KRITA TRUE)
    set(SHOULD_BUILD_KEXI TRUE)
    set(SHOULD_BUILD_KIVIO TRUE)
    set(SHOULD_BUILD_KPLATO TRUE)
    set(SHOULD_BUILD_KFORMULA TRUE)
    set(SHOULD_BUILD_KCHART TRUE)
    set(SHOULD_BUILD_KDGANTT TRUE)
    set(SHOULD_BUILD_KOUNAVAIL TRUE)
    set(SHOULD_BUILD_SCRIPTING TRUE)
    set(SHOULD_BUILD_F_OFFICE TRUE)
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
find_package(KDE4 4.3.0 REQUIRED)
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
## Check for OpenEXR           ##
#################################
################################


macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)
configure_file(config-openexr.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h )
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)


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
macro_log_feature(CREATERESOURCES_FOUND "Create Resources" "Create Resources (brushes, palettes etc.)" "http://create.freedesktop.org" FALSE "" "Required by KOffice for optional extra resources.")

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

macro_log_feature(HAVE_REQUIRED_LCMS_VERSION "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "1.18" "Required for color management and Krita")

if(SHOULD_BUILD_KEXI)
    ##
    ## Test for sqlite
    ##
    set(SQLITE_RECOMMENDED_VERSION "3.6.22")
    find_package(KofficeSqlite 3.6.16)
    macro_log_feature(SQLITE_FOUND "SQLite" "Embedded SQL database engine" "http://www.sqlite.org"
                      FALSE ${SQLITE_MIN_VERSION} "Required by Kexi (for its default database driver).")
    ## Test for bzip2
    find_program(BZIP2_EXECUTABLE NAMES bzip2)
    macro_log_feature(BZIP2_EXECUTABLE "bzip2" "High-quality data compressor" "http://www.bzip.org"
                      FALSE "" "Required by Kexi.")

    if(NOT BZIP2_EXECUTABLE OR NOT SQLITE_FOUND)
        set(SHOULD_BUILD_KEXI FALSE)
        message(STATUS "WARNING: Kexi will not be built.")
    endif(NOT BZIP2_EXECUTABLE OR NOT SQLITE_FOUND)
endif(SHOULD_BUILD_KEXI)

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
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools
                   ${CMAKE_BINARY_DIR}/libs/flake)

# KoText depends on koplugin, odf
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext
                   ${CMAKE_BINARY_DIR}/libs/kotext
                   ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                   ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                   ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                   ${FLAKE_INCLUDES}
                   ${KOODF_INCLUDES})

# komain depends on kotext & flake
set(KOMAIN_INCLUDES ${KDE4_INCLUDES}
                    ${SOPRANO_INCLUDE_DIR}
                    ${KOTEXT_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/main
                    ${CMAKE_SOURCE_DIR}/libs/main/config)

if(SHOULD_BUILD_SCRIPTING)
  set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)
endif(SHOULD_BUILD_SCRIPTING)

# kopageapp
set(KOPAGEAPP_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

# koproperty
SET(KOPROPERTY_TARGETNAME koproperty)                           # TODO remove when we move to independent place
SET(KOPROPERTY_LIBS ${KOPROPERTY_TARGETNAME})                   # TODO remove when we move to independent place
SET(KOPROPERTY_LIB_VERSION ${GENERIC_KOFFICE_LIB_VERSION})      # TODO remove when we move to independent place
SET(KOPROPERTY_LIB_SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION})  # TODO remove when we move to independent place
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


### KSpread: eigen2
if(NOT EIGEN2_FOUND)
    set(SHOULD_BUILD_KSPREAD FALSE)
endif(NOT EIGEN2_FOUND)

### KPresenter: boost
if(NOT Boost_FOUND)
    set(SHOULD_BUILD_KPRESENTER FALSE)
endif(NOT Boost_FOUND)

### Krita: pigment, blitz, eigen2, exiv2
if(NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)
   set(SHOULD_BUILD_KRITA FALSE)
endif(NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)

### KPlato: kdepimlibs
if(NOT KdepimLibs_FOUND)
    set(SHOULD_BUILD_KPLATO FALSE)
endif(NOT KdepimLibs_FOUND)

if(NOT BUILD_kchart OR NOT SHOULD_BUILD_KCHART)
    set(SHOULD_BUILD_KPLATO FALSE)
    set(SHOULD_BUILD_KEXI FALSE)
endif(NOT BUILD_kchart OR NOT SHOULD_BUILD_KCHART)

 ###################
####################
## Subdirectories ##
####################
###################

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
    set (WITH_kchart true CACHE BOOL "" FORCE)
    add_definitions(-DHAVE_KCHART)
    macro_optional_add_subdirectory(kchart)
else (SHOULD_BUILD_KCHART)
    set (WITH_kchart false CACHE BOOL "" FORCE)
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
macro_display_application_enabled(f-office ${SHOULD_BUILD_F_OFFICE})

message(STATUS "--------------------------------------------------------------------------" )

add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
