include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_contact_LIB_SRCS
    KoContactSemanticItemFactory.cpp
    KoContactSemanticItemPlugin.cpp
    KoRdfFoaF.cpp
    KoRdfFoaFTreeWidgetItem.cpp
)

kde4_add_ui_files( calligra_semanticitem_contact_LIB_SRCS
   KoRdfFoaFEditWidget.ui
)

kde4_add_plugin(calligra_semanticitem_contact ${calligra_semanticitem_contact_LIB_SRCS})

if (KDEPIMLIBS_FOUND)
   include_directories(${KDEPIMLIBS_INCLUDE_DIR})
   add_definitions(-DKDEPIMLIBS_FOUND)
   target_link_libraries(calligra_semanticitem_contact
      ${KDEPIMLIBS_KABC_LIBS}
      ${KDEPIMLIBS_AKONADI_LIBS}
   )
endif ()

target_link_libraries(calligra_semanticitem_contact ${KDE4_KDECORE_LIBS}  ${KORDF_LIBS})


install(TARGETS calligra_semanticitem_contact  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   calligra_semanticitem_contact.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
