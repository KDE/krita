project(koffice)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# define the generic version of the KOffice libraries here
# this makes it easy to advance it when the next KOffice release comes
set(GENERIC_KOFFICE_LIB_VERSION "5.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "5")

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
macro_optional_find_package(OpenEXR)
macro_optional_find_package(Exif)
macro_optional_find_package(PythonLibs)
macro_optional_find_package(LibArt)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)
include (ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/config.h )

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# kostore is at the bottom of stack, so it has the dependency on the kde4 includes.
set(KOSTORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/store ${CMAKE_BINARY_DIR}/libs/store ${KDE4_INCLUDES})
# kofficecore depends on kostore
set(KOFFICECORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kofficecore ${CMAKE_BINARY_DIR}/libs/kofficecore ${KOSTORE_INCLUDES})
# flake depends on kofficecore
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake ${CMAKE_BINARY_DIR}/libs/flake )
# kofficeui depends on kofficecore
set(KOFFICEUI_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kofficeui ${CMAKE_BINARY_DIR}/libs/kofficeui ${KOFFICECORE_INCLUDES} ${FLAKE_INCLUDES} )
# pigment depends on kofficecore
set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/pigment ${CMAKE_SOURCE_DIR}/libs/pigment/colorspaces )

# The rest must be specified independently, in addition to one of the above three
set(KWMF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kwmf ${CMAKE_BINARY_DIR}/libs/kwmf)
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext ${CMAKE_BINARY_DIR}/libs/kotext)
set(KOPAINTER_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopainter ${CMAKE_BINARY_DIR}/libs/kopainter ${CMAKE_SOURCE_DIR}/libs/pigment)
set(KOPALETTE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopalette ${CMAKE_BINARY_DIR}/libs/kopalette)
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

add_subdirectory(libs)
MESSAGE(STATUS "Before koffice-2.0 disable compile of example subdir, not necessary for official release")
add_subdirectory(example)
add_subdirectory(interfaces)
if(LIBART_FOUND)
#	add_subdirectory(karbon)
endif(LIBART_FOUND)
add_subdirectory(kdgantt)
add_subdirectory(kchart)
#add_subdirectory(kexi)
add_subdirectory(kformula)
add_subdirectory(kivio)
add_subdirectory(koshell)
add_subdirectory(kounavail)
add_subdirectory(kplato)
add_subdirectory(kpresenter)
add_subdirectory(krita)
add_subdirectory(kspread)
add_subdirectory(kugar)
add_subdirectory(kword)
add_subdirectory(mimetypes)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)
add_subdirectory(autocorrect)
add_subdirectory(doc)
add_subdirectory( cmake )
message( STATUS "Re-add kexi when we will port them")


ADD_CUSTOM_TARGET(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
