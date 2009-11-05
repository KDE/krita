add_subdirectory(collections)

include_directories(${OPENSHIVA_INCLUDE_DIR})

if(HAVE_OPENSHIVA_912)

set(kritashiva_912_PART_SRCS
    UpdaterProgressReport.cpp
    )

endif(HAVE_OPENSHIVA_912)

set(kritashiva_PART_SRCS
    shivaplugin.cpp
    shivafilter.cpp
    shivagenerator.cpp
    ShivaGeneratorConfigWidget.cpp
    PaintDeviceImage.cpp
    QVariantValue.cpp
    ${kritashiva_912_PART_SRCS}
    )

kde4_add_plugin(kritashiva ${kritashiva_PART_SRCS})

target_link_libraries(kritashiva kritaui ${OPENSHIVA_LIBRARIES} ${QTSHIVA_LIBRARIES})

install(TARGETS kritashiva  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritashiva.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
