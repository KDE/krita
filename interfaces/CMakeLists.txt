

include_directories( ${CMAKE_SOURCE_DIR}/kchart/kdchart ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kochart_LIB_SRCS koChart.cc )

kde4_automoc(${kochart_LIB_SRCS})

kde4_add_library(kochart SHARED ${kochart_LIB_SRCS})

target_link_libraries(kochart  ${KDE4_KDECORE_LIBS} kofficecore )

set_target_properties(kochart PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kochart  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  koChart.h  DESTINATION ${INCLUDE_INSTALL_DIR})

