INCLUDES = -I$(srcdir)/../core $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkisui.la
libkisui_la_LIBADD = ./labels/libkislabel.la ./dialogs/libkisdialogs.la
libkisui_la_SOURCES = integerwidget.cc kis_dlg_preferences.cc kis_dlg_builder_progress.cc \
    kis_tabbar.cc  kis_canvas.cc \
    kis_iconwidget.cc \
    kis_gradientwidget.cc kis_previewwidget.cc \
    kis_channelview.cc gradientview.cc \
    kis_dlg_toolopts.cc kis_dlg_toolopts_polygon_preview.cc \
    kis_sidebar.cc kis_listbox.cc \
    kis_itemchooser.cc kis_dlg_paint_properties.cc kis_dlg_paintoffset.cc \
    kis_dlg_new_layer.cc kis_dlg_create_img.cc kis_dlg_dimension.cc \
    kis_icon_item.cc kis_ruler.cc \
    kis_cmb_composite.cc \
    kis_cmb_imagetype.cc \
    kis_dlg_transform.cc \
    kis_color_widget.cc \
    kis_rgb_widget.cc kis_hsv_widget.cc \
    kis_gray_widget.cc
    
#    blendchooser.cc
#    kis_dlg_gradient.cc 

libkisui_la_METASOURCES = AUTO
SUBDIRS = labels dialogs

