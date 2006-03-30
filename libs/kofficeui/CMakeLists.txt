kde4_header()

add_subdirectory( tests )
add_subdirectory( pics )




########### next target ###############

set(kofficeui_LIB_SRCS 
   KoPageLayoutDia.cpp 
   KoZoomAction.cpp 
   KoTabBar.cpp 
   KoPartSelectDia.cpp 
   KoPartSelectAction.cpp 
   KoRuler.cpp 
   KoTemplateChooseDia.cpp 
   KoTabChooser.cpp 
   KoKoolBar.cpp 
   KoTemplateCreateDia.cpp 
   KoContextHelp.cpp 
   kcoloractions.cpp 
   KoPictureFilePreview.cpp 
   KoUnitWidgets.cpp 
   tkaction.cpp 
   tkcoloractions.cpp 
   tkcombobox.cpp 
   tktoolbarbutton.cpp 
   KoCharSelectDia.cpp 
   KoInsertLink.cpp 
   KoEditPath.cpp 
   KoCommandHistory.cpp 
   KoSelectAction.cpp 
   Kolinewidthaction.cpp 
   Kolinestyleaction.cpp 
   KoTooluButton.cpp 
   KoBrush.cpp 
   KoImageResource.cpp 
   KoToolBox.cpp 
   KoZoomHandler.cpp 
   KoGuideLineDia.cpp 
   KoGuides.cpp 
   KoPageLayoutColumns.cpp 
   KoPageLayoutSize.cpp 
   KoPageLayoutHeader.cpp 
   KoPen.cpp 
   KoZoomMode.cpp )

kde4_automoc(${kofficeui_LIB_SRCS})

kde4_add_ui3_files(kofficeui_LIB_SRCS KoGeneralPropertyUi.ui KoPageLayoutColumnsBase.ui KoPageLayoutHeaderBase.ui )

kde4_add_library(kofficeui SHARED ${kofficeui_LIB_SRCS})

target_link_libraries(kofficeui  ${KDE4_KDECORE_LIBS} kofficecore )

set_target_properties(kofficeui PROPERTIES VERSION 3.0.0 SOVERSION 3 )
install_targets(${LIB_INSTALL_DIR} kofficeui )


########### install files ###############

install_files( /include FILES  KoPageLayoutDia.h KoZoomAction.h KoTabBar.h  	KoPartSelectDia.h KoPartSelectAction.h  	KoRuler.h KoTemplateChooseDia.h KoTabChooser.h  	KoKoolBar.h KoTemplateCreateDia.h KoContextHelp.h  	kcoloractions.h  	KoPictureFilePreview.h KoUnitWidgets.h  	tkaction.h tkcoloractions.h tktoolbarbutton.h tkcombobox.h  	KoCharSelectDia.h KoInsertLink.h KoTooluButton.h KoEditPath.h  	KoCommandHistory.h  KoImageResource.h  	KoSelectAction.h Kolinewidthaction.h Kolinestyleaction.h  	KoZoomHandler.h KoGuideLineDia.h KoGuides.h  	KoPageLayoutHeader.h KoPageLayoutSize.h KoGeneralPropertyUi.h KoPageLayoutColumns.h  	KoBrush.h KoPen.h KoZoomMode.h )

kde4_footer()



#original Makefile.am contents follow:

####### General stuff
#
#KDE_CXXFLAGS = $(USE_RTTI) -DMAKE_KOFFICEUI_LIB
#INCLUDES =  $(KSTORE_INCLUDES) $(KOFFICECORE_INCLUDES) $(all_includes)
#
######## Files
#
#SUBDIRS = . tests pics
#
#lib_LTLIBRARIES = libkofficeui.la
#
#libkofficeui_la_SOURCES = \
#        KoPageLayoutDia.cpp KoZoomAction.cpp KoTabBar.cpp \
#	KoPartSelectDia.cpp KoPartSelectAction.cpp \
#	KoRuler.cpp KoTemplateChooseDia.cpp KoTabChooser.cpp \
#	KoKoolBar.cpp KoTemplateCreateDia.cpp KoContextCelp.cpp\
#	kcoloractions.cpp \
#	KoPictureFilePreview.cpp KoUnitWidgets.cpp \
#	tkaction.cpp tkcoloractions.cpp tkcombobox.cpp tktoolbarbutton.cpp \
#	KoCharSelectDia.cpp KoInsertLink.cpp KoEditPath.cpp KoCommandHistory.cpp \
#	KoSelectAction.cpp Kolinewidthaction.cpp Kolinestyleaction.cpp \
#	KoTooluButton.cpp \
#	KoBrush.cpp KoImageResource.cpp KoToolBox.cpp KoZoomHandler.cpp \
#	KoGuideLineDia.cpp KoGuides.cpp \
#	KoGeneralPropertyUi.ui KoPageLayoutColumns.cpp KoPageLayoutColumnsBase.ui KoPageLayoutSize.cpp \
#	KoPageLayoutHeaderBase.ui KoPageLayoutHeader.cpp KoPen.cpp KoZoomMode.cpp
#
#libkofficeui_la_LDFLAGS	= -version-info 3:0:0 -no-undefined $(all_libraries)
#libkofficeui_la_LIBADD  = $(LIB_KOFFICECORE) 
#
#include_HEADERS = \
#	KoPageLayoutDia.h KoZoomAction.h KoTabBar.h \
#	KoPartSelectDia.h KoPartSelectAction.h \
#	KoRuler.h KoTemplateChooseDia.h KoTabChooser.h \
#	KoKoolBar.h KoTemplateCreateDia.h KoContextCelp.h \
#	kcoloractions.h \
#	KoPictureFilePreview.h KoUnitWidgets.h \
#	tkaction.h tkcoloractions.h tktoolbarbutton.h tkcombobox.h \
#	KoCharSelectDia.h KoInsertLink.h KoTooluButton.h KoEditPath.h \
#	KoCommandHistory.h  KoImageResource.h \
#	KoSelectAction.h Kolinewidthaction.h Kolinestyleaction.h \
#	KoZoomHandler.h KoGuideLineDia.h KoGuides.h \
#	KoPageLayoutHeader.h KoPageLayoutSize.h KoGeneralPropertyUi.h KoPageLayoutColumns.h \
#	KoBrush.h KoPen.h KoZoomMode.h
#
#
## FIXME: Disabled for now as it breaks the installation process if
## KOffice is not installed in $KDEDIR
## 
##AM_CXXFLAGS = -DQT_PLUGIN
##kde_widget_LTLIBRARIES = kofficewidgets.la
##kofficewidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
##kofficewidgets_la_LIBADD = $(LIB_KIO) libkofficeui.la
##kofficewidgets_la_SOURCES =  kofficewidgets.cpp
#
##kofficewidgets.cpp: $(srcdir)/koffice.widgets
##	$(MAKEKDEWIDGETS) -o kofficewidgets.cpp $(srcdir)/koffice.widgets
#
##CLEANFILES = kofficewidgets.cpp
#
#METASOURCES = AUTO
