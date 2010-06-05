

include_directories( ${CMAKE_SOURCE_DIR}/kchart/ ${KOMAIN_INCLUDES} )


########### next target ###############

set(kochart_LIB_SRCS koChart.cc)


kde4_add_library(kochart SHARED ${kochart_LIB_SRCS})

target_link_libraries(kochart komain )

set_target_properties(kochart PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kochart  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES KoGenericRegistry.h KoID.h koChart.h KoChartInterface.h KoChartModel.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

