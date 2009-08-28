MACRO_ENSURE_VERSION( "4.1.81" ${KDE_VERSION} KDE_VERSION_OK)
if(NOT KDE_VERSION_OK)
    add_definitions( -DDISABLE_ADD_REMOVE )
    message(STATUS "KDE_VERSION: ${KDE_VERSION} (4.1.81 needed) ScriptManager: add/remove will not be compiled")
endif(NOT KDE_VERSION_OK)

include_directories(${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KOMAIN_INCLUDES})

set(kokross_LIB_SRCS
    KoScriptingModule.cpp
    KoScriptingDocker.cpp
    KoScriptManager.cpp
    KoScriptManagerAdd.cpp
    KoScriptingPart.cpp
    KoScriptingOdf.cpp
)
kde4_add_library(kokross SHARED ${kokross_LIB_SRCS})

target_link_libraries(kokross
    komain
    ${KDE4_KFILE_LIBRARY}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KROSSCORE_LIBRARY}
    ${KDE4_KNEWSTUFF2_LIBRARY}
    ${KDE4_KROSSUI_LIBS}
)

target_link_libraries(kokross LINK_INTERFACE_LIBRARIES ${KDE4_KROSSCORE_LIBRARY} ${KDE4_KROSSUI_LIBS})

set_target_properties(kokross PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kokross ${INSTALL_TARGETS_DEFAULT_ARGS})
