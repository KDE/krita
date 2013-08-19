##
# Compile gmic as static library
##

set(gmic_sources_SRCS
gmic.cpp
)

set(gmic_headers_SRCS
CImg.h
gmic_def.h
gmic.h
)

add_definitions(-Dgmic_build )
add_definitions(-Dgmic_float_only )

add_definitions(-Dcimg_display=0 )
add_definitions(-fexceptions )
# TODO: this only for debug
add_definitions(-Dcimg_use_vt100 )

if(FFTW3_FOUND)
    add_definitions(-Dcimg_use_fftw3 )
endif(FFTW3_FOUND)


# fPIC is somehow needed to be able to link static library into shared object
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_library(gmic STATIC ${gmic_sources_SRCS} ${gmic_headers_SRCS})

if(FFTW3_FOUND)
  target_link_libraries(gmic ${FFTW3_LIBRARIES})
endif(FFTW3_FOUND)



##
# compile Krita plug-in and link static library in
##

set(kritagmic_shared_PART_SRCS kis_gmic_parser.cpp
kis_gmic_filter_model.cpp
kis_gmic_filter_settings.cpp
kis_gmic_settings_widget.cpp
kis_gmic_input_output_widget.cpp
kis_gmic_widget.cpp
Component.h
Category.cpp
Command.cpp
Filters.cpp
Parameter.cpp
)

set(kritagmic_PART_SRCS
    dlg_gmic.cpp
    kis_gmic.cpp
    kis_gmic_processing_visitor.cpp
    kis_gmic_plugin.cpp ${kritagmic_shared_PART_SRCS}
)

set(gmicparser_PART_SRCS main.cpp ${kritagmic_shared_PART_SRCS})


kde4_add_ui_files(kritagmic_PART_SRCS wdg_gmic.ui )
kde4_add_plugin(kritagmic ${kritagmic_PART_SRCS})

target_link_libraries(kritagmic kritaui gmic)

kde4_add_executable(gmicparser ${gmicparser_PART_SRCS})
target_link_libraries(gmicparser ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kritaui gmic)

install(TARGETS kritagmic  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS gmicparser  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES  gmic.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritagmic.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

