include_directories(${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KOMAIN_INCLUDES})
set(kokross_LIB_SRCS KoScriptingModule.cpp KoScriptingDocker.cpp KoScriptManager.cpp KoScriptManagerAdd.cpp KoScriptingPart.cpp)
kde4_add_library(kokross SHARED ${kokross_LIB_SRCS})

target_link_libraries(kokross ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KNEWSTUFF2_LIBRARY} ${KDE4_KROSSUI_LIBS} komain)
set_target_properties(kokross PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION})
install(TARGETS kokross DESTINATION ${LIB_INSTALL_DIR})
