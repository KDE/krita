project(koffice)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# define the generic version of the KOffice libraries here
# this makes it easy to advance it when the next KOffice release comes
set(GENERIC_KOFFICE_LIB_VERSION "5.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "5")

# search packages used by KDE
set(KDE_MIN_VERSION "4.1.0")
find_package(KDE4 4.1.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
macro_optional_find_package(Boost)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(LCMS REQUIRED)
find_package(PNG REQUIRED)
find_package(CreateResources)
message( STATUS "Create Resources will be required in the final release")
macro_optional_find_package(Eigen2)
macro_optional_find_package(Blitz)
macro_optional_find_package(QCA2)
macro_optional_find_package(Exif)
macro_optional_find_package(Freetype)
macro_optional_find_package(Fontconfig)

macro_log_feature(EIGEN2_FOUND "Eigen2" "Eigen2 is needed by KSpread and Krita. They won't be built." "Module in kdesupport" FALSE "2.0")

macro_log_feature(QCA2_FOUND "libqca2" "QCA2 is needed to compile support for encrypted OpenDocument files" "Module in kdesupport" FALSE "2.0" "" )

macro_log_feature(Boost_FOUND "libboost-dev" "Boost is needed by KPresenter. KPresenter will not be built" "http://www.boost.org" FALSE "")



set(REQUIRED_LCMS_VERSION 115)

if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(HAVE_REQUIRED_LCMS_VERSION FALSE)
endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})

macro_log_feature(HAVE_REQUIRED_LCMS_VERSION "lcms" "LittleCMS is required to build KOffice" "http://www.littlecms.com" TRUE "1.15")

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})


include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)

configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )

macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)
configure_file(config-openexr.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h )

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

# Look for OpenCTL
macro_optional_find_package(OpenCTL)

macro_log_feature(OPENCTL_FOUND "OpenCTL" "OpenCTL is needed for some color spaces (High Dynamic Range Color Spaces, YCbCr and LMS)" "http://www.openctl.org" FALSE "0.9.2")
configure_file(config-openctl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openctl.h )


include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
configure_file(config-endian.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-endian.h )

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${QDBUS_INCLUDE_DIRS}  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# kostore is at the bottom of stack, so it has the dependency on the kde4 includes.
set(KOSTORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/store ${CMAKE_BINARY_DIR}/libs/store ${KDE4_INCLUDES})
# koodf depends on kostore
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf ${CMAKE_BINARY_DIR}/libs/odf ${KOSTORE_INCLUDES})
# komain depends on koodf
set(KOMAIN_INCLUDES ${CMAKE_SOURCE_DIR}/libs/main ${CMAKE_SOURCE_DIR}/libs/main/threadAction ${CMAKE_BINARY_DIR}/libs/main ${KOODF_INCLUDES})
# pigment depends on komain and lcms
set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/pigment ${CMAKE_SOURCE_DIR}/libs/pigment/colorspaces ${CMAKE_SOURCE_DIR}/libs/pigment/colorprofiles ${LCMS_INCLUDE_DIR} )
# flake depends on komain
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake ${CMAKE_SOURCE_DIR}/libs/flake/commands ${CMAKE_SOURCE_DIR}/libs/flake/tools ${CMAKE_BINARY_DIR}/libs/flake)
# koresources depends on pigment
set(KORESOURCES_INCLUDES ${CMAKE_SOURCE_DIR}/libs/resources ${CMAKE_BINARY_DIR}/libs/resources ${PIGMENT_INCLUDES})
# koguiutils depends on komain, flake and pigment
set(KOGUIUTILS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/guiutils ${CMAKE_BINARY_DIR}/libs/guiutils ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${PIGMENT_INCLUDES} ${KORESOURCES_INCLUDES})

# The rest must be specified independently, in addition to one of the above three
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext/styles ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument ${CMAKE_SOURCE_DIR}/libs/kotext ${CMAKE_BINARY_DIR}/libs/kotext)
set(KFORMULA_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kformula ${CMAKE_BINARY_DIR}/libs/kformula)
set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)
# kopageapp
set(KOPAGEAPP_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )
# Those are included with e.g. <koproperty/set.h>
set(KOPROPERTY_INCLUDES ${CMAKE_SOURCE_DIR}/libs ${CMAKE_BINARY_DIR}/libs)

set(SHOULD_BUILD_KRITA TRUE)

if(NOT BLITZ_FOUND OR NOT EIGEN2_FOUND)
   set(SHOULD_BUILD_KRITA FALSE)
endif(NOT BLITZ_FOUND OR NOT EIGEN2_FOUND)

macro_log_feature(BLITZ_FOUND "qimageblitz" "Krita requires qimageblitz. Krita will not be built" "qimageblitz is available from kdesupport" FALSE "")

#Check if build kpresenter here to be able to test if we can build filters/kpresenter
if(Boost_FOUND)
    set(SHOULD_BUILD_KPRESENTER TRUE)
else(Boost_FOUND)
    set(SHOULD_BUILD_KPRESENTER FALSE)
endif(Boost_FOUND)

if(WIN32)
    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif(WIN32)

add_subdirectory(libs)
add_subdirectory(interfaces)
add_subdirectory(cmake)

macro_log_feature(FREETYPE_FOUND "FreeType" "Karbon requires FreeType. Karbon will not be built" "http://www.freetype.org" FALSE "")
macro_log_feature(FONTCONFIG_FOUND "Fontconfig" "Karbon requires Fontconfig. Karbon will not be built" "http://fontconfig.org" FALSE "")

if(FREETYPE_FOUND AND FONTCONFIG_FOUND )
   set(SHOULD_BUILD_KARBON true)
endif(FREETYPE_FOUND AND FONTCONFIG_FOUND)

if(SHOULD_BUILD_KARBON)
	macro_optional_add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON)


if(EIGEN2_FOUND)
    macro_optional_add_subdirectory(kspread)
    if(BUILD_kspread)
        set(SHOULD_BUILD_KSPREAD true)
    else(BUILD_kspread)
        set(SHOULD_BUILD_KSPREAD FALSE)
    endif(BUILD_kspread)
else(EIGEN2_FOUND)
    set(SHOULD_BUILD_KSPREAD FALSE)
endif(EIGEN2_FOUND)

if(SHOULD_BUILD_KPRESENTER)
    macro_optional_add_subdirectory(kpresenter)
endif(SHOULD_BUILD_KPRESENTER)

macro_optional_add_subdirectory(kdgantt)
macro_optional_add_subdirectory(kchart)

FIND_PROGRAM(BZIP2_EXECUTABLE NAMES bzip2 )

macro_log_feature(BZIP2_EXECUTABLE "bzip2" "bzip2 program is necessary to compile kexi. kexi will not be built" "http://www.bzip.org" FALSE "")

if( BZIP2_EXECUTABLE )
  macro_optional_add_subdirectory(kexi)
endif( BZIP2_EXECUTABLE )

macro_optional_add_subdirectory(kformula)
macro_optional_add_subdirectory(kool)
macro_optional_add_subdirectory(kivio)
macro_optional_add_subdirectory(kounavail)
macro_optional_add_subdirectory(kplato)
macro_optional_add_subdirectory(krita)
macro_optional_add_subdirectory(kword)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)
macro_optional_add_subdirectory(doc)

MACRO_DISPLAY_FEATURE_LOG()

ADD_CUSTOM_TARGET(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install( FILES
         ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h
         DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
