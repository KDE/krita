project(koffice)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
# Perhaps define share/apps as ${DATA_INSTALL_DIR}
#set(CMAKE_MODULE_PATH "${KDEDIR}/share/apps/cmake/modules/ ${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# Always include srcdir and builddir in include path
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# support for Dart: http://public.kitware.com/dashboard.php?name=kde
enable_testing()
include(CTest)

# search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)

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
include_directories( ${CMAKE_SOURCE_DIR}/lib/kofficecore/ ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# TODO remove this (wrong order) once we have per-lib variables to use like in kdelibs.
include_directories( ${KDE4_INCLUDE_DIRS} )

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
	
kde4_add_ui3_files(libkdgantt_SRCS 
	${CMAKE_SOURCE_DIR}/kdgantt/itemAttributeDialog.ui
	)


add_subdirectory(libs)
#add_subdirectory(example)
add_subdirectory(interfaces)
#add_subdirectory(karbon)
add_subdirectory(kdgantt)
#add_subdirectory(kchart)
#add_subdirectory(kexi)
#add_subdirectory(kformula)
#add_subdirectory(kivio)
#add_subdirectory(koshell)
add_subdirectory(kounavail)
#add_subdirectory(kplato)
#add_subdirectory(kpresenter)
#add_subdirectory(krita)
#add_subdirectory(kspread)
add_subdirectory(kugar)
#add_subdirectory(kword)
#add_subdirectory(ascend)
add_subdirectory(mimetypes)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)
add_subdirectory(autocorrect)
add_subdirectory(doc)
MESSAGE(STATUS "Perhaps not install FindKDE4. It will conflict when we install with rpm/deb package")
add_subdirectory( cmake )

kde4_create_doxygen_docs(  )

