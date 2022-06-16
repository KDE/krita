add_subdirectory(tests)

set(kritapdfimport_SOURCES kis_pdf_import.cpp kis_pdf_import_widget.cpp )

ki18n_wrap_ui(kritapdfimport_SOURCES pdfimportwidgetbase.ui )

add_library(kritapdfimport MODULE ${kritapdfimport_SOURCES})

target_link_libraries(kritapdfimport kritaui Poppler::Qt5)

install(TARGETS kritapdfimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(PROGRAMS krita_pdf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
