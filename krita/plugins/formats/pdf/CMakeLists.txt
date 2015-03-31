
include_directories( ${CMAKE_BINARY_DIR}/filters/ ${POPPLER_INCLUDE_DIR} )

########### next target ###############

set(kritapdfimport_PART_SRCS kis_pdf_import.cpp kis_pdf_import_widget.cpp )

ki18n_wrap_ui(kritapdfimport_PART_SRCS pdfimportwidgetbase.ui )

add_library(kritapdfimport MODULE ${kritapdfimport_PART_SRCS})



target_link_libraries(kritapdfimport kritaui ${POPPLER_LIBRARY} )

install(TARGETS kritapdfimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
########### install files ###############

install( FILES  krita_pdf_import.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_pdf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
