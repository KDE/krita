install( FILES 
light_template_dslr.png
dark_template_dslr.png
DESTINATION
${KDE_INSTALL_DATADIR}/krita/pics )

install( FILES  
    Canon_550D_5184x3456.kra
    Canon_5Dmk3_5760x3840.kra
    Nikon_D3000_3872x2592.kra
    Nikon_D5000_4288x2848.kra
    Nikon_D7000_4928x3264.kra
DESTINATION ${KDE_INSTALL_DATADIR}/krita/templates/dslr/.source)

install( FILES  
    .directory
    Canon_550D_5184x3456.desktop
    Canon_5Dmk3_5760x3840.desktop
    Nikon_D3000_3872x2592.desktop
    Nikon_D5000_4288x2848.desktop
    Nikon_D7000_4928x3264.desktop
DESTINATION ${KDE_INSTALL_DATADIR}/krita/templates/dslr)
