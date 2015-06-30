add_subdirectory( tests )

set(libkispsd_LIB_SRCS
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

kde4_add_library(libkispsd SHARED ${libkispsd_LIB_SRCS} )

if (WIN32)
    target_link_libraries(libkispsd pigmentcms libglobal ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    target_link_libraries(libkispsd pigmentcms libglobal ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif (WIN32)

target_link_libraries(libkispsd LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})


set_target_properties(libkispsd PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS libkispsd  ${INSTALL_TARGETS_DEFAULT_ARGS})

