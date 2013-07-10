

include_directories( ${KOMAIN_INCLUDES} )


########### next target ###############

set(kochart_LIB_SRCS koChart.cc)


kde4_add_library(kochart SHARED ${kochart_LIB_SRCS})

target_link_libraries(kochart komain )

set_target_properties(kochart PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS kochart  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES KoGenericRegistry.h KoID.h koChart.h KoChartInterface.h KoChartModel.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)

