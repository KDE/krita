include_directories( ${CMAKE_BINARY_DIR}/filters/ ${POPPLER_INCLUDE_DIR} )

set(kritapdfimport_SOURCES kis_pdf_import.cpp kis_pdf_import_widget.cpp )

ki18n_wrap_ui(kritapdfimport_SOURCES pdfimportwidgetbase.ui )

add_library(kritapdfimport MODULE ${kritapdfimport_SOURCES})

target_link_libraries(kritapdfimport kritaui  ${POPPLER_LIBRARY} )

install(TARGETS kritapdfimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install(PROGRAMS krita_pdf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
