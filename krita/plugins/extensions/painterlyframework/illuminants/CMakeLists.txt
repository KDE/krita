file(GLOB _xll_files *.xll)
install(FILES ${_xll_files} 
DESTINATION ${DATA_INSTALL_DIR}/krita/illuminants)
