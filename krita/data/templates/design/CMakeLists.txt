kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons )
install( FILES  
    Designcinema16_10_2484x1200_96dpiRGB_8bit].kra
    Designcinema2.39_1_2484x1040_96dpiRGB_8bit.kra
    DesignpresentationA3Landscape[4960x3508,300dpiRGB,8bit].kra
    DesignpresentationA4portrait[2480x3508,300dpiRGB,8bit].kra
    Designscreen4_3_2250x1680_96dpiRGB_8bit.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design/.source)

install( FILES  
    .directory 
    Designcinema16_10_2484x1200_96dpiRGB_8bit.desktop
    Designcinema2.39_1_2484x1040_96dpiRGB_8bit.desktop
    DesignpresentationA3Landscape[4960x3508,300dpiRGB,8bit].desktop
    DesignpresentationA4portrait[2480x3508,300dpiRGB,8bit].desktop
    Designscreen4_3_2250x1680_96dpiRGB_8bit_.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design)
