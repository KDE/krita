project(kritaanimation)

set(kritaanimationpart_PART_SRCS kritaanimation_part_init.cpp)
kde4_add_plugin(kritaanimationpart ${kritaanimationpart_PART_SRCS})
target_link_libraries(kritaanimationpart kritaui)
install(TARGETS kritaanimationpart DESTINATION ${PLUGIN_INSTALL_DIR})

set(kritaanimation_KDEINIT_SRCS main.cpp)
kde4_add_kdeinit_executable(kritaanimation ${kritaanimation_KDEINIT_SRCS})
target_link_libraries(kdeinit_kritaanimation kritaui)
target_link_libraries(kritaanimation ${QT_QTCORE_LIBRARY} kdeinit_kritaanimation)

install(TARGETS kritaanimation ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kdeinit_kritaanimation ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kritaanimation.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kritaanimationpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  krita.rc DESTINATION ${DATA_INSTALL_DIR}/kritaanimation)
