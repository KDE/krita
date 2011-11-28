########### install files ###############
install(FILES
    Adobe.icm
    Apple.icm
    CIE.icm
    ColorMatch.icm
    lcmslabi.icm
    lcmsxyzi.icm
    NTSC.icm
    PAL.icm
    README
    scRGB.icm
    SMPTE-C.icm
    sRGB.icm
    srgbspac.icm
    WideGamut.icm
    ycc601.icm
    ycc709.icm
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/color/icc/krita)
