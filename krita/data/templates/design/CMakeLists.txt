kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons )
install( FILES  
    Designcinema16_10_2484x1200_96dpiRGB_8bit_.kra
    Designcinema2.39_1_2484x1040_96dpiRGB_8bit_.kra
    DesignpresentationA3Landscape_4960x3508_300dpiRGB_8bit_.kra
    DesignpresentationA4portrait_2480x3508_300dpiRGB_8bit_.kra
    Designscreen4_3_2250x1680_96dpiRGB_8bit_.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design/.source)

install( FILES  
    .directory 
    Designcinema16_10_2484x1200_96dpiRGB_8bit_.desktop
    Designcinema2.39_1_2484x1040_96dpiRGB_8bit_.desktop
    DesignpresentationA3Landscape_4960x3508_300dpiRGB_8bit_.desktop
    DesignpresentationA4portrait_2480x3508_300dpiRGB_8bit_.desktop
    Designscreen4_3_2250x1680_96dpiRGB_8bit_.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design)
