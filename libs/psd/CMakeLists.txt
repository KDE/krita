include_directories( ${CMAKE_BINARY_DIR}/libs/psd  #For kispsd_include.h
)

set(kritapsd_LIB_SRCS
    psd_utils.cpp
    psd.cpp
    compression.cpp
    psd_pattern.cpp

    asl/kis_asl_reader.cpp
    asl/kis_asl_reader_utils.cpp
    asl/kis_asl_xml_parser.cpp
    asl/kis_asl_object_catcher.cpp
    asl/kis_asl_callback_object_catcher.cpp
    asl/kis_asl_xml_writer.cpp
    asl/kis_asl_writer_utils.cpp
    asl/kis_asl_patterns_writer.cpp
    asl/kis_asl_writer.cpp
)

kis_add_core_library(kritapsd SHARED ${kritapsd_LIB_SRCS} )
generate_export_header(kritapsd BASE_NAME kritapsd)

if (WIN32)
    target_link_libraries(kritapsd kritapigment kritaglobal KF5::I18n  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    target_link_libraries(kritapsd kritapigment kritaglobal KF5::I18n  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif (WIN32)

set_target_properties(kritapsd PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritapsd  ${INSTALL_TARGETS_DEFAULT_ARGS})

