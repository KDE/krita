include_directories(BEFORE ${KROSS_INCLUDES} )

add_subdirectory( kritacore )
#add_subdirectory( samples )
add_subdirectory( scripts )

########### next target ###############

set(kritascripting_PART_SRCS scriptingmonitor.cpp scriptingdocker.cpp scriptingpart.cpp)

kde4_automoc(${kritascripting_PART_SRCS})
kde4_add_plugin(kritascripting ${kritascripting_PART_SRCS})

target_link_libraries(kritascripting kritaui ${KDE4_KROSSCORE_LIBS} krosskritacore)

install(TARGETS kritascripting  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  scripting.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritascripting.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
