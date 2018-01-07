find_package(KF5NewStuffCore ${KF5_DEP_VERSION})
set_package_properties(KF5NewStuffCore PROPERTIES
    DESCRIPTION "The KNewStuff library implements collaborative data sharing for applications."
    URL "https://api.kde.org/frameworks/knewstuff/html/index.html"
    TYPE OPTIONAL
    PURPOSE "Required to implement sharing functionality with share.krita.org and krita")
macro_bool_to_01(KF5NewStuffCore_FOUND HAVE_NEWSTUFF)

find_package(KF5Attica ${KF5_DEP_VERSION})
set_package_properties(KF5Attica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for KF5NewStuffCore to get the contents."
                       URL "https://projects.kde.org/attica"
                       TYPE OPTIONAL
                      )
macro_bool_to_01(KF5NewStuffCore_FOUND HAVE_ATTICA)

set(kritaresourcemanager_SOURCES
    resourcemanager.cpp
    dlg_create_bundle.cpp
    dlg_bundle_manager.cpp
)

if (HAVE_NEWSTUFF)
    set(kritaresourcemanager_SOURCES
        ${kritaresourcemanager_SOURCES}
        ContentDownloaderWidget.cpp
        content_downloader_dialog.cpp
        widgetquestionlistener.cpp
        entrydetailsdialog.cpp
        imagepreviewwidget.cpp
        itemsgridviewdelegate.cpp
        itemsviewbasedelegate.cpp
        itemsviewdelegate.cpp
        itemsview.cpp
    )
endif()

ki18n_wrap_ui(kritaresourcemanager_SOURCES
    wdgdlgcreatebundle.ui 
    wdgdlgbundlemanager.ui
)

if (HAVE_NEWSTUFF) 
    add_definitions(-DHAVE_NEWSTUFF)
    ki18n_wrap_ui(kritaresourcemanager_SOURCES
        wdgdlgcontentdownloader.ui
    )    
    install( FILES
        data/kritaresourcebundles.knsrc
        data/kritatemplates.knsrc
        data/kritaprofiles.knsrc
        DESTINATION ${CONFIG_INSTALL_DIR} 
    )
endif()



add_library(kritaresourcemanager MODULE ${kritaresourcemanager_SOURCES})
if (HAVE_NEWSTUFF)
    target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop KF5::NewStuffCore)
else()
    target_link_libraries(kritaresourcemanager kritawidgets kritaui kritalibpaintop)
endif()
install(TARGETS kritaresourcemanager  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES resourcemanager.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
