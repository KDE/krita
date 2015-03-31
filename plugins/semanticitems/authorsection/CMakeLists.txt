include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_authorsection_LIB_SRCS
    CAuSectionSemanticItemFactory.cpp
    CAuSectionSemanticItemPlugin.cpp
    CAuSectionRdf.cpp
)

ki18n_wrap_ui( calligra_semanticitem_authorsection_LIB_SRCS
   CAuSectionRdfEditWidget.ui
)

add_library(calligra_semanticitem_authorsection MODULE ${calligra_semanticitem_authorsection_LIB_SRCS})
kcoreaddons_desktop_to_json(calligra_semanticitem_authorsection calligra_semanticitem_authorsection.desktop)

target_link_libraries(calligra_semanticitem_authorsection   ${KORDF_LIBS})

install(TARGETS calligra_semanticitem_authorsection  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
