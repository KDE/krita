set(kritaimageenhancement_SOURCES
    imageenhancement.cpp
    kis_simple_noise_reducer.cpp
    kis_wavelet_noise_reduction.cpp
    )
kis_add_library(kritaimageenhancement MODULE ${kritaimageenhancement_SOURCES})
target_link_libraries(kritaimageenhancement kritaui)
install(TARGETS kritaimageenhancement  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
