install( FILES 
template_animation.png
DESTINATION
${DATA_INSTALL_DIR}/krita/pics
)

install( FILES  
    Anim-Jp-EN.kra
    Anim-Jp-JP.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/animation/.source)

install( FILES  
    Anim-Jp-EN.desktop 
    Anim-Jp-JP.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/animation)
