SUBDIRS = kohyphen . tests
INCLUDES= $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(KSTORE_INCLUDES) $(all_includes)
## Needed for kohyphen...
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

lib_LTLIBRARIES = libkotext.la
libkotext_la_SOURCES = KoComplexText.cpp KoRichText.cpp \
	KoTextZoomHandler.cpp KoBorder.cpp \
	KoTextFormat.cpp KoParagCounter.cpp KoStyleCollection.cpp KoTextCommand.cpp \
	KoTextParag.cpp KoTextDocument.cpp KoTextFormatter.cpp KoParagLayout.cpp \
	KoTextObject.cpp KoTextView.cpp KoAutoFormat.cpp KoAutoFormatDia.cpp \
	KoParagDia.cpp KoVariable.cpp KoCustomVariablesDia.cpp \
	KoChangeCaseDia.cpp KoStyleManager.cpp KoSearchDia.cpp \
	timedateformatwidget.ui TimeFormatWidget.cpp DateFormatWidget.cpp \
	KFontDialog_local.cpp kofonttabbase.ui KoFontTab.cpp kohighlightingtabbase.ui KoHighlightingTab.cpp \
	kodecorationtabbase.ui KoDecorationTab.cpp kolayouttabbase.ui KoLayoutTab.cpp kolanguagetabbase.ui KoLanguageTab.cpp \
	KoFontDiaPreview.cpp KoCompletionDia.cpp KoCompletionBase.ui \
	KoTextViewIface.skel KoTextViewIface.cc KoFontDia.cpp KoBgSpellCheck.cpp \
	KoCommentDia.cpp KoSpell.cpp KoCreateStyleDia.cpp KoTextCustomItem.cpp \
	KoImportStyleDia.cpp KoTextIterator.cpp KoOasisContext.cpp KoListStyleStack.cpp \
	KoUserStyle.cpp KoUserStyleCollection.cpp KoParagStyle.cpp KoParagDecorationTab.ui \
	KoTextBookmark.cpp

libkotext_la_LDFLAGS = $(all_libraries) -version-info 3:0:0 -no-undefined

libkotext_la_LIBADD = $(LIB_KOFFICEUI) kohyphen/libkohyphen.la $(LIBKSPELL2) -lkutils

METASOURCES = AUTO

# include_HEADERS = 
#	KoTextZoomHandler.h KoTextFormat.h KoParagCounter.h KoStyleCollection.h 
#	KoTextDocument.h KoBgSpellCheck.h KoCommentDia.h KoCreateStyleDia.h
