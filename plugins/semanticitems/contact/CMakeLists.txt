add_definitions(-DTRANSLATION_DOMAIN=\"calligra_semanticitem_contact\")

include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_contact_LIB_SRCS
    KoContactSemanticItemFactory.cpp
    KoContactSemanticItemPlugin.cpp
    KoRdfFoaF.cpp
    KoRdfFoaFTreeWidgetItem.cpp
)

ki18n_wrap_ui( calligra_semanticitem_contact_LIB_SRCS
   KoRdfFoaFEditWidget.ui
)

add_library(calligra_semanticitem_contact MODULE ${calligra_semanticitem_contact_LIB_SRCS})

if (KF5Contacts_FOUND AND KF5Akonadi_FOUND)
   add_definitions(-DKDEPIMLIBS_FOUND)
   target_link_libraries(calligra_semanticitem_contact
      KF5::Contacts
      KF5::AkonadiCore
   )
endif ()

target_link_libraries(calligra_semanticitem_contact   ${KORDF_LIBS} KF5::I18n KF5::KDELibs4Support)
kcoreaddons_desktop_to_json(calligra_semanticitem_contact calligra_semanticitem_contact.desktop)

install(TARGETS calligra_semanticitem_contact  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
