include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_authorsection_LIB_SRCS
    CAuSectionSemanticItemFactory.cpp
    CAuSectionSemanticItemPlugin.cpp
    CAuSectionRdf.cpp
)

kde4_add_ui_files( calligra_semanticitem_authorsection_LIB_SRCS
   CAuSectionRdfEditWidget.ui
)

kde4_add_plugin(calligra_semanticitem_authorsection ${calligra_semanticitem_authorsection_LIB_SRCS})

target_link_libraries(calligra_semanticitem_authorsection ${KDE4_KDECORE_LIBS}  ${KORDF_LIBS})


install(TARGETS calligra_semanticitem_authorsection  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   calligra_semanticitem_authorsection.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
