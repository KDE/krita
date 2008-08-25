include_directories(${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KOMAIN_INCLUDES})
set(kokross_LIB_SRCS KoScriptingModule.cpp KoScriptingDocker.cpp KoScriptManager.cpp KoScriptManagerAdd.cpp KoScriptingPart.cpp KoScriptingOdf.cpp)
kde4_add_library(kokross SHARED ${kokross_LIB_SRCS})

target_link_libraries(kokross kstore koodf ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KROSSCORE_LIBRARY} ${KDE4_KNEWSTUFF2_LIBRARY} ${KDE4_KROSSUI_LIBS} komain)
set_target_properties(kokross PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION})
install(TARGETS kokross ${INSTALL_TARGETS_DEFAULT_ARGS})
