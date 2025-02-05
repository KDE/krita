install( FILES 
light_template_animation.png
dark_template_animation.png
DESTINATION
${KDE_INSTALL_DATADIR}/krita/pics
)

install( FILES  
    Anim-Jp-EN.kra
    Anim-Jp-JP.kra
DESTINATION ${KDE_INSTALL_DATADIR}/krita/templates/animation/.source)

install( FILES  
    .directory
    Anim-Jp-EN.desktop 
    Anim-Jp-JP.desktop
DESTINATION ${KDE_INSTALL_DATADIR}/krita/templates/animation)
