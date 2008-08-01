

add_subdirectory( test )

include_directories( ${CMAKE_SOURCE_DIR}/libs/koproperty/editors ${CMAKE_SOURCE_DIR}/libs/guiutils ${KOMAIN_INCLUDES} )


set(libkopropertyeditors_SRCS
  editors/booledit.cpp
  editors/coloredit.cpp
  editors/combobox.cpp
  editors/cursoredit.cpp
  editors/dateedit.cpp
  editors/datetimeedit.cpp
  editors/dummywidget.cpp
  editors/fontedit.cpp
  editors/linestyledit.cpp
  editors/pixmapedit.cpp
  editors/pointedit.cpp
  editors/rectedit.cpp
  editors/sizeedit.cpp
  editors/sizepolicyedit.cpp
  editors/spinbox.cpp
  editors/stringedit.cpp
  editors/stringlistedit.cpp
  editors/symbolcombo.cpp
  editors/timeedit.cpp
  editors/urledit.cpp
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


kde4_add_library(koproperty SHARED ${koproperty_LIB_SRCS})

target_link_libraries(koproperty koguiutils ${KDE4_KDE3SUPPORT_LIBS} )

set_target_properties(koproperty PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koproperty  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons  )
install( FILES
customproperty.h
editor.h
editoritem.h
factory.h
koproperty_global.h
koproperty_export.h
property.h
set.h
utils.h
widget.h
widgetproxy.h
DESTINATION ${INCLUDE_INSTALL_DIR}/koproperty COMPONENT Devel)

install( FILES
editors/booledit.h
editors/booledit.h
editors/coloredit.h
editors/combobox.h
editors/cursoredit.h
editors/dateedit.h
editors/datetimeedit.h
editors/dummywidget.h
editors/fontedit.h
editors/linestyleedit.h
editors/pixmapedit.h
editors/pointedit.h
editors/rectedit.h
editors/sizeedit.h
editors/sizepolicyedit.h
editors/spinbox.h
editors/spinbox.h
editors/stringedit.h
editors/stringlistedit.h
editors/symbolcombo.h
editors/timeedit.h
editors/urledit.h
DESTINATION ${INCLUDE_INSTALL_DIR}/koproperty/editors COMPONENT Devel)

