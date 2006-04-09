kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/profiles FILES
    Adobe.icm 
    Apple.icm 
    CIE.icm 
    CMY.icm 
    cmyk.icm 
    ColorMatch.icm 
    lcmslabi.icm 
    lcmsxyzi.icm 
    monoscnr.icm 
    NTSC.icm 
    PAL.icm 
    SMPTE-C.icm 
    srgb_color_space_profile.icm 
    sRGB.icm 
    srgbspac.icm 
    tifflab8spac.icm 
    WideGamut.icm 
    ycc601.icm 
    ycc709.icm
    )

kde4_footer()



#original Makefile.am contents follow:

#
#kritaprofilesdir = $(prefix)/share/apps/krita/profiles
#
#kritaprofiles_DATA = Adobe.icm Apple.icm CIE.icm CMY.icm cmyk.icm ColorMatch.icm lcmslabi.icm lcmsxyzi.icm monoscnr.icm NTSC.icm PAL.icm SMPTE-C.icm srgb_color_space_profile.icm sRGB.icm srgbspac.icm tifflab8spac.icm WideGamut.icm ycc601.icm ycc709.icm 
