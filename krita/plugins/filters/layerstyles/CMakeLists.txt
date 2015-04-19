add_subdirectory( tests )

set(kritalayerstylesfilter_PART_SRCS
    layerstyles.cpp

    kis_asl_reader.cpp
    kis_asl_xml_parser.cpp
    kis_asl_object_catcher.cpp
    kis_asl_callback_object_catcher.cpp
    kis_asl_xml_writer.cpp
    kis_asl_writer_utils.cpp
    kis_asl_patterns_writer.cpp
    kis_asl_writer.cpp

    kis_asl_layer_style_serializer.cpp
    )

kde4_add_plugin(kritalayerstylesfilter ${kritalayerstylesfilter_PART_SRCS})



target_link_libraries(kritalayerstylesfilter kritaui libkispsd)

install(TARGETS kritalayerstylesfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritalayerstylesfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
