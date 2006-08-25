include_directories(BEFORE ${KROSS_INCLUDES} )

#add_subdirectory( kritacore )
#add_subdirectory( samples )

########### next target ###############

set(kritascripting_PART_SRCS scriptingprogress.cpp scriptingmonitor.cpp scriptingmodule.cpp scriptingpart.cpp)

kde4_automoc(${kritascripting_PART_SRCS})
kde4_add_plugin(kritascripting ${kritascripting_PART_SRCS})
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritascripting )
target_link_libraries(kritascripting kritaui krosscore)

install(TARGETS kritascripting  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  scripting.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritascripting.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
