include_directories(lib/ libpfs/)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(kritatonemapping_PART_SRCS
    tonemapping.cc
    kis_tonemapping_dialog.cc
    kis_bookmarked_tone_mapping_operator_configurations_model.cc
# Krita Tone Mapping Library
    lib/kis_tone_mapping_operator.cc
    lib/kis_tone_mapping_operators_registry.cc
    lib/kis_tone_mapping_operator_configuration_widget.cc
# PFS Library (only needed for most operators)
    libpfs/kis_array2d.cc
    libpfs/pfs.cpp
# ashikhmin 02 operator
#     operators/ashikhmin02/tmo_ashikhmin02.cpp
#     operators/ashikhmin02/kis_ashikhmin02_operator.cc
# reinhard 05 operator
    operators/reinhard05/tmo_reinhard05.cpp
    operators/reinhard05/kis_reinhard05_operator.cc
# reinhard 02 operator
    operators/reinhard02/approx.cpp
    operators/reinhard02/tmo_reinhard02.cpp
    operators/reinhard02/kis_reinhard02_operator.cc
)


kde4_add_ui_files(kritatonemapping_PART_SRCS
#     operators/ashikhmin02/ashikhmin02_configuration_widget.ui
    operators/reinhard05/reinhard05_configuration_widget.ui
    operators/reinhard02/reinhard02_configuration_widget.ui
    wdgtonemappingdialog.ui )

kde4_add_plugin(kritatonemapping ${kritatonemapping_PART_SRCS})

target_link_libraries(kritatonemapping kritaui)

install(TARGETS kritatonemapping  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tonemapping.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritatonemapping.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

