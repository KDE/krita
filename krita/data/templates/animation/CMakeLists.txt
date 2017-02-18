install( FILES 
light_template_animation.png
dark_template_animation.png
DESTINATION
${DATA_INSTALL_DIR}/krita/pics
)

install( FILES  
    Anim-Jp-EN.kra
    Anim-Jp-JP.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/animation/.source)

install( FILES  
    .directory
    Anim-Jp-EN.desktop 
    Anim-Jp-JP.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/animation)
