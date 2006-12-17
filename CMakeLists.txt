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
macro_optional_find_package(QCA2)
macro_optional_find_package(OpenEXR)
macro_optional_find_package(Exif)
macro_optional_find_package(PythonLibs)
macro_optional_find_package(LibArt)
macro_optional_find_package(Freetype)
macro_optional_find_package(Fontconfig)

macro_log_feature(QCA2_FOUND "libqca2" "QCA2 is needed to compile support for encrypted OpenDocument files" "Module into kdesupport" FALSE "2.0" "" )

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)
include (ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/config.h )

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${QDBUS_INCLUDE_DIRS}  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# kostore is at the bottom of stack, so it has the dependency on the kde4 includes.
set(KOSTORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/store ${CMAKE_BINARY_DIR}/libs/store ${KDE4_INCLUDES})
# kofficecore depends on kostore
set(KOFFICECORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kofficecore ${CMAKE_BINARY_DIR}/libs/kofficecore ${KOSTORE_INCLUDES})
# flake depends on kofficecore
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake ${CMAKE_SOURCE_DIR}/libs/flake/commands ${CMAKE_BINARY_DIR}/libs/flake )
# kofficeui depends on kofficecore
set(KOFFICEUI_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kofficeui ${CMAKE_BINARY_DIR}/libs/kofficeui ${KOFFICECORE_INCLUDES} ${FLAKE_INCLUDES} )
# pigment depends on kofficecore and lcms
set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/pigment ${CMAKE_SOURCE_DIR}/libs/pigment/colorspaces ${LCMS_INCLUDE_DIR} )

# The rest must be specified independently, in addition to one of the above three
set(KWMF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kwmf ${CMAKE_BINARY_DIR}/libs/kwmf)
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext/styles ${CMAKE_SOURCE_DIR}/libs/kotext ${CMAKE_BINARY_DIR}/libs/kotext)
set(KOPAINTER_INCLUDES $(CMAKE_SOURCE_DIR)/libs/kofficeui ${CMAKE_SOURCE_DIR}/libs/kopainter ${CMAKE_BINARY_DIR}/libs/kopainter ${CMAKE_SOURCE_DIR}/libs/pigment)
set(KFORMULA_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kformula ${CMAKE_BINARY_DIR}/libs/kformula)
set(KROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kross ${CMAKE_BINARY_DIR}/libs/kross)
# Those are included with e.g. <koproperty/set.h>
set(KOPROPERTY_INCLUDES ${CMAKE_SOURCE_DIR}/libs ${CMAKE_BINARY_DIR}/libs)

set(libkdgantt_SRCS
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttView.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewEventItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewItemDrag.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewSubwidgets.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewSummaryItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewTaskItem.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewTaskLink.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewTaskLinkGroup.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttMinimizeSplitter.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttSemiSizingControl.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttSizingControl.cpp
	${CMAKE_SOURCE_DIR}/kdgantt/KDGanttXMLTools.cpp
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
add_subdirectory(example)
add_subdirectory(interfaces)

if(FREETYPE_FOUND AND FONTCONFIG_FOUND )
   set(SHOULD_BUILD_KARBON true)
endif(FREETYPE_FOUND AND FONTCONFIG_FOUND)

if(SHOULD_BUILD_KARBON)
	add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON)	

add_subdirectory(kdgantt)
add_subdirectory(kchart)
#add_subdirectory(kexi)
# Disabled while libs/kformula is disabled
#add_subdirectory(kformula)
add_subdirectory(kivio)
#add_subdirectory(koshell)
add_subdirectory(kounavail)
add_subdirectory(kplato)
add_subdirectory(kpresenter)
add_subdirectory(krita)
add_subdirectory(kspread)
add_subdirectory(kugar)
add_subdirectory(kword)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)
add_subdirectory(shapes)
add_subdirectory(autocorrect)
add_subdirectory(doc)
add_subdirectory( cmake )
message( STATUS "Re-add kexi when we will port them")
message( STATUS "Re-add kpresenter when it starts to use flake")


ADD_CUSTOM_TARGET(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
