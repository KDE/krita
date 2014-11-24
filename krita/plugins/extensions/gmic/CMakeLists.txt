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

if(APPLE)
    add_definitions(-D_APPLE)
endif()
add_definitions(-Dgmic_build )
add_definitions(-Dgmic_float_only )
add_definitions(-Dcimg_use_zlib)

add_definitions(-Dcimg_display=0 )
add_definitions(-fexceptions )
# TODO: this only for debug

find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif( ZLIB_FOUND )

add_definitions(-Dcimg_use_vt100 )

if(FFTW3_FOUND)
    add_definitions(-Dcimg_use_fftw3 )
    add_definitions(-Dcimg_use_fftw3_singlethread )
endif(FFTW3_FOUND)


# fPIC is somehow needed to be able to link static library into shared object
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_library(gmic STATIC ${gmic_sources_SRCS} ${gmic_headers_SRCS})

if(FFTW3_FOUND)
  target_link_libraries(gmic ${FFTW3_LIBRARIES})
endif(FFTW3_FOUND)

target_link_libraries(gmic ${ZLIB_LIBRARIES})

if (WIN32)
    target_link_libraries(gmic)
else (WIN32)
    target_link_libraries(gmic pthread)
endif (WIN32)

set(GMIC_INSTALL_DIR ${DATA_INSTALL_DIR}/krita/gmic)
install(FILES
    gmic_def.gmic
    gmic_def.gmic.blacklist
DESTINATION ${GMIC_INSTALL_DIR} )

##
# compile Krita plug-in and link static library in
##

set(kritagmic_shared_PART_SRCS
    kis_gmic_parser.cpp
    kis_gmic_blacklister.cpp
    kis_gmic_filter_model.cpp
    kis_gmic_filter_settings.cpp
    Category.cpp
    Command.cpp
    Parameter.cpp
    kis_input_output_mapper.cpp
    kis_html_delegate.cpp
    kis_gmic_settings_widget.cpp
    kis_gmic_input_output_widget.cpp
    kis_gmic_filter_proxy_model.cpp
    kis_gmic_widget.cpp
    kis_gmic_updater.cpp
    kis_filter_preview_widget.cpp
)

kde4_add_ui_files(kritagmic_shared_PART_SRCS wdg_gmic.ui wdg_gmic_input_output.ui)

set(kritagmic_PART_SRCS
    kis_gmic_simple_convertor.cpp
    kis_export_gmic_processing_visitor.cpp
    kis_gmic_applicator.cpp
    kis_gmic_command.cpp
    kis_gmic_synchronize_layers_command.cpp
    kis_import_gmic_processing_visitor.cpp
    kis_gmic_plugin.cpp ${kritagmic_shared_PART_SRCS}
)

set(gmicparser_PART_SRCS main.cpp ${kritagmic_shared_PART_SRCS})

kde4_add_plugin(kritagmic ${kritagmic_PART_SRCS})

target_link_libraries(kritagmic kritaui gmic ${QT_QTXML_LIBRARY} ${ZLIB_LIBRARIES})

kde4_add_executable(gmicparser ${gmicparser_PART_SRCS})
target_link_libraries(gmicparser ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kritaui gmic ${ZLIB_LIBRARIES})

install(TARGETS kritagmic  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS gmicparser  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES  gmic.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritagmic.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

# tests, currently broken on OSX due to fftw linking problem
if(NOT APPLE)
    add_subdirectory(tests)
endif()
