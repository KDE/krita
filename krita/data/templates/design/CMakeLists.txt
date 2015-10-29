install( FILES 
template_DIN_A3_landscape.png
template_DIN_A4_portrait.png
template_ratio_1610.png
template_ratio_2391.png
template_ratio_43.png
template_web_design.png
DESTINATION
${DATA_INSTALL_DIR}/krita/pics)

install( FILES  
    Designcinema16_10_2484x1200_96dpiRGB_8bit_.kra
    Designcinema2.39_1_2484x1040_96dpiRGB_8bit_.kra
    DesignpresentationA3Landscape_4960x3508_300dpiRGB_8bit_.kra
    DesignpresentationA4portrait_2480x3508_300dpiRGB_8bit_.kra
    Designscreen4_3_2250x1680_96dpiRGB_8bit_.kra
    web_design.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design/.source)

install( FILES  
    .directory 
    Designcinema16_10_2484x1200_96dpiRGB_8bit_.desktop
    Designcinema2.39_1_2484x1040_96dpiRGB_8bit_.desktop
    DesignpresentationA3Landscape_4960x3508_300dpiRGB_8bit_.desktop
    DesignpresentationA4portrait_2480x3508_300dpiRGB_8bit_.desktop
    Designscreen4_3_2250x1680_96dpiRGB_8bit_.desktop
    web_design.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design)
