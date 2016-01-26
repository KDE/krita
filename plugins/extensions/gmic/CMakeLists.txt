##
# CMake for gmic
##

set(GMIC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gmic")

include_directories(${KOWIDGETS_INCLUDES} ${GMIC_SOURCE_DIR})

if(NOT MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
endif()

# turn off O2 and Ob1 or Ob2 for g'mic
if (MSVC)
    # RelWithDebInfo
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oi /Ot /Oy /Gs /GF /Gy")
    string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    string(REPLACE "/Ob1" "/Ob0" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})

    #Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /Ot /Oy /Gs /GF /Gy")
    string(REPLACE "/O2" "/Od"  CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    string(REPLACE "/Ob2" "/Ob0" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()

set(gmic_sources_SRCS
  ${GMIC_SOURCE_DIR}/gmic.cpp
)

set(gmic_headers_SRCS
  ${GMIC_SOURCE_DIR}/CImg.h
  ${GMIC_SOURCE_DIR}/gmic_def.h
  ${GMIC_SOURCE_DIR}/gmic.h
)

# Mandatory flags
add_definitions(-Dgmic_gimp)
add_definitions(-Dgmic_build)
add_definitions(-Dgmic_float_only)
add_definitions(-Dcimg_use_vt100 )

# PARALLEL
if (Threads_FOUND)
    message("G'Mic: using pthreads")
    add_definitions(-Dgmic_is_parallel)
    add_definitions(-Dcimg_use_rng)
endif()

#OpenMP
if(OPENMP_FOUND)
    message("G'Mic: using OpenMP")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#FFTW
if(FFTW3_FOUND)
    message("G'Mic: using FFTW3")
    add_definitions(-Dcimg_use_fftw3 )
    add_definitions(-Dcimg_use_fftw3_singlethread )
    include_directories(${FFTW3_INCLUDE_DIR})
endif()

# PNG
if (PNG_FOUND)
    add_definitions(${PNG_DEFINITIONS})
    add_definitions(-Dcimg_use_png)
    include_directories(${PNG_INCLUDE_DIR})
endif()

# ZLIB
if (ZLIB_FOUND)
    add_definitions(-Dcimg_use_zlib)
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif()


# CURL
if (CURL_FOUND)
    message("G'Mic: using Curl")
    add_definitions(-Dcimg_use_curl)
    include_directories( ${CURL_INCLUDE_DIRS} )
endif()

# X11
if(X11_FOUND)
    add_definitions(-Dcimg_display=1)
    add_definitions(-Dcimg_appname="gmic")
elseif (WIN32 AND MSVC)
    # CMake for MSVC automatically links and finds headers for gdi32
    add_definitions(-Dcimg_display=2)
    add_definitions(-Dcimg_appname="gmic")
else()
    add_definitions(-Dcimg_display=0)
endif()


# Extra for Krita
if(APPLE)
    add_definitions(-D_APPLE)
endif()

add_library(gmic STATIC ${gmic_sources_SRCS} ${gmic_headers_SRCS})

if(FFTW3_FOUND)
  target_link_libraries(gmic ${FFTW3_LIBRARIES})
endif()

if (X11_FOUND)
    
    target_link_libraries(gmic ${X11_LIBRARIES} KF5::WindowSystem Qt5::Core)
endif()

if(CURL_FOUND)
    target_link_libraries(gmic ${CURL_LIBRARIES})
endif()

target_link_libraries(gmic ${ZLIB_LIBRARIES})
target_link_libraries(gmic ${PNG_LIBRARIES})

if(Threads_FOUND)
    target_link_libraries(gmic ${CMAKE_THREAD_LIBS_INIT})
endif()


##
# compile Krita plug-in and link static library in
##

set(kritagmic_shared_SOURCES
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

ki18n_wrap_ui(kritagmic_shared_SOURCES wdg_gmic.ui wdg_gmic_input_output.ui)

set(kritagmic_SOURCES
    kis_gmic_simple_convertor.cpp
    kis_export_gmic_processing_visitor.cpp
    kis_gmic_applicator.cpp
    kis_gmic_command.cpp
    kis_gmic_data.cpp
    kis_gmic_synchronize_layers_command.cpp
    kis_gmic_synchronize_image_size_command.cpp
    kis_import_gmic_processing_visitor.cpp
    kis_gmic_progress_manager.cpp
    kis_gmic_small_applicator.cpp
    kis_gmic_plugin.cpp ${kritagmic_shared_SOURCES}
)
add_library(kritagmic MODULE ${kritagmic_SOURCES})
target_link_libraries(kritagmic kritaui gmic Qt5::Xml Qt5::Network ${ZLIB_LIBRARIES})

# gmicparser for debugging purposes

set(gmicparser_SOURCES gmicparser.cpp ${kritagmic_shared_SOURCES})
add_executable(gmicparser ${gmicparser_SOURCES})
target_link_libraries(gmicparser Qt5::Core Qt5::Gui Qt5::Network kritaui gmic ${ZLIB_LIBRARIES})

########### install files ###############
set(GMIC_INSTALL_DIR ${DATA_INSTALL_DIR}/krita/gmic)

install(TARGETS kritagmic  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install(TARGETS gmicparser  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES  gmic.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES
  #${GMIC_SOURCE_DIR}/gmic_def.gmic
  ${GMIC_SOURCE_DIR}/update1610.gmic
  gmic_def.gmic.blacklist
DESTINATION ${GMIC_INSTALL_DIR} )

# tests, currently broken on OSX due to fftw linking problem
if(NOT APPLE)
    #add_subdirectory(tests)
endif()
