project(koffice)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# define the generic version of the KOffice libraries here
# this makes it easy to advance it when the next KOffice release comes
set(GENERIC_KOFFICE_LIB_VERSION "5.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "5")

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(LCMS REQUIRED)
find_package(PNG REQUIRED)
find_package(Eigen)
macro_optional_find_package(QCA2)
macro_optional_find_package(Exif)
macro_optional_find_package(LibArt)
macro_optional_find_package(Freetype)
macro_optional_find_package(Fontconfig)
macro_log_feature(EIGEN_FOUND "Eigen" "Eigen is needed by KSpread, KSpread won't be built" "http://eigen.tuxfamily.org" TRUE "1.0.5")

macro_log_feature(QCA2_FOUND "libqca2" "QCA2 is needed to compile support for encrypted OpenDocument files" "Module into kdesupport" FALSE "2.0" "" )

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

include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
configure_file(config-endian.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-endian.h )

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${QDBUS_INCLUDE_DIRS}  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# kostore is at the bottom of stack, so it has the dependency on the kde4 includes.
set(KOSTORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/store ${CMAKE_BINARY_DIR}/libs/store ${KDE4_INCLUDES})
# komain depends on kostore
set(KOMAIN_INCLUDES ${CMAKE_SOURCE_DIR}/libs/main ${CMAKE_SOURCE_DIR}/libs/main/threadAction ${CMAKE_BINARY_DIR}/libs/main ${KOSTORE_INCLUDES})
# pigment depends on komain and lcms
set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/pigment ${CMAKE_SOURCE_DIR}/libs/pigment/colorspaces ${LCMS_INCLUDE_DIR} )
# flake depends on komain
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake ${CMAKE_SOURCE_DIR}/libs/flake/commands ${CMAKE_BINARY_DIR}/libs/flake)
# koresources depends on pigment
set(KORESOURCES_INCLUDES ${CMAKE_SOURCE_DIR}/libs/resources ${CMAKE_BINARY_DIR}/libs/resources ${PIGMENT_INCLUDES})
# koguiutils depends on komain, flake and pigment
set(KOGUIUTILS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/guiutils ${CMAKE_BINARY_DIR}/libs/guiutils ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${PIGMENT_INCLUDES} ${KORESOURCES_INCLUDES})
# kopageapp
set(KOPAGEAPP_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

# The rest must be specified independently, in addition to one of the above three
set(KWMF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kwmf ${CMAKE_BINARY_DIR}/libs/kwmf)
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext/styles ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument ${CMAKE_SOURCE_DIR}/libs/kotext ${CMAKE_BINARY_DIR}/libs/kotext)
set(KOPAINTER_INCLUDES ${KOGUIUTILS_INCLUDES} ${CMAKE_SOURCE_DIR}/libs/kopainter ${CMAKE_BINARY_DIR}/libs/kopainter ${PIGMENT_INCLUDES})
set(KFORMULA_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kformula ${CMAKE_BINARY_DIR}/libs/kformula)
set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)
# Those are included with e.g. <koproperty/set.h>
set(KOPROPERTY_INCLUDES ${CMAKE_SOURCE_DIR}/libs ${CMAKE_BINARY_DIR}/libs)

set(libkdgantt1_SRCS
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttView.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewEventItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewItemDrag.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewSubwidgets.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewSummaryItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskLink.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskLinkGroup.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttMinimizeSplitter.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttSemiSizingControl.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttSizingControl.cpp
	${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttXMLTools.cpp
	)

# check if build krita there to be able to test if we can build filters/krita
set(REQUIRED_LCMS_VERSION 115)

if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(SHOULD_BUILD_KRITA TRUE)
else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
        set(SHOULD_BUILD_KRITA FALSE)
        message(STATUS  "krita requires LittleCMS 1.15 or greater. Krita will not be built.\n"
                        "   If you want to compile Krita you should install:\n"
                        "   lcms 1.15 or newer (http://www.littlecms.com/)")
endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})



add_subdirectory(libs)
MESSAGE(STATUS "Before koffice-2.0 disable compile of example subdir, not necessary for official release")
#add_subdirectory(example)
add_subdirectory(interfaces)

if(FREETYPE_FOUND AND FONTCONFIG_FOUND )
   set(SHOULD_BUILD_KARBON true)
endif(FREETYPE_FOUND AND FONTCONFIG_FOUND)

if(SHOULD_BUILD_KARBON)
	macro_optional_add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON)

if(EIGEN_FOUND)
    set(SHOULD_BUILD_KSPREAD true)
else(EIGEN_FOUND)
        set(SHOULD_BUILD_KSPREAD FALSE)
        message(STATUS  "KSpread requires the eigen library, KSpread will not be built.\n"
                        "   If you want to compile KSpread you should install:\n"
                        "   lib eigen http://eigen.tuxfamily.org")
endif(EIGEN_FOUND)

if(SHOULD_BUILD_KSPREAD)
    macro_optional_add_subdirectory(kspread)
endif(SHOULD_BUILD_KSPREAD)

macro_optional_add_subdirectory(kdgantt1)
macro_optional_add_subdirectory(kdgantt)
# macro_optional_add_subdirectory(kchart)
FIND_PROGRAM(BZIP2_EXECUTABLE NAMES bzip2 )
if( BZIP2_EXECUTABLE )
  macro_optional_add_subdirectory(kexi)
else( BZIP2_EXECUTABLE )
  MESSAGE(STATUS "bzip2 program is necessary to compile kexi")
endif( BZIP2_EXECUTABLE )
macro_optional_add_subdirectory(kformula)
macro_optional_add_subdirectory(kivio)
#macro_optional_add_subdirectory(koshell)
macro_optional_add_subdirectory(kounavail)
macro_optional_add_subdirectory(kplato)
macro_optional_add_subdirectory(kpresenter)
macro_optional_add_subdirectory(krita)
macro_optional_add_subdirectory(kugar)
macro_optional_add_subdirectory(kword)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)
add_subdirectory(shapes)
add_subdirectory(doc)
add_subdirectory( cmake )
message( STATUS "Re-add koshell when ported")


ADD_CUSTOM_TARGET(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
