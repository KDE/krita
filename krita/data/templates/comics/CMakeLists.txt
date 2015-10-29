install( FILES 
template_comics_empty.png
DESTINATION
${DATA_INSTALL_DIR}/krita/pics
)

install( FILES  
    a4_waffle_grid.kra   
    BD-EuroTemplate.kra
    Comics-USTemplate.kra
    Manga-JpTemplate.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/comics/.source)

install( FILES  
    .directory 
    a4_waffle_grid.desktop  
    BD-EuroTemplate.desktop
    Comics-USTemplate.desktop
    Manga-JpTemplate.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/comics)
