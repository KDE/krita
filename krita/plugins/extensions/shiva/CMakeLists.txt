add_subdirectory(collections)

include_directories(${OPENSHIVA_INCLUDE_DIRS})

set(kritashivalib_SRCS
    ShivaGeneratorConfigWidget.cpp
    PaintDeviceImage.cpp
    QVariantValue.cpp
    UpdaterProgressReport.cpp
)

########### shiva filters ###############

set(kritashivafilters_PART_SRCS
    shivafiltersplugin.cpp
    shivafilter.cpp
    ${kritashivalib_SRCS}
    )

kde4_add_plugin(kritashivafilters ${kritashivafilters_PART_SRCS})

target_link_libraries(kritashivafilters kritaimage ${OPENSHIVA_LDFLAGS} ${QTSHIVA_LDFLAGS})

install(TARGETS kritashivafilters  DESTINATION ${PLUGIN_INSTALL_DIR})

########### shiva filters ###############

set(kritashivagenerators_PART_SRCS
    shivageneratorsplugin.cpp
    shivagenerator.cpp
    ${kritashivalib_SRCS}
    )

kde4_add_plugin(kritashivagenerators ${kritashivagenerators_PART_SRCS})

target_link_libraries(kritashivagenerators kritaimage ${OPENSHIVA_LDFLAGS} ${QTSHIVA_LDFLAGS})

install(TARGETS kritashivagenerators  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritashivafilters.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  kritashivagenerators.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
