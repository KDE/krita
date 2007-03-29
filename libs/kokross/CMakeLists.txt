include_directories(${KDE4_KDECORE_INCLUDES} ${KOFFICECORE_INCLUDES})
set(kokross_LIB_SRCS KoScriptingModule.cpp KoScriptingDocker.cpp)
kde4_automoc(kokross ${kokross_LIB_SRCS})
kde4_add_library(kokross SHARED ${kokross_LIB_SRCS})

target_link_libraries(kokross ${KDE4_KDECORE_LIBS} ${KDE4_KROSSCORE_LIBS} kofficecore)
set_target_properties(kokross PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kokross DESTINATION ${LIB_INSTALL_DIR})
