file(GLOB _ill_files *.ill)
install(FILES ${_ill_files} 
DESTINATION ${DATA_INSTALL_DIR}/krita/illuminants)
