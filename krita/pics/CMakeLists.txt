install(FILES  
    AboutKrita.png
    newlayer.png  	
    visible.png  	
    novisible.png  	
    linked.png  	
    unlinked.png          
    tool_screenshot.png  	
    tablet.png  	
    locked.png  	
    unlocked.png
    transparency-locked.png
    transparency-unlocked.png
    transparency-enabled.png
    transparency-disabled.png
    selection_add.png
    selection_exclude.png
    selection_intersect.png
    selection_replace.png
    selection_subtract.png
    select_pixel.png
    select_shape.png
    local_selection_active.png
    local_selection_inactive.png
    paintop_settings_01.png
    paintop_settings_02.png
    paintop_settings_disabled.png
    paintop_presets_disabled.png
    width_icon.png
    height_icon.png
    ratio_icon.png
    shear_horizontal.png
    shear_vertical.png
    offset_horizontal.png
    offset_vertical.png
    portrait.png
    landscape.png
    broken-preset.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

add_subdirectory(app)


