set(kritawaveletdecompose_SOURCES
    waveletdecompose.cpp
    dlg_waveletdecompose.cpp
    kis_wavelet_kernel.cpp
    )

ki18n_wrap_ui(kritawaveletdecompose_SOURCES
    wdg_waveletdecompose.ui
    )

kis_add_library(kritawaveletdecompose MODULE ${kritawaveletdecompose_SOURCES})
target_link_libraries(kritawaveletdecompose kritaui)
install(TARGETS kritawaveletdecompose  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  waveletdecompose.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
