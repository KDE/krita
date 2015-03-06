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

target_link_libraries(calligra_semanticitem_authorsection   ${KORDF_LIBS})


install(TARGETS calligra_semanticitem_authorsection  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   calligra_semanticitem_authorsection.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
