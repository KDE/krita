set(kritathumbnail_SRCS
    kritacreator.cpp
)

add_library(kritathumbnail MODULE ${kritathumbnail_SRCS})

target_link_libraries(kritathumbnail
    KF5::KIOWidgets
    KF5::Archive
    Qt5::Gui
)

install(TARGETS kritathumbnail  DESTINATION ${PLUGIN_INSTALL_DIR})

install(
    FILES
        krita_kra_thumbnail.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
