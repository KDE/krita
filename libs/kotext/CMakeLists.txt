


include_directories( ${KOGUIUTILS_INCLUDES} ${KOMAIN_INCLUDES} )

add_subdirectory( kohyphen ) 
add_subdirectory( tests ) 
add_subdirectory( styles/tests )
add_subdirectory( opendocument/tests )


# Need by kohyphen
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############
set(libkohyphen_SRCS 
	${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen/hnjalloc.c 
	${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen/hyphen.c 
	${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen/kohyphen.cpp
	)

set(kotext_LIB_SRCS ${libkohyphen_SRCS}
    KoTextShapeData.cpp
    KoText.cpp
    KoTextBlockData.cpp
    KoTextSelectionHandler.cpp
    KoTextBlockBorderData.cpp
    KoTextDocumentLayout.cpp
    KoTextOdfSaveHelper.cpp
    KoTextPaste.cpp

    KoTextEditingRegistry.cpp
    KoTextEditingFactory.cpp
    KoTextEditingPlugin.cpp

    KoInlineTextObjectManager.cpp
#    KoInlineObjectFactory.cpp
#    KoInlineObjectRegistry.cpp
    InsertVariableActionBase.cpp
    InsertVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertTextReferenceAction.cpp
    KoInlineObject.cpp
    KoVariable.cpp
    KoVariableManager.cpp
    KoNamedVariable.cpp
    KoVariableFactory.cpp
    KoVariableRegistry.cpp
    KoTextLocator.cpp
    KoTextReference.cpp
    KoTextAnchor.cpp
    KoTextShapeContainerModel.cpp
    KoBookmark.cpp
    KoBookmarkManager.cpp
    KoInlineNote.cpp
    KoRulerController.cpp
    FindDirection_p.cpp
    KoFindStrategy.cpp
    KoReplaceStrategy.cpp
    KoFind_p.cpp
    KoFind.cpp
    KoTextDebug.cpp

    styles/Styles_p.cpp
    styles/KoCharacterStyle.cpp
    styles/KoParagraphStyle.cpp
    styles/KoStyleManager.cpp
    styles/KoListStyle.cpp
    styles/KoListLevelProperties.cpp
    styles/ChangeFollower.cpp

    opendocument/KoTextSharedLoadingData.cpp
    opendocument/KoTextLoader.cpp
)


kde4_add_ui_files(kotext_LIB_SRCS
   timedateformatwidget.ui
    KoParagDecorationTab.ui
    KoCompletionBase.ui
)

kde4_add_library(kotext SHARED ${kotext_LIB_SRCS})


target_link_libraries(kotext flake ${KDE4_KUTILS_LIBS} ${QT_QTGUI_LIBRARY} komain koguiutils )

set_target_properties(kotext PROPERTIES 
   VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
   ${KDE4_DISABLE_PROPERTY_}LINK_INTERFACE_LIBRARIES "${QT_QTGUI_LIBRARY}"
)
install(TARGETS kotext  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES texteditingplugin.desktop inlinetextobject.desktop textvariableplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
