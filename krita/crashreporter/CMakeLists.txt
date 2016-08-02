remove_definitions(
  -DTRANSLATION_DOMAIN=\"krita\"
)
add_definitions(
  -DTRANSLATION_DOMAIN=\"kritacrashhandler\"
)

set(crashhandler_SRCS main.cpp mainwindow.cpp)
ki18n_wrap_ui(crashhandler_SRCS wdg_crash_reporter.ui)
add_executable(crashhandler ${crashhandler_SRCS})
target_link_libraries(crashhandler kritawidgetutils kritaglobal KF5::CoreAddons KF5::I18n Qt5::Widgets Qt5::Network)
install(TARGETS crashhandler ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES KikiNurse_sm.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
