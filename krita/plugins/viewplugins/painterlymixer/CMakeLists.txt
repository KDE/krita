set(kritapainterlymixer_PART_SRCS
    kis_painterlymixer.cpp
    painterlymixer.cpp
    kis_painterlymixerdocker.cpp
    )

kde4_automoc(${kritapainterlymixer_PART_SRCS}
             kis_painterlymixer.moc
             painterlymixer.moc)

# XXX: Thrain, for the layout of buttons, use Qt Designer, don't do it
#      by hand. Then add your ui files here
#kde4_add_ui_files(kritapainterlymixer_PART_SRCS
#    )

kde4_add_plugin(kritapainterlymixer ${kritapainterlymixer_PART_SRCS})

target_link_libraries(kritapainterlymixer kritaui)

install(TARGETS kritapainterlymixer  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES  kritapainterlymixer.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
