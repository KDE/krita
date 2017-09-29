add_subdirectory(draganddrop)
add_subdirectory(kritasketchplugin)
install(DIRECTORY components    DESTINATION ${QML_INSTALL_DIR}/org/krita/sketch PATTERN "*~" EXCLUDE)
