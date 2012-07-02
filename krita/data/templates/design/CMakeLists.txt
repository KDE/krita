kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons )
install( FILES  
    Designcinema2.39:1[2484x1040,96dpiRGB,8bit].kra
    Designcinema16:10[2484x1200,96dpiRGB,8bit].kra
    DesignpresentationA3Landscape[4960x3508,300dpiRGB,8bit].kra
    DesignpresentationA4portrait[2480x3508,300dpiRGB,8bit].kra
    Designscreen4:3[2250x1680,96dpiRGB,8bit].kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design/.source)

install( FILES  
    .directory 
    Designcinema2.39:1[2484x1040,96dpiRGB,8bit].desktop
    Designcinema16:10[2484x1200,96dpiRGB,8bit].desktop
    DesignpresentationA3Landscape[4960x3508,300dpiRGB,8bit].desktop
    DesignpresentationA4portrait[2480x3508,300dpiRGB,8bit].desktop
    Designscreen4:3[2250x1680,96dpiRGB,8bit].desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/design)
