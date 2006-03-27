####### General stuff

SUBDIRS = . tests
KDE_CXXFLAGS = $(USE_RTTI) $(WOVERLOADED_VIRTUAL)
INCLUDES= $(KSTORE_INCLUDES) $(KWMF_INCLUDES) $(all_includes)
libkofficecore_la_LIBADD  = $(LIB_KSTORE) $(LIB_KOWMF) $(LIB_KPARTS) \
	$(LIB_KDEPRINT) $(LIB_KABC) $(LIB_KWMF)

####### Files

lib_LTLIBRARIES = libkofficecore.la 
kde_module_LTLIBRARIES = kodocinfopropspage.la

libkofficecore_la_SOURCES = KoDocument.cpp \
	KoGlobal.cpp KoUnit.cpp KoFilterManager.cpp \
	KoMainWindow.cpp \
	KoApplication.cpp KoQueryTrader.cpp \
	KoFilter.cpp KoFilterChain.cpp \
	KoPictureKey.cpp KoPictureBase.cpp KoPicture.cpp KoPictureShared.cpp \
	KoPictureImage.cpp KoPictureClipart.cpp KoPictureCollection.cpp \
	KoPictureEps.cpp KoPictureWmf.cpp \
	KoDocumentInfo.cpp \
	KoView.cpp KoFrame.cpp KoContainerHandler.cpp KoDocumentChild.cpp \
	KoDocumentInfoDlg.cpp KoFactory.cpp KoChild.cpp \
	koDocumentInfoAboutWidget.ui koDocumentInfoAuthorWidget.ui \
	koDocumentInfoUserMetadataWidget.ui \
	KoApplicationIface.cc KoApplicationIface.skel \
	KoDocumentIface.cc KoDocumentIface.skel KoViewIface.cc KoViewIface.skel \
	KoMainWindowIface.cc KoMainWindowIface.skel kofficeversion.cc KoOasisStyles.cpp \
	KoStyleStack.cpp KoGenStyles.cpp KoOasisSettings.cpp KoPageLayout.cpp \
	KoFileDialog.cpp KoXmlNS.cpp KoDom.cpp Koversiondialog.cpp KoOasisStore.cpp \
	kkbdaccessextensions.cpp koDetailsPaneBase.ui koOpenPaneBase.ui KoOpenPane.cpp \
	KoTemplates.cpp KoDetailsPane.cpp \
	KoSpeaker.cpp KoOasisLoadingContext.cpp \
	KoRect.cpp

libkofficecore_la_LDFLAGS = $(all_libraries) -version-info 3:0:0 -no-undefined

include_HEADERS = KoContainerHandler.h \
	KoFilter.h KoFilterChain.h \
	KoGlobal.h KoUnit.h KoDocument.h \
	KoMainWindow.h \
	KoApplication.h KoQueryTrader.h \
	KoFilterManager.h \
	KoDocumentInfo.h \
	KoView.h KoFrame.h KoDocumentChild.h \
	KoDocumentInfoDlg.h KoFactory.h KoChild.h \
	KoApplicationIface.h KoDocumentIface.h KoViewIface.h KoMainWindowIface.h \
	KoPictureKey.h KoPicture.h KoPictureCollection.h kofficeversion.h KoOasisStyles.h \
	KoStyleStack.h KoGenStyles.h KoOasisSettings.h KoPageLayout.h KoXmlNS.h KoDom.h Koversiondialog.h \
	kkbdaccessextensions.h \
	koffice_export.h KoOpenPane.h \
	KoSpeaker.h KoOasisLoadingContext.h \
	KoPoint.h

noinst_HEADERS = KoDocument_p.h KoFilterManager_p.h \
	KoPictureShared.h KoPictureBase.h KoPictureImage.h KoPictureClipart.h \
	KoPictureEps.h KoPictureWmf.h

kodocinfopropspage_la_SOURCES = KoDocInfoPropsFactory.cpp
kodocinfopropspage_la_LIBADD = ./libkofficecore.la
kodocinfopropspage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

rcdir = $(kde_datadir)/koffice
rc_DATA = koffice_shell.rc

service_DATA = kodocinfopropspage.desktop
servicedir = $(kde_servicesdir)
