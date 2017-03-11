if(OPENEXR_FOUND)
	include_directories(${OPENEXR_INCLUDE_DIR})
endif()

set(libkritatiffconverter_LIB_SRCS
    kis_tiff_converter.cc
    kis_tiff_writer_visitor.cpp
    kis_tiff_reader.cc
    kis_tiff_ycbcr_reader.cc
    kis_buffer_stream.cc
    )

set(krita_phf_import_SOURCES 
    ${libkritatiffconverter_LIB_SRCS}
	kis_phf_import.cpp 
)

ki18n_wrap_ui(krita_phf_import_SOURCES 
    wdgphfimport.ui 
)

add_library(krita_phf_import MODULE ${krita_phf_import_SOURCES})
target_link_libraries(krita_phf_import kritaui  ${TIFF_LIBRARIES})

install(TARGETS krita_phf_import  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_phf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

