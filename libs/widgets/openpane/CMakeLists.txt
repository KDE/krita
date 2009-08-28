include_directories(${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR}/libs/widgets)
add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(koopenpane_LIB_SRCS
   KoDetailsPane.cpp
   KoExistingDocumentPane.cpp
   KoOpenPane.cpp
   KoRecentDocumentsPane.cpp
   KoTemplate.cpp
   KoTemplateCreateDia.cpp
   KoTemplateGroup.cpp
   KoTemplates.cpp
   KoTemplatesPane.cpp
   KoTemplateTree.cpp
)


kde4_add_ui_files( koopenpane_LIB_SRCS
   koOpenPaneBase.ui
   koDetailsPaneBase.ui 
)

kde4_add_library(koopenpane SHARED ${koopenpane_LIB_SRCS})

target_link_libraries(koopenpane
    ${KDE4_KFILE_LIBRARY}
    ${KDE4_KIO_LIBRARY}
)

set_target_properties(koopenpane PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koopenpane ${INSTALL_TARGETS_DEFAULT_ARGS} )
