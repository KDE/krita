########### install files ###############
install(FILES
    Adobe.icm 
    Apple.icm 
    CIE.icm 
    ColorMatch.icm 
    lcmslabi.icm 
    lcmsxyzi.icm 
    monoscnr.icm 
    NTSC.icm 
    PAL.icm 
    SMPTE-C.icm 
    sRGB.icm
    scRGB.icm
    srgbspac.icm 
    tifflab8spac.icm 
    WideGamut.icm 
    ycc601.icm 
    ycc709.icm
    ISO.icc
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/color/icc/krita)
