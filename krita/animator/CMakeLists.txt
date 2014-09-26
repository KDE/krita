project(kritaanimation)

set(kritaanimationpart_PART_SRCS kritaanimation_part_init.cpp)
kde4_add_plugin(kritaanimationpart ${kritaanimationpart_PART_SRCS})
target_link_libraries(kritaanimationpart kritaui)
install(TARGETS kritaanimationpart DESTINATION ${PLUGIN_INSTALL_DIR})

set(kritaanimation_KDEINIT_SRCS main.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../kis_splash_screen.cpp")

if(WIN32 AND USE_BREAKPAD)
    set(kritaanimation_KDEINIT_SRCS ${kritaanimation_KDEINIT_SRCS} ../kis_crash_handler.cpp)
    set(BREAKPAD_LIBS breakpad)
endif(WIN32 AND USE_BREAKPAD)

kde4_add_ui_files(kritaanimation_KDEINIT_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/../wdgsplash.ui"
)

kde4_add_app_icon(kritaanimation_KDEINIT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pics/app/hi*-app-calligrakrita.png")
kde4_add_kdeinit_executable(kritaanimation ${kritaanimation_KDEINIT_SRCS})

target_link_libraries(kdeinit_kritaanimation kritaui)
target_link_libraries(kritaanimation ${BREAKPAD_LIBS} kdeinit_kritaanimation)

install(TARGETS kritaanimation ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kdeinit_kritaanimation ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kritaanimation.desktop DESTINATION ${XDG_APPS_INSTALL_DIR}/calligra)
install(FILES kritaanimationpart.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install(FILES  kritaanimation.rc DESTINATION ${DATA_INSTALL_DIR}/kritaanimation)
install(FILES kritaanimationrc DESTINATION ${CONFIG_INSTALL_DIR} )
install(FILES krita_animationdock.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES kritaanimation.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
