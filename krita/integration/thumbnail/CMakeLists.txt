set(kritathumbnail_SRCS
    kritacreator.cpp
)

kde4_add_plugin(kritathumbnail ${kritathumbnail_SRCS})

target_link_libraries(kritathumbnail
    koodf
)

install(TARGETS kritathumbnail  DESTINATION ${PLUGIN_INSTALL_DIR})

install(
    FILES
        krita_kra_thumbnail.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
