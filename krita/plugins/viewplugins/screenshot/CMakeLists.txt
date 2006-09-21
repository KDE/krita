




########### next target ###############

set(kritascreenshot_PART_SRCS screenshot.cpp ksnapshot.cpp regiongrabber.cpp )

kde4_automoc(${kritascreenshot_PART_SRCS})

kde4_add_ui3_files(kritascreenshot_PART_SRCS ksnapshotwidget.ui )

kde4_add_plugin(kritascreenshot ${kritascreenshot_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritascreenshot )

target_link_libraries(kritascreenshot kritaui)

install(TARGETS kritascreenshot  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  screenshot-krita.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritascreenshot.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

