project(kritaanimator)

set(kritaanimatorpart_PART_SRCS krita_part_init.cpp)
kde4_add_plugin(kritaanimatorpart ${kritaanimatorpart_PART_SRCS})
target_link_libraries(kritaanimatorpart kritaui)
install(TARGETS kritaanimatorpart DESTINATION ${PLUGIN_INSTALL_DIR})

set(kritaanimator_KDEINIT_SRCS main.cpp)
kde4_add_kdeinit_executable(kritaanimator ${kritaanimator_KDEINIT_SRCS})
target_link_libraries(kdeinit_kritaanimator kritaui)
target_link_libraries(kritaanimator ${QT_QTCORE_LIBRARY} kdeinit_kritaanimator)

install(TARGETS kritaanimator ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kdeinit_kritaanimator ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kritaanimator.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kritaanimatorpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
