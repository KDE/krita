include_directories( 
    ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets/documentsectionbox
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${PIGMENT_INCLUDES}
)

if(KDE4_BUILD_TESTS)
    # only with this definition will the KOPAGEAPP_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)
add_subdirectory(tools)

### kopageapp ###
set( kopageapp_LIB_SRCS
     KoPADocument.cpp
     KoShapeTraversal.cpp
     KoPAView.cpp
     KoPACanvas.cpp
     KoPASavingContext.cpp
     KoPAPageBase.cpp
     KoPAMasterPage.cpp
     KoPAPage.cpp
     KoPAViewMode.cpp
     KoPAViewModeNormal.cpp
     KoPALoadingContext.cpp
     KoPAOdfPageSaveHelper.cpp
     KoPAPastePage.cpp
     KoPADocumentModel.cpp
     KoPAPageThumbnailModel.cpp
     KoPADocumentStructureDocker.cpp
     KoPAUtil.cpp
     KoPAPrintJob.cpp
     commands/KoPAPageInsertCommand.cpp
     commands/KoPAPageDeleteCommand.cpp
     commands/KoPAPageMoveCommand.cpp
     commands/KoPAChangeMasterPageCommand.cpp
     commands/KoPADisplayMasterShapesCommand.cpp
     commands/KoPADisplayMasterBackgroundCommand.cpp
     dialogs/KoPAMasterPageDialog.cpp
)

kde4_add_library(kopageapp SHARED ${kopageapp_LIB_SRCS})

target_link_libraries(kopageapp koodf flake kodocumentsectionbox komain kostore ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} kozoomaction  kotext)
target_link_libraries(kopageapp LINK_INTERFACE_LIBRARIES flake komain )

set_target_properties(kopageapp PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kopageapp  ${INSTALL_TARGETS_DEFAULT_ARGS})

