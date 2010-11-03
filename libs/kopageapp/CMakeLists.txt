include_directories( 
    ${KOMAIN_INCLUDES}
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
     KoPAViewBase.cpp
     KoPACanvas.cpp
     KoPACanvasItem.cpp
     KoPACanvasBase.cpp
     KoPASavingContext.cpp
     KoPAPixmapCache.cpp
     KoPAPageBase.cpp
     KoPAMasterPage.cpp
     KoPAPage.cpp
     KoPAPageContainerModel.cpp
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
     KoPATextPage.cpp
     KoPAPageProvider.cpp
     commands/KoPAPageInsertCommand.cpp
     commands/KoPAPageDeleteCommand.cpp
     commands/KoPAPageMoveCommand.cpp
     commands/KoPAChangeMasterPageCommand.cpp
     commands/KoPAChangePageLayoutCommand.cpp
     commands/KoPADisplayMasterShapesCommand.cpp
     commands/KoPADisplayMasterBackgroundCommand.cpp
     dialogs/KoPAMasterPageDialog.cpp
     dialogs/KoPAPageLayoutDialog.cpp
     dialogs/KoPAConfigureDialog.cpp
)

kde4_add_library(kopageapp SHARED ${kopageapp_LIB_SRCS})

target_link_libraries(kopageapp komain)
target_link_libraries(kopageapp LINK_INTERFACE_LIBRARIES komain)

set_target_properties(kopageapp PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kopageapp  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kopageapp_export.h
    KoPACanvas.h
    KoPACanvasItem.h
    KoPACanvasBase.h
    KoPADocument.h
    KoPageApp.h
    KoPALoadingContext.h
    KoPAPageBase.h
    KoPAPageContainerModel.h
    KoPAUtil.h
    KoPAView.h
    KoPAViewBase.h
    KoPAViewMode.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
