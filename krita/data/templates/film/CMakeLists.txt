kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons )
install( FILES  
    1K_super35_fullAp_1024x778.kra
    2K_super35_fullAp_2048x1556.kra
    4K_super35_fullAp_4096x3112.kra
    HD_720p_1280x720.kra
    HD_full_1920x1080.kra
    HD_half_960x540.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/film/.source)

install( FILES  
    .directory
    1K_super35_fullAp_1024x778.desktop
    2K_super35_fullAp_2048x1556.desktop
    4K_super35_fullAp_4096x3112.desktop
    HD_720p_1280x720.desktop
    HD_full_1920x1080.desktop
    HD_half_960x540.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/film)
