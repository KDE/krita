KDE_OPTIONS = nofinal

INCLUDES = -I$(srcdir)/../core $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)

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
    kis_dlg_toolopts.cc \
    kis_dlg_toolopts_polygon_preview.cc \
    kis_dlg_transform.cc \
    kis_dockframedocker.cc \
    kis_gradientwidget.cc \
    kis_gray_widget.cc \
    kis_icon_item.cc kis_ruler.cc \
    kis_iconwidget.cc \
    kis_itemchooser.cc \
    kis_previewwidgetbase.ui \
    kis_previewwidget.cc \
    kis_rgb_widget.cc \
    kis_hsv_widget.cc 
#    kis_histogram.cc


#    blendchooser.cc
#    kis_dlg_gradient.cc 

libkisui_la_METASOURCES = AUTO
SUBDIRS = labels dialogs

