KDE_OPTIONS = nofinal

INCLUDES = -I$(srcdir)/../core -I$(srcdir)/../core/compositeop -I$(srcdir)/../core/color_strategy -I$(srcdir)/../core/tiles $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkisui.la
libkisui_la_LIBADD = ./labels/libkislabel.la ./dialogs/libkisdialogs.la
libkisui_la_SOURCES = integerwidget.cc \
    kis_dlg_preferences.cc \
    kis_dlg_builder_progress.cc \
    gradientview.cc \
    kis_basedocker.cc \
    kis_canvas.cc \
    kis_channelview.cc \
    kis_cmb_composite.cc \
    kis_cmb_imagetype.cc \
    kis_colordocker.cc \
    kis_color_widget.cc \
    kis_controlframe.cc \
    kis_layerbox.cc \
    kis_dlg_create_img.cc \
    kis_dlg_new_layer.cc \
    kis_dlg_paintoffset.cc \
    kis_dlg_paint_properties.cc \
    kis_dlg_transform.cc \
    kis_dockframedocker.cc \
    kis_gradientwidget.cc \
    kis_gray_widget.cc \
    kis_icon_item.cc \
    kis_ruler.cc \
    kis_iconwidget.cc \
    kis_itemchooser.cc \
    kis_previewwidget.cc \
    kis_rgb_widget.cc \
    kis_hsv_widget.cc \
    kis_histogram_widget.cc \
    kis_wdg_autobrush.ui \
    kis_autobrush.cc \
    kis_brush_chooser.cc \
    kis_pattern_chooser.cc

noinst_HEADERS = integerwidget.h \
    kis_dlg_preferences.h \
    kis_dlg_builder_progress.h \
    gradientview.h \
    kis_basedocker.h \
    kis_canvas.h \
    kis_channelview.h \
    kis_cmb_composite.h \
    kis_cmb_imagetype.h \
    kis_colordocker.h \
    kis_color_widget.h \
    kis_controlframe.h \
    kis_layerbox.h \
    kis_dlg_create_img.h \
    kis_dlg_new_layer.h \
    kis_dlg_paintoffset.h \
    kis_dlg_paint_properties.h \
    kis_dlg_transform.h \
    kis_dockframedocker.h \
    kis_gradientwidget.h \
    kis_gray_widget.h \
    kis_icon_item.h \
    kis_ruler.h \
    kis_iconwidget.h \
    kis_itemchooser.h \
    kis_previewwidget.h \
    kis_rgb_widget.h \
    kis_hsv_widget.h \
    kis_histogram_widget.h \
    kis_autobrush.h \
    kis_brush_chooser.h \
    kis_pattern_chooser.h

libkisui_la_METASOURCES = AUTO
SUBDIRS = labels dialogs 

