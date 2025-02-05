########### install files ###############
add_subdirectory(elles-icc-profiles)
add_subdirectory(ycbcr-icc-profiles)
install(FILES
    README
    scRGB.icm
    cmyk.icm
    krita25_lcms-builtin-sRGB_g100-truegamma.icc
    ITUR_2100_PQ_FULL.ICC
    DESTINATION ${KDE_INSTALL_DATADIR}/color/icc/krita)
