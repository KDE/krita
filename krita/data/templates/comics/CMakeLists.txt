install( FILES 
light_template_comics_empty.png
dark_template_comics_empty.png
DESTINATION
${KDE_INSTALL_DATADIR}/krita/pics
)

install( FILES  
    a4_waffle_grid.kra   
    BD-EuroTemplate.kra
    Comics-USTemplate.kra
    Manga-JpTemplate.kra
DESTINATION ${KDE_INSTALL_DATADIR}/krita/templates/comics/.source)

install( FILES  
    .directory 
    a4_waffle_grid.desktop  
    BD-EuroTemplate.desktop
    Comics-USTemplate.desktop
    Manga-JpTemplate.desktop
DESTINATION ${KDE_INSTALL_DATADIR}/krita/templates/comics)
