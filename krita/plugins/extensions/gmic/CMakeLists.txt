##
# CMake for gmic
##

include_directories(src)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")

set(gmic_sources_SRCS
  src/gmic.cpp
)

set(gmic_headers_SRCS
  src/CImg.h
  src/gmic_def.h
  src/gmic.h
)

# Mandatory flags
add_definitions(-Dgmic_build)
add_definitions(-Dgmic_float_only)
add_definitions(-Dcimg_use_vt100 )

# PARALLEL
find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    add_definitions(-Dgmic_is_parallel)
    add_definitions(-Dcimg_use_rng)
endif()

#OpenMP
find_package(OpenMP)
if(OPENMP_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif(OPENMP_FOUND)

#FFTW
if(FFTW3_FOUND)
    add_definitions(-Dcimg_use_fftw3 )
    add_definitions(-Dcimg_use_fftw3_singlethread )
endif(FFTW3_FOUND)

# PNG
if (PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIR})
    add_definitions(${PNG_DEFINITIONS})
    add_definitions(-Dcimg_use_png)
endif(PNG_FOUND)

# ZLIB
find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
    add_definitions(-Dcimg_use_zlib)
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif( ZLIB_FOUND )


# CURL
find_package(CURL)
if (CURL_FOUND)
    add_definitions(-Dcimg_use_curl)
    include_directories( ${CURL_INCLUDE_DIRS} )
endif(CURL_FOUND)

# X11
if(X11_FOUND)
    add_definitions(-Dcimg_display=1)
    add_definitions(-Dcimg_appname="gmic")
elseif (WIN32 AND MSVC)
    # CMake for MSVC automatically links and finds headers for gdi32
    add_definitions(-Dcimg_display=2)
    add_definitions(-Dcimg_appname="gmic")
else(X11_FOUND)
    add_definitions(-Dcimg_display=0)
endif(X11_FOUND)


# Extra for Krita
if(APPLE)
    add_definitions(-D_APPLE)
endif()

kde4_add_library(gmic STATIC ${gmic_sources_SRCS} ${gmic_headers_SRCS})

if(FFTW3_FOUND)
  target_link_libraries(gmic ${FFTW3_LIBRARIES})
endif(FFTW3_FOUND)

if (X11_FOUND)
    target_link_libraries(gmic ${X11_LIBRARIES})
endif(X11_FOUND)

if(CURL_FOUND)
    target_link_libraries(gmic ${CURL_LIBRARIES})
endif(CURL_FOUND)

target_link_libraries(gmic ${ZLIB_LIBRARIES})
target_link_libraries(gmic ${PNG_LIBRARIES})

if (WIN32)
    target_link_libraries(gmic)
else (WIN32)
    target_link_libraries(gmic pthread)
endif (WIN32)


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
    kis_gmic_progress_manager.cpp
    kis_gmic_small_applicator.cpp
    kis_gmic_plugin.cpp ${kritagmic_shared_PART_SRCS}
)
kde4_add_plugin(kritagmic ${kritagmic_PART_SRCS})
target_link_libraries(kritagmic kritaui gmic ${QT_QTXML_LIBRARY} ${ZLIB_LIBRARIES})

# gmicparser for debugging purposes

set(gmicparser_PART_SRCS gmicparser.cpp ${kritagmic_shared_PART_SRCS})
kde4_add_executable(gmicparser ${gmicparser_PART_SRCS})
target_link_libraries(gmicparser ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kritaui gmic ${ZLIB_LIBRARIES})

########### install files ###############
set(GMIC_INSTALL_DIR ${DATA_INSTALL_DIR}/krita/gmic)

install(TARGETS kritagmic  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS gmicparser  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES  gmic.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritagmic.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES  src/gmic_def.gmic gmic_def.gmic.blacklist DESTINATION ${GMIC_INSTALL_DIR} )

# tests, currently broken on OSX due to fftw linking problem
if(NOT APPLE)
    add_subdirectory(tests)
endif()
