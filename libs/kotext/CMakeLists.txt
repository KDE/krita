

include_directories( ${KOFFICEUI_INCLUDES} )

add_subdirectory( kohyphen ) 
add_subdirectory( tests ) 
add_subdirectory( styles/tests )


# Need by kohyphen
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############
set(libkohyphen_SRCS 
	${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen/hnjalloc.c 
	${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen/hyphen.c 
	${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen/kohyphen.cpp
	)

set(kotext_LIB_SRCS ${libkohyphen_SRCS}
    KoFontTab.cpp 
    KoHighlightingTab.cpp 
    KoDecorationTab.cpp 
    KoLayoutTab.cpp 
    KoLanguageTab.cpp 
    KoFontDia.cpp 
    KoTextShapeData.cpp
    KoText.cpp
    KoTextBlockData.cpp
    KoTextSelectionHandler.cpp
    KoTextBlockBorderData.cpp
    KoTextDocumentLayout.cpp

    KoTextEditingRegistry.cpp
    KoTextEditingFactory.cpp
    KoTextEditingPlugin.cpp

    KoInlineTextObjectManager.cpp
    KoInlineObjectFactory.cpp
    KoInlineObjectRegistry.cpp
    InsertVariableActionBase.cpp
    InsertVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertTextReferenceAction.cpp
    KoInlineObject.cpp
    KoVariable.cpp
    KoVariableManager.cpp
    KoNamedVariable.cpp
    KoTextLocator.cpp
    KoTextReference.cpp

    styles/Styles_p.cpp
    styles/KoCharacterStyle.cpp
    styles/KoParagraphStyle.cpp
    styles/KoStyleManager.cpp
    styles/KoListStyle.cpp
    styles/ChangeFollower.cpp
 )

kde4_automoc(${kotext_LIB_SRCS})

kde4_add_ui_files(kotext_LIB_SRCS
   timedateformatwidget.ui
   kolanguagetabbase.ui
    KoDecorationTab.ui
    KoParagDecorationTab.ui
    KoLayoutTab.ui
    KoHighlightingTab.ui
    KoCompletionBase.ui
)

kde4_add_library(kotext SHARED ${kotext_LIB_SRCS})


target_link_libraries(kotext  ${KDE4_SONNETCORE_LIBS} flake ${KDE4_KUTILS_LIBS} kofficecore kofficeui )

set_target_properties(kotext PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kotext  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES texteditingplugin.desktop inlinetextobject.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
