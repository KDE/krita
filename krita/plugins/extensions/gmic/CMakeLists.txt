set(kritagmic_PART_SRCS kis_gmic.cpp dlg_gmic.cpp kis_gmic_plugin.cpp gmic.cpp)
set(gmicparser_PART_SRCS kis_gmic_parser.cpp Component.h Category.cpp Command.cpp Filters.cpp Parameter.cpp kis_gmic_filter_model.cpp)

kde4_add_ui_files(kritagmic_PART_SRCS wdg_gmic.ui )

kde4_add_plugin(kritagmic ${kritagmic_PART_SRCS})

add_definitions(-Dgmic_build )
add_definitions(-Dcimg_use_vt100 )
add_definitions(-Dcimg_use_fftw3 )
add_definitions(-Dcimg_display=0 )
add_definitions(-Dgmic_float_only )
add_definitions(-fexceptions )

target_link_libraries(kritagmic kritaui ${FFTW3_LIBRARIES})

kde4_add_executable(gmicparser ${gmicparser_PART_SRCS})
target_link_libraries(gmicparser ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS kritagmic  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS gmicparser  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES  gmic.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritagmic.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

