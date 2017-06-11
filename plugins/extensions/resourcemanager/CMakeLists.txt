find_package(KF5NewStuffCore ${KF5_DEP_VERSION} CONFIG REQUIRED)
set_package_properties(KF5NewStuffCore PROPERTIES
    DESCRIPTION "The KNewStuff library implements collaborative data sharing for applications."
    URL "https://api.kde.org/frameworks/knewstuff/html/index.html"
    TYPE REQUIRED
    PURPOSE "Required to implement sharing functionality with share.krita.org and krita")

find_package(KF5Attica ${KF5_DEP_VERSION} CONFIG REQUIRED)
set_package_properties(KF5Attica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for KF5NewStuffCore to get the contents."
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                      )

set(kritaresourcemanager_SOURCES
    resourcemanager.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp

    widgetquestionlistener.cpp
    entrydetailsdialog.cpp
    itemsgridviewdelegate.cpp
    itemsviewbasedelegate.cpp
    itemsviewdelegate.cpp
    itemsview.cpp
    imagepreviewwidget.cpp
    dlg_content_downloader.cpp
)

ki18n_wrap_ui(kritaresourcemanager_SOURCES
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
    wdgdlgcontentdownloader.ui
)

install( FILES
    data/kritapresets.knsrc
    datakritabrushes.knsrc
    data/kritapatterns.knsrc
    data/kritagradients.knsrc
    DESTINATION ${CONFIG_INSTALL_DIR} )


add_library(kritaresourcemanager MODULE ${kritaresourcemanager_SOURCES})
target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop KF5::NewStuffCore)
install(TARGETS kritaresourcemanager  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES resourcemanager.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
