add_subdirectory( test )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44022)

set(libkopropertyeditors_SRCS
  editors/utils.cpp
  editors/booledit.cpp
  editors/coloredit.cpp
  editors/combobox.cpp
  editors/cursoredit.cpp
  editors/dateedit.cpp
  editors/datetimeedit.cpp
#  editors/dummywidget.cpp
  editors/fontedit.cpp
  editors/pixmapedit.cpp
  editors/pointedit.cpp
  editors/pointfedit.cpp
  editors/rectedit.cpp
  editors/sizeedit.cpp
  editors/sizefedit.cpp
  editors/sizepolicyedit.cpp
  editors/spinbox.cpp
  editors/stringedit.cpp
  editors/linestyleedit.cpp
#  editors/stringlistedit.cpp
#  editors/symbolcombo.cpp
  editors/timeedit.cpp
#  editors/urledit.cpp
)

include_directories(${KOPROPERTY_INCLUDES})

########### next target ###############

set(koproperty_LIB_SRCS
   Property.cpp
   Set.cpp
   Factory.cpp
   DefaultFactory.cpp
   EditorView.cpp
   EditorDataModel.cpp
   Utils.cpp
   ${libkopropertyeditors_SRCS}
)

kde4_add_library(koproperty SHARED ${koproperty_LIB_SRCS})
target_link_libraries(koproperty ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kowidgets)
target_link_libraries(koproperty LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kowidgets)

set_target_properties(koproperty PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION})
install(TARGETS koproperty ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

kde4_install_icons(${DATA_INSTALL_DIR}/koproperty/icons)

if(FALSE) # TODO: install when we move to independent place
  install( FILES
    Factory.h
    koproperty_global.h
    koproperty_export.h
    Property.h
    Set.h
    Utils.h
    Factory.h
    EditorView.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/koproperty COMPONENT Devel
  )

  install( FILES
    editors/utils.h
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
endif(FALSE)
