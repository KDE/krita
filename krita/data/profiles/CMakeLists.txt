########### install files ###############
install(FILES
    README
    scRGB.icm
    sRGB.icm
    WideGamut.icm
    krita25_lcms-builtin-sRGB_g100-truegamma.icc
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/color/icc/krita)
