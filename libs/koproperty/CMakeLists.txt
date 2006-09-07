

add_subdirectory( editors )
add_subdirectory( test )

include_directories( ${CMAKE_SOURCE_DIR}/libs/koproperty/editors ${KOFFICECORE_INCLUDES} )


set(libkopropertyeditors_SRCS
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/booledit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/coloredit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/combobox.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/cursoredit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/dateedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/datetimeedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/dummywidget.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/fontedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/linestyledit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/pixmapedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/pointedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/rectedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/sizeedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/sizepolicyedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/spinbox.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/stringedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/stringlistedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/symbolcombo.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/timeedit.cpp
	${CMAKE_SOURCE_DIR}/libs/koproperty/editors/urledit.cpp
)

########### next target ###############

set(koproperty_LIB_SRCS ${libkopropertyeditors_SRCS}
   property.cpp
   customproperty.cpp
   set.cpp
   editor.cpp
   editoritem.cpp
   factory.cpp
   widget.cpp )

kde4_automoc(${koproperty_LIB_SRCS})

kde4_add_library(koproperty SHARED ${koproperty_LIB_SRCS})

target_link_libraries(koproperty ${KDE4_KIO_LIBS} )

set_target_properties(koproperty PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koproperty  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons  )
