kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons )
install( FILES  
    Canon_550D_5184x3456.kra
    Canon_5Dmk3_5760x3840.kra
    Nikon_D3000_3872x2592.kra
    Nikon_D5000_4288x2848.kra
    Nikon_D7000_4928x3264.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/dslr/.source)

install( FILES  
    .directory
    Canon_550D_5184x3456.desktop
    Canon_5Dmk3_5760x3840.desktop
    Nikon_D3000_3872x2592.desktop
    Nikon_D5000_4288x2848.desktop
    Nikon_D7000_4928x3264.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/dslr)
