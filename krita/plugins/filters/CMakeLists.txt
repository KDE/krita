if (METAPLUGINS)

    add_definitions(${KDE4_ENABLE_EXCEPTIONS} -DMETAPLUGIN)

    set(kritametafilter_PART_SRCS
        oilpaintfilter/kis_oilpaint_filter.cpp
        roundcorners/kis_round_corners_filter.cpp
        randompickfilter/kis_wdg_random_pick.cpp
        randompickfilter/randompickfilter.cpp
        convolutionfilters/kis_convolution_filter.cpp
        convolutionfilters/convolutionfilters.cpp
        colorsfilters/kis_brightness_contrast_filter.cpp
        colorsfilters/colorsfilters.cpp
        colorsfilters/kis_perchannel_filter.cpp
        colorsfilters/kis_hsv_adjustment_filter.cpp
        raindropsfilter/kis_raindrops_filter.cpp
        colors/kis_wdg_color_to_alpha.cpp
        colors/kis_minmax_filters.cpp
        colors/kis_color_to_alpha.cpp
        wavefilter/kis_wdg_wave.cpp
        wavefilter/wavefilter.cpp
        smalltilesfilter/kis_small_tiles_filter.cpp
        sobelfilter/kis_sobel_filter.cpp
        phongbumpmap/kis_phong_bumpmap_filter.cpp
        phongbumpmap/phong_pixel_processor.cpp
        phongbumpmap/kis_phong_bumpmap_config_widget.cpp
        embossfilter/kis_emboss_filter.cpp
        dodgeburn/DodgeBurn.cpp
        noisefilter/noisefilter.cpp
        noisefilter/kis_wdg_noise.cpp
        blur/kis_wdg_lens_blur.cpp
        blur/kis_motion_blur_filter.cpp
        blur/kis_gaussian_blur_filter.cpp
        blur/kis_lens_blur_filter.cpp
        blur/kis_wdg_gaussian_blur.cpp
        blur/kis_wdg_motion_blur.cpp
        blur/kis_blur_filter.cpp
        blur/kis_wdg_blur.cpp
        levelfilter/kis_level_filter.cpp
        fastcolortransfer/fastcolortransfer.cpp
        fastcolortransfer/kis_wdg_fastcolortransfer.cpp
        imageenhancement/kis_wavelet_noise_reduction.cpp                         
        imageenhancement/kis_simple_noise_reducer.cpp
        example/example.cpp
        pixelizefilter/kis_pixelize_filter.cpp
        unsharp/kis_unsharp_filter.cpp
        unsharp/kis_wdg_unsharp.cpp
    )

   kde4_add_ui_files(kritametafilter_PART_SRCS
        randompickfilter/wdgrandompickoptions.ui
        colorsfilters/wdg_perchannel.ui
        colorsfilters/wdg_brightness_contrast.ui
        colorsfilters/wdg_hsv_adjustment.ui
        colors/wdgcolortoalphabase.ui
        wavefilter/wdgwaveoptions.ui
        phongbumpmap/wdgphongbumpmap.ui
        dodgeburn/DodgeBurnConfigurationBaseWidget.ui
        noisefilter/wdgnoiseoptions.ui
        blur/wdg_gaussian_blur.ui
        blur/wdgblur.ui
        blur/wdg_motion_blur.ui
        blur/wdg_lens_blur.ui
        levelfilter/wdg_level.ui
        fastcolortransfer/wdgfastcolortransfer.ui
        unsharp/wdgunsharp.ui
   )

   kde4_add_plugin(kritametafilter ${kritametafilter_PART_SRCS})
   target_link_libraries(kritametafilter kritaui)
   install(TARGETS kritametafilter DESTINATION ${PLUGIN_INSTALL_DIR})
   install(FILES kritametafilter.desktop DESTINATION ${SERVICES_INSTALL_DIR})

else (METAPLUGINS)
    add_subdirectory( tests )
    add_subdirectory( blur )
    add_subdirectory( colors )
    add_subdirectory( colorsfilters )
    add_subdirectory( convolutionfilters )
    add_subdirectory( embossfilter )
    add_subdirectory( example )
    add_subdirectory( fastcolortransfer )
    add_subdirectory( imageenhancement )
    add_subdirectory( noisefilter )
    add_subdirectory( oilpaintfilter )
    add_subdirectory( pixelizefilter )
    add_subdirectory( raindropsfilter )
    add_subdirectory( randompickfilter )
    add_subdirectory( roundcorners )
    add_subdirectory( smalltilesfilter )
    add_subdirectory( sobelfilter )
    add_subdirectory( unsharp )
    add_subdirectory( wavefilter )
    add_subdirectory( levelfilter )
    add_subdirectory( dodgeburn )
    add_subdirectory( phongbumpmap )
endif (METAPLUGINS)
