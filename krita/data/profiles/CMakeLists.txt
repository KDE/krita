########### install files ###############
add_subdirectory(elles-icc-profiles)
install(FILES
    README
    scRGB.icm
    cmyk.icm
    krita25_lcms-builtin-sRGB_g100-truegamma.icc
    DESTINATION ${SHARE_INSTALL_PREFIX}/color/icc/krita)
