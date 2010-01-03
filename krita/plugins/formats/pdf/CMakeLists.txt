
include_directories( ${CMAKE_BINARY_DIR}/filters/ ${POPPLER_INCLUDE_DIR} )

add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kritapdfimport_PART_SRCS kis_pdf_import.cpp kis_pdf_import_widget.cpp )

kde4_add_ui_files(kritapdfimport_PART_SRCS pdfimportwidgetbase.ui )

kde4_add_plugin(kritapdfimport WITH_PREFIX ${kritapdfimport_PART_SRCS})



target_link_libraries(kritapdfimport kritaui ${POPPLER_LDFLAGS} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kritapdfimport  DESTINATION ${PLUGIN_INSTALL_DIR})
########### install files ###############

install( FILES  krita_pdf_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_pdf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
