include_directories(lib/ libpfs/)

set(kritatonemapping_PART_SRCS
    tonemapping.cc
# Krita Tone Mapping Library
    lib/kis_tone_mapping_operator.cc
    lib/kis_tone_mapping_operators_registry.cc
# PFS Library (only needed for most operators)

# ashikhmin 02 operator
    operators/ashikhmin02/tmo_ashikhmin02.cpp


)


# kde4_add_ui_files(kritatonemapping_PART_SRCS myuifile.ui )

kde4_add_plugin(kritatonemapping ${kritatonemapping_PART_SRCS})

target_link_libraries(kritatonemapping kritaui)

install(TARGETS kritatonemapping  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tonemapping.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritatonemapping.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

