set(kritapsdutils_LIB_SRCS
    psd.cpp
    compression.cpp

    asl/kis_asl_reader.cpp
    asl/kis_asl_xml_parser.cpp
    asl/kis_asl_object_catcher.cpp
    asl/kis_asl_callback_object_catcher.cpp
    asl/kis_asl_xml_writer.cpp
    asl/kis_asl_patterns_writer.cpp
    asl/kis_asl_writer.cpp

    cos/kis_cos_parser.cpp
    cos/kis_cos_writer.cpp
    cos/kis_txt2_utls.cpp
)

kis_add_library(kritapsdutils SHARED ${kritapsdutils_LIB_SRCS})
generate_export_header(kritapsdutils BASE_NAME kritapsdutils)

target_link_libraries(kritapsdutils
    PUBLIC
        kritapigment
        kritaglobal
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
    PRIVATE
        ZLIB::ZLIB
)

set_target_properties(kritapsdutils PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritapsdutils  ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
