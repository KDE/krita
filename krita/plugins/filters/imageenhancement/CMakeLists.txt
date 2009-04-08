
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(kritaimageenhancement_PART_SRCS
    imageenhancement.cpp
    kis_simple_noise_reducer.cpp
    kis_wavelet_noise_reduction.cpp
    )

kde4_add_plugin(kritaimageenhancement ${kritaimageenhancement_PART_SRCS})



target_link_libraries(kritaimageenhancement kritaui)

install(TARGETS kritaimageenhancement  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaimageenhancement.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

