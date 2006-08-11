

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

#   KoComplexText.cpp  not compiled

set(kotext_LIB_SRCS ${libkohyphen_SRCS}
   KoRichText.cpp 
   KoTextZoomHandler.cpp 
   KoBorder.cpp 
   KoTextFormat.cpp 
   KoParagCounter.cpp 
   KoStyleCollection.cpp 
   KoTextCommand.cpp 
   KoTextParag.cpp 
   KoTextDocument.cpp 
   KoTextFormatter.cpp 
   KoParagLayout.cpp 
   KoTextObject.cpp 
   KoTextView.cpp 
   KoAutoFormat.cpp 
   KoAutoFormatDia.cpp 
   KoParagDia.cpp 
   KoVariable.cpp 
   KoCustomVariablesDia.cpp 
   KoChangeCaseDia.cpp 
   KoStyleDialog.cpp 
   KoSearchDia.cpp 
   TimeFormatWidget.cpp 
   DateFormatWidget.cpp 
   KFontDialog_local.cpp 
   KoFontTab.cpp 
   KoHighlightingTab.cpp 
   KoDecorationTab.cpp 
   KoLayoutTab.cpp 
   KoLanguageTab.cpp 
   KoFontDiaPreview.cpp 
   KoCompletionDia.cpp 
   KoFontDia.cpp 
   KoBgSpellCheck.cpp 
   KoCommentDia.cpp 
   KoSpell.cpp 
   KoCreateStyleDia.cpp 
   KoTextCustomItem.cpp 
   KoImportStyleDia.cpp 
   KoTextIterator.cpp 
   KoOasisContext.cpp 
   KoListStyleStack.cpp 
   KoUserStyle.cpp 
   KoUserStyleCollection.cpp 
   KoParagStyle.cpp 
   KoTextBookmark.cpp
   KoBorderPreview.cpp
   KoTextShapeData.cpp
    styles/Styles_p.cpp
    styles/KoCharacterStyle.cpp
    styles/KoParagraphStyle.cpp
    styles/KoStyleManager.cpp
    styles/ChangeFollower.cpp
 )

kde4_automoc(${kotext_LIB_SRCS})

kde4_add_ui_files(kotext_LIB_SRCS 
   timedateformatwidget.ui
)

kde4_add_ui3_files(kotext_LIB_SRCS 
   kofonttabbase.ui 
   kohighlightingtabbase.ui 
   kodecorationtabbase.ui 
   kolayouttabbase.ui 
   kolanguagetabbase.ui 
   KoCompletionBase.ui 
   KoParagDecorationTab.ui )

# kde4_add_dcop_skels(kotext_LIB_SRCS KoTextViewIface.h )

kde4_add_library(kotext SHARED ${kotext_LIB_SRCS})


target_link_libraries(kotext  ${KDE4_KSPELL2_LIBS} flake ${KDE4_KUTILS_LIBS} kofficecore kofficeui )

set_target_properties(kotext PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kotext  DESTINATION ${LIB_INSTALL_DIR})


########### Flake Plugin library ###############
set(kotext2_PART_SRCS
    KoTextShape.cpp
    KoTextShapeFactory.cpp
    KoTextTool.cpp
    KoTextToolFactory.cpp
 )

kde4_automoc(${kotext2_PART_SRCS})

kde4_add_plugin(kotext2 ${kotext2_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kotext2 )

target_link_libraries(kotext2 kotext flake kofficeui)

install(TARGETS kotext2  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES textshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

